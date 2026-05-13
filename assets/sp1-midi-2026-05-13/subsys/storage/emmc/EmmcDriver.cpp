#include "EmmcDriver.hpp"

#include <stem_player/Smf.hpp>
#include <hal/nrf_gpio.h>
#include <hal/nrf_spim.h>
#include <nrf.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <array>
#include <cstring>

LOG_MODULE_REGISTER(emmc, LOG_LEVEL_INF);

namespace hardware {

namespace {

EmmcDriver* g_async_emmc_instance = nullptr;

void spim3_irq_thunk(const void* arg) {
    EmmcDriver::spim3_irq_handler(arg);
}

void pwm0_irq_thunk(const void* arg) {
    EmmcDriver::pwm0_irq_handler(arg);
}

static inline void clk_delay() {
    k_busy_wait(1);
}

static inline void clk_delay_fast() {
    __NOP();
    __NOP();
}

// Helper: extract 32-bit card status from R1 response (bits [39:8] of 48-bit frame)
static uint32_t r1_status(const uint8_t* resp) {
    return ((uint32_t)resp[1] << 24) |
           ((uint32_t)resp[2] << 16) |
           ((uint32_t)resp[3] << 8) |
           (uint32_t)resp[4];
}

} // namespace

#define CLOCK_PULSES_FAST(n) \
    do { \
        volatile uint32_t* outset = &NRF_P0->OUTSET; \
        volatile uint32_t* outclr = &NRF_P0->OUTCLR; \
        for (int _i = 0; _i < (n); _i++) { \
            *outset = CLK_MASK; \
            clk_delay_fast(); \
            *outclr = CLK_MASK; \
            clk_delay_fast(); \
        } \
    } while(0)

// ============================================================================
// SMF init state callbacks (static member functions for private access)
// ============================================================================

void EmmcDriver::smf_power_on_entry(void* o) {
    LOG_INF("eMMC init: THGBMNG5D1LBAIL (4GB, JEDEC v5.0, 1-bit mode)");
}

smf_state_result EmmcDriver::smf_power_on_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    m.driver->pin_init();
    smf::set_state(m.ctx, init_states[INIT_CMD0]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd0_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD0 (GO_IDLE_STATE)");
    if (!m.driver->cmd0_go_idle()) {
        LOG_ERR("eMMC: CMD0 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }
    smf::set_state(m.ctx, init_states[INIT_CMD1_POLL]);
    return SMF_EVENT_HANDLED;
}

void EmmcDriver::smf_cmd1_poll_entry(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    m.cmd1_attempts = 0;
}

smf_state_result EmmcDriver::smf_cmd1_poll_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);

    bool ready = false;
    if (!m.driver->cmd1_send_op_cond_once(&ready)) {
        LOG_ERR("eMMC: CMD1 send failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }

    if (ready) {
        smf::set_state(m.ctx, init_states[INIT_CMD2]);
        return SMF_EVENT_HANDLED;
    }

    if (++m.cmd1_attempts >= 1000) {
        LOG_ERR("eMMC CMD1: card not ready after 1000 attempts");
        smf::terminate(m.ctx, -ETIMEDOUT);
        return SMF_EVENT_HANDLED;
    }

    // HYBRID POLLING STRATEGY:
    // For the first 30 attempts (~3ms), use a hardware busy-wait. 
    // This entirely bypasses the RTOS scheduler and catches fast-booting cards
    // instantly, shaving massive amounts of latency off your cold boot.
    if (m.cmd1_attempts < 30) {
        k_busy_wait(100); 
    } 
    // If the card is unusually slow (needs >3ms), fall back to yielding the 
    // thread so we don't trigger hardware watchdogs or starve other threads.
    else {
        k_sleep(K_MSEC(1)); 
    }

    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd2_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD2 (ALL_SEND_CID)");
    if (!m.driver->cmd2_all_send_cid()) {
        LOG_ERR("eMMC: CMD2 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }
    smf::set_state(m.ctx, init_states[INIT_CMD3]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd3_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD3 (SET_RELATIVE_ADDR)");
    if (!m.driver->cmd3_set_relative_addr()) {
        LOG_ERR("eMMC: CMD3 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }
    smf::set_state(m.ctx, init_states[INIT_CMD9]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd9_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD9 (SEND_CSD)");
    if (!m.driver->cmd9_send_csd()) {
        LOG_ERR("eMMC: CMD9 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }
    smf::set_state(m.ctx, init_states[INIT_CMD7]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd7_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD7 (SELECT_CARD)");
    if (!m.driver->cmd7_select_card()) {
        LOG_ERR("eMMC: CMD7 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }
    smf::set_state(m.ctx, init_states[INIT_CMD8]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd8_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD8 (SEND_EXT_CSD)");
    if (!m.driver->cmd8_send_ext_csd(m.ext_csd)) {
        LOG_ERR("eMMC: CMD8 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }

    // Extract capacity from EXT_CSD[212..215] = SEC_COUNT
    m.driver->capacity_blocks_ =
        ((uint32_t)m.ext_csd[215] << 24) |
        ((uint32_t)m.ext_csd[214] << 16) |
        ((uint32_t)m.ext_csd[213] << 8) |
        (uint32_t)m.ext_csd[212];

    LOG_DBG("eMMC capacity: %u blocks (%u MB)",
            m.driver->capacity_blocks_,
            m.driver->capacity_blocks_ / 2048);

    smf::set_state(m.ctx, init_states[INIT_CMD16]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_cmd16_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    LOG_DBG("  CMD16 (SET_BLOCKLEN)");
    if (!m.driver->cmd16_set_blocklen(512)) {
        LOG_ERR("eMMC: CMD16 failed");
        smf::terminate(m.ctx, -EIO);
        return SMF_EVENT_HANDLED;
    }
    smf::set_state(m.ctx, init_states[INIT_VERIFY]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_verify_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    uint32_t status = 0;
    m.driver->cmd13_send_status(&status);
    uint8_t card_state = (status >> 9) & 0x0F;
    LOG_DBG("eMMC card state: %u (4=TRANSFER)", card_state);

    m.driver->initialized_ = true;
    smf::set_state(m.ctx, init_states[INIT_SETUP_ASYNC]);
    return SMF_EVENT_HANDLED;
}

smf_state_result EmmcDriver::smf_setup_async_run(void* o) {
    auto& m = smf::ctx<InitMachine>(o);
    m.driver->setup_async_irqs();
    LOG_INF("eMMC init complete! RCA=0x%04x", m.driver->rca_);
    smf::terminate(m.ctx, 1);  // success (non-zero terminates the run loop)
    return SMF_EVENT_HANDLED;
}

// State table — flat (no parent, no initial transitions).
const smf_state EmmcDriver::init_states[INIT_STATE_COUNT] = {
    [INIT_POWER_ON]    = SMF_CREATE_STATE(smf_power_on_entry, smf_power_on_run, NULL, NULL, NULL),
    [INIT_CMD0]        = SMF_CREATE_STATE(NULL, smf_cmd0_run, NULL, NULL, NULL),
    [INIT_CMD1_POLL]   = SMF_CREATE_STATE(smf_cmd1_poll_entry, smf_cmd1_poll_run, NULL, NULL, NULL),
    [INIT_CMD2]        = SMF_CREATE_STATE(NULL, smf_cmd2_run, NULL, NULL, NULL),
    [INIT_CMD3]        = SMF_CREATE_STATE(NULL, smf_cmd3_run, NULL, NULL, NULL),
    [INIT_CMD9]        = SMF_CREATE_STATE(NULL, smf_cmd9_run, NULL, NULL, NULL),
    [INIT_CMD7]        = SMF_CREATE_STATE(NULL, smf_cmd7_run, NULL, NULL, NULL),
    [INIT_CMD8]        = SMF_CREATE_STATE(NULL, smf_cmd8_run, NULL, NULL, NULL),
    [INIT_CMD16]       = SMF_CREATE_STATE(NULL, smf_cmd16_run, NULL, NULL, NULL),
    [INIT_VERIFY]      = SMF_CREATE_STATE(NULL, smf_verify_run, NULL, NULL, NULL),
    [INIT_SETUP_ASYNC] = SMF_CREATE_STATE(NULL, smf_setup_async_run, NULL, NULL, NULL),
};

// ============================================================================
// Constructor / GPIO config
// ============================================================================

EmmcDriver::EmmcDriver() {
    k_sem_init(&async_done_sem_, 0, 1);
}

void EmmcDriver::configure_control_gpios(const gpio_dt_spec& supply_pin,
                                         const gpio_dt_spec& reset_pin) {
    supply_pin_ = supply_pin;
    reset_pin_ = reset_pin;
    control_gpios_configured_ = true;
}

// ============================================================================
// Zero-Overhead Compile-Time CRC Table
// ============================================================================
constexpr std::array<uint8_t, 256> build_crc7_table() {
    std::array<uint8_t, 256> t{};
    for (int idx = 0; idx < 256; ++idx) {
        uint8_t crc = static_cast<uint8_t>(idx);
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80U) ? static_cast<uint8_t>((crc << 1) ^ 0x12U)
                                : static_cast<uint8_t>(crc << 1);
        }
        t[idx] = static_cast<uint8_t>((crc >> 1) & 0x7FU);
    }
    return t;
}

static constexpr auto crc7_table = build_crc7_table();

uint8_t EmmcDriver::crc7(const uint8_t* data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; ++i) {
        crc = crc7_table[((crc << 1) ^ data[i]) & 0xFFU];
    }
    return crc;
}

// ============================================================================
// GPIO Pin Management
// ============================================================================

void EmmcDriver::pin_init() {
    if (!control_gpios_configured_ || supply_pin_.port == nullptr || reset_pin_.port == nullptr) {
        LOG_ERR("eMMC power/reset GPIOs not configured");
        return;
    }

    if (!gpio_is_ready_dt(&supply_pin_) || !gpio_is_ready_dt(&reset_pin_)) {
        LOG_ERR("eMMC power/reset GPIOs not ready");
        return;
    }

    // VccQ enable — power the eMMC I/O supply
    (void)gpio_pin_configure_dt(&supply_pin_, GPIO_OUTPUT_ACTIVE);

    // Reset — assert reset (active-low, drive LOW)
    (void)gpio_pin_configure_dt(&reset_pin_, GPIO_OUTPUT_ACTIVE);
    k_sleep(K_MSEC(1));

    // Release reset
    (void)gpio_pin_set_dt(&reset_pin_, 0);
    k_sleep(K_MSEC(5));  // eMMC boot time

    // CLK (P0.06) — output, high-drive for 32 MHz stability
    nrf_gpio_cfg(PIN_CLK,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_H0H1,
                 NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(PIN_CLK);

    // CMD (P0.08) — output, high-drive
    nrf_gpio_cfg(PIN_CMD,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_H0H1,
                 NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_set(PIN_CMD);

    // DAT0 (P0.07) — input with pull-up (eMMC idles high)
    nrf_gpio_cfg_input(PIN_DAT0, NRF_GPIO_PIN_PULLUP);
}

static bool wait_for_dat0_ready_fast(uint32_t max_cycles) {
    constexpr uint32_t kClkMask = 1U << 6U;
    constexpr uint32_t kDat0Mask = 1U << 7U;
    
    // Cache the hardware registers locally
    volatile uint32_t* outset = &NRF_P0->OUTSET;
    volatile uint32_t* outclr = &NRF_P0->OUTCLR;
    const volatile uint32_t* in_reg = &NRF_P0->IN;

    for (uint32_t i = 0; i < max_cycles; ++i) {
        *outset = kClkMask;
        clk_delay_fast();
        const uint32_t dat0 = (*in_reg & kDat0Mask);
        *outclr = kClkMask;
        clk_delay_fast();
        if (dat0 != 0U) {
            return true;
        }
    }
    return false;
}

void EmmcDriver::clock_pulse() {
    NRF_P0->OUTSET = CLK_MASK;
    clk_delay();
    NRF_P0->OUTCLR = CLK_MASK;
    clk_delay();
}

void EmmcDriver::clock_pulses(int n) {
    for (int i = 0; i < n; i++) {
        NRF_P0->OUTSET = CLK_MASK;
        clk_delay();
        NRF_P0->OUTCLR = CLK_MASK;
        clk_delay();
    }
}

void EmmcDriver::send_bits(uint64_t data, int nbits) {
    for (int i = nbits - 1; i >= 0; i--) {
        if ((data >> i) & 1) {
            NRF_P0->OUTSET = CMD_MASK;
        } else {
            NRF_P0->OUTCLR = CMD_MASK;
        }
        clock_pulse();
    }
}

uint64_t EmmcDriver::receive_bits(int nbits) {
    NRF_P0->PIN_CNF[PIN_CMD] = CMD_INPUT_PULLUP_CNF;

    uint64_t result = 0;
    for (int i = 0; i < nbits; i++) {
        NRF_P0->OUTSET = CLK_MASK;
        clk_delay();
        result = (result << 1) | (((NRF_P0->IN >> PIN_CMD) & 1U) ? 1U : 0U);
        NRF_P0->OUTCLR = CLK_MASK;
        clk_delay();
    }

    NRF_P0->PIN_CNF[PIN_CMD] = CMD_OUTPUT_CNF;
    nrf_gpio_pin_set(PIN_CMD);

    return result;
}

// ============================================================================
// eMMC Command Protocol
// ============================================================================
__ramfunc void EmmcDriver::execute_fast_blast(uint32_t w0, uint32_t w1) {
    volatile uint32_t* outset = &NRF_P0->OUTSET;
    volatile uint32_t* outclr = &NRF_P0->OUTCLR;

    unsigned int key = irq_lock();

    const uint32_t cmd_mask = CMD_MASK;
    const uint32_t clk_mask = CLK_MASK;

    #pragma GCC unroll 32
    for (int i = 0; i < 32; i++) {
        if (w0 & 0x80000000U) {
            *outset = cmd_mask;
        } else {
            *outclr = cmd_mask;
        }
        w0 <<= 1;
        *outset = clk_mask;
        __NOP(); __NOP(); 
        *outclr = clk_mask;
        __NOP(); __NOP();
    }

    #pragma GCC unroll 16
    for (int i = 0; i < 16; i++) {
        if (w1 & 0x80000000U) {
            *outset = cmd_mask;
        } else {
            *outclr = cmd_mask;
        }
        w1 <<= 1;
        *outset = clk_mask;
        __NOP(); __NOP(); 
        *outclr = clk_mask;
        __NOP(); __NOP();
    }

    irq_unlock(key);
}

bool EmmcDriver::send_command(uint8_t cmd_index, uint32_t argument,
    uint8_t * resp_buf, int resp_bits) {
    // Build 6-byte command frame:
    // [0 1 cmd_index(6)] [argument(32)] [crc7(7) 1]
    uint8_t cmd_frame[6];
    cmd_frame[0] = 0x40 | (cmd_index & 0x3F);
    cmd_frame[1] = (argument >> 24) & 0xFF;
    cmd_frame[2] = (argument >> 16) & 0xFF;
    cmd_frame[3] = (argument >> 8) & 0xFF;
    cmd_frame[4] = argument & 0xFF;
    cmd_frame[5] = (crc7(cmd_frame, 5) << 1) | 0x01;

    const bool fast_tx = initialized_;

    if (fast_tx) {
        // --- 1. HIGH-SPEED BRANCHLESS TRANSMIT ---
        // Pack directly into 2x 32-bit registers avoiding slow 384-byte array builds
        uint32_t w0 = ((uint32_t)cmd_frame[0] << 24) |
                      ((uint32_t)cmd_frame[1] << 16) |
                      ((uint32_t)cmd_frame[2] << 8)  |
                      (uint32_t)cmd_frame[3];
        uint32_t w1 = ((uint32_t)cmd_frame[4] << 24) |
                      ((uint32_t)cmd_frame[5] << 16);
        
        execute_fast_blast(w0, w1);
    } else {
        // --- 2. STANDARD SPEED TRANSMIT (Initialization) ---
        for (int byte_idx = 0; byte_idx < 6; byte_idx++) {
            for (int bit = 7; bit >= 0; bit--) {
                if ((cmd_frame[byte_idx] >> bit) & 1) {
                    NRF_P0->OUTSET = CMD_MASK;
                } else {
                    NRF_P0->OUTCLR = CMD_MASK;
                }
                clock_pulse();
            }
        }
    }

    if (resp_bits == 0) {
        if (fast_tx) {
            CLOCK_PULSES_FAST(8);
        } else {
            clock_pulses(8);
        }
        return true;
    }

    // --- 3. BIT-BANG CMD RESPONSE ---
    bool got_response = false;
    {
        NRF_P0 -> PIN_CNF[PIN_CMD] = CMD_INPUT_PULLUP_CNF;

        // Cache pointers for the RX loop to speed up read times
        volatile uint32_t* outset = &NRF_P0->OUTSET;
        volatile uint32_t* outclr = &NRF_P0->OUTCLR;
        const volatile uint32_t* in_reg = &NRF_P0->IN;

        bool got_start = false;

        // Note: We DO NOT lock interrupts here because waiting for the start bit 
        // could take up to 10,000 cycles, which would crash the I2S audio DMA.
        // Hoisted pipeline-breaking CPU branches completely outside the tight looping scope
        if (fast_tx) {
            for (int i = 0; i < 10000; i++) {
                *outset = CLK_MASK; clk_delay_fast();
                const uint32_t val = (*in_reg >> PIN_CMD) & 1U;
                *outclr = CLK_MASK; clk_delay_fast();

                if (val == 0) {
                    got_start = true;
                    break;
                }
            }
        } else {
            for (int i = 0; i < 10000; i++) {
                *outset = CLK_MASK; clk_delay();
                const uint32_t val = (*in_reg >> PIN_CMD) & 1U;
                *outclr = CLK_MASK; clk_delay();

                if (val == 0) {
                    got_start = true;
                    break;
                }
            }
        }

        if (got_start) {
            const int remaining = resp_bits - 1;
            memset(resp_buf, 0, (resp_bits + 7) / 8);

            if (fast_tx) {
                // Safely lock interrupts *only* during the highly sensitive 5µs data read window 
                // to lock out hardware scheduler jitter
                unsigned int key = irq_lock();
                
                int byte_idx = 0;
                int bit_idx = 6;
                for (int i = 0; i < remaining; i++) {
                    *outset = CLK_MASK;
                    clk_delay_fast();
                    const uint32_t bit_val = (*in_reg >> PIN_CMD) & 1U;
                    *outclr = CLK_MASK;
                    clk_delay_fast();

                    // Replaced heavy mathematical CPU division / modulo cycles with state trackers
                    if (bit_val != 0U) {
                        resp_buf[byte_idx] |= static_cast<uint8_t>(1U << bit_idx);
                    }
                    if (--bit_idx < 0) {
                        bit_idx = 7;
                        byte_idx++;
                    }
                }
                irq_unlock(key);
            } else {
                int byte_idx = 0;
                int bit_idx = 6;
                for (int i = 0; i < remaining; i++) {
                    *outset = CLK_MASK;
                    clk_delay();
                    const uint32_t bit_val = (*in_reg >> PIN_CMD) & 1U;
                    *outclr = CLK_MASK;
                    clk_delay();

                    if (bit_val != 0U) {
                        resp_buf[byte_idx] |= static_cast<uint8_t>(1U << bit_idx);
                    }
                    if (--bit_idx < 0) {
                        bit_idx = 7;
                        byte_idx++;
                    }
                }
            }
            got_response = true;
        }
    }

    NRF_P0 -> PIN_CNF[PIN_CMD] = CMD_OUTPUT_CNF;
    nrf_gpio_pin_set(PIN_CMD);

    if (!got_response) {
        LOG_ERR("CMD%u: no response", cmd_index);
        return false;
    }

    if (fast_tx) {
        CLOCK_PULSES_FAST(8);
    } else {
        clock_pulses(8);
    }
    
    return true;
}

// ============================================================================
// SPIM3 Data Read Layer
// ============================================================================

void EmmcDriver::spim3_init_once() {
    if (spim3_configured_) {
        return;
    }
    NRF_SPIM3->ENABLE = SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos;
    NRF_SPIM3->PSEL.SCK  = PIN_CLK;
    NRF_SPIM3->PSEL.MISO = PIN_DAT0;
    NRF_SPIM3->PSEL.MOSI = 0xFFFFFFFF;
    NRF_SPIM3->PSEL.CSN  = 0xFFFFFFFF;
    NRF_SPIM3->PSELDCX   = 0xFFFFFFFF;
    NRF_SPIM3->CONFIG = 2;
    NRF_SPIM3->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;
    NRF_SPIM3->RXD.LIST = 0;
    NRF_SPIM3->TXD.MAXCNT = 0;
    NRF_SPIM3->TXD.PTR = 0;
    NRF_SPIM3->IFTIMING.RXDELAY = 0;
    NRF_SPIM3->IFTIMING.CSNDUR = 0;
    NRF_SPIM3->SHORTS = 0;
    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->INTENCLR = 0xFFFFFFFF;

    nrf_gpio_cfg(PIN_CLK,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_H0H1,
                 NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg_input(PIN_DAT0, NRF_GPIO_PIN_PULLUP);
    spim3_configured_ = true;
}

void EmmcDriver::spim3_enable() {
    spim3_init_once();
    NRF_SPIM3->ENABLE = SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos;
}

void EmmcDriver::spim3_disable() {
    NRF_SPIM3->ENABLE = SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos;
}

void EmmcDriver::spim3_setup() {
    spim3_enable();
}

void EmmcDriver::spim3_set_miso_pin(uint32_t pin) {
    const bool was_enabled =
        ((NRF_SPIM3->ENABLE >> SPIM_ENABLE_ENABLE_Pos) & SPIM_ENABLE_ENABLE_Msk) != 0U;
    if (was_enabled) {
        spim3_disable();
    }
    NRF_SPIM3->PSEL.MISO = pin;
    if (pin == PIN_DAT0) {
        nrf_gpio_cfg_input(PIN_DAT0, NRF_GPIO_PIN_PULLUP);
    } else if (pin == PIN_CMD) {
        NRF_P0->PIN_CNF[PIN_CMD] = CMD_INPUT_PULLUP_CNF;
    }
    if (was_enabled) {
        spim3_enable();
    }
}

bool EmmcDriver::spim3_receive_bytes(uint8_t* buffer, size_t length) {
    if (buffer == nullptr || length == 0U) {
        return false;
    }

    NRF_SPIM3->RXD.PTR = reinterpret_cast<uint32_t>(buffer);
    NRF_SPIM3->RXD.MAXCNT = length;

    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->TASKS_START = 1;

    // A tight hardware polling loop on a 64MHz Cortex-M takes ~4-5 cycles.
    // 2,000,000 iterations guarantees a >100ms timeout with zero RTOS jitter.
    uint32_t timeout_loops = 2000000U;

    while (NRF_SPIM3->EVENTS_ENDRX == 0) {
        if (--timeout_loops == 0) {
            LOG_ERR("SPIM3 receive block timeout");
            return false;
        }
    }

    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    return true;
}

bool EmmcDriver::spim3_receive_block(uint8_t* buffer) {
    if (!spim3_receive_bytes(async_block_buffer_, BLOCK_SIZE + 2U)) {
        return false;
    }
    memcpy(buffer, async_block_buffer_, BLOCK_SIZE);
    return true;
}

bool EmmcDriver::spim3_receive_block_direct(uint8_t* buffer) {
    if (buffer == nullptr) return false;
    // Zero-copy direct DMA. The 2-byte CRC bleed is safely handled 
    // by the cmd18_read_multiple N-1 branching logic!
    return spim3_receive_bytes(buffer, BLOCK_SIZE + 2U);
}

void EmmcDriver::start_async_block_receive() {
    if (async_buffer_ == nullptr || async_next_block_index_ >= async_num_blocks_) {
        async_success_ = false;
        async_state_.store(AsyncReadState::Done, std::memory_order_release);
        k_sem_give(&async_done_sem_);
        return;
    }

    // DMA directly into caller's buffer for ALL blocks.
    // Caller guarantees (num_blocks * BLOCK_SIZE) + 2 bytes of space,
    // so the 2-byte CRC tail of the last block won't overrun.
    NRF_SPIM3->RXD.PTR = reinterpret_cast<uint32_t>(
        async_buffer_ + (async_next_block_index_ * BLOCK_SIZE));
    NRF_SPIM3->RXD.MAXCNT = BLOCK_SIZE + 2;
    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->INTENCLR = 0xFFFFFFFF;
    NRF_SPIM3->INTENSET = SPIM_INTENSET_ENDRX_Msk;
    spim3_enable();
    NRF_SPIM3->TASKS_START = 1;
}

void EmmcDriver::start_async_wait_burst() {
    async_waiting_for_block_ = true;
    async_wait_bursts_.fetch_add(1U, std::memory_order_relaxed);
    NRF_SPIM3->INTENCLR = 0xFFFFFFFF;
    spim3_disable();
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NRF_PWM0->EVENTS_STOPPED = 0;
    NRF_PWM0->INTENSET = 2;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;
}

bool EmmcDriver::wait_for_data_start_burst_sync() {
    NRF_SPIM3->INTENCLR = 0xFFFFFFFF;
    spim3_disable();
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NRF_PWM0->EVENTS_STOPPED = 0;
    NRF_PWM0->INTENCLR = 2;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;

    // ~10,000,000 loops guarantees a >500ms multi-block burst initialization timeout.
    uint32_t timeout_loops = 10000000U;
    bool timeout_occurred = false;

    while (NRF_PWM0->EVENTS_STOPPED == 0) {
        if (--timeout_loops == 0) {
            timeout_occurred = true;
            break;
        }
    }

    NRF_PWM0->EVENTS_STOPPED = 0;
    const bool got_start = NRF_GPIOTE->EVENTS_IN[0] != 0;
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    
    if (!got_start || timeout_occurred) {
        return false;
    }

    return true;
}

void EmmcDriver::spim3_teardown() {
    spim3_disable();
}

void EmmcDriver::finish_async_read(bool success) {
    async_waiting_for_block_ = false;
    NRF_PWM0->INTENCLR = 2;
    spim3_teardown();
    CLOCK_PULSES_FAST(16);
    async_success_ = success;
    if (success) {
        async_reads_completed_.fetch_add(1U, std::memory_order_relaxed);
    }
}

void EmmcDriver::spim3_irq_handler(const void* arg) {
    auto* self = reinterpret_cast<EmmcDriver*>(const_cast<void*>(arg));
    if (self == nullptr) {
        self = g_async_emmc_instance;
    }
    if (self == nullptr) {
        return;
    }

    if (self->async_state_.load(std::memory_order_acquire) != EmmcDriver::AsyncReadState::Busy) {
        return;
    }

    if (NRF_SPIM3->EVENTS_ENDRX == 0) {
        return;
    }
    self->async_block_irqs_.fetch_add(1U, std::memory_order_relaxed);
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->ENABLE = 0;

    if (self->async_buffer_ == nullptr || self->async_next_block_index_ >= self->async_num_blocks_) {
        self->async_success_ = false;
        self->async_state_.store(EmmcDriver::AsyncReadState::Done, std::memory_order_release);
        k_sem_give(&self->async_done_sem_);
        return;
    }

    self->async_next_block_index_++;
    if (self->async_next_block_index_ >= self->async_num_blocks_) {
        self->async_success_ = true;
        self->async_state_.store(EmmcDriver::AsyncReadState::Done, std::memory_order_release);
        k_sem_give(&self->async_done_sem_);
        return;
    }

    self->start_async_wait_burst();
}

void EmmcDriver::pwm0_irq_handler(const void* arg) {
    auto* self = reinterpret_cast<EmmcDriver*>(const_cast<void*>(arg));
    if (self == nullptr) {
        self = g_async_emmc_instance;
    }
    if (self == nullptr) {
        return;
    }

    if (NRF_PWM0->EVENTS_STOPPED == 0) {
        return;
    }
    self->async_pwm_stops_.fetch_add(1U, std::memory_order_relaxed);
    NRF_PWM0->EVENTS_STOPPED = 0;
    NRF_PWM0->INTENCLR = 2;

    if (!self->async_waiting_for_block_ ||
        self->async_state_.load(std::memory_order_acquire) != EmmcDriver::AsyncReadState::Busy) {
        return;
    }

    self->async_waiting_for_block_ = false;
    self->start_async_block_receive();
}

bool EmmcDriver::wait_for_data_start() {
    volatile uint32_t* outset = &NRF_P0->OUTSET;
    volatile uint32_t* outclr = &NRF_P0->OUTCLR;
    const volatile uint32_t* in_reg = &NRF_P0->IN;

    // This loop includes 16 NOPs + GPIO ops (~25 cycles per iteration).
    // 2,000,000 loops * 25 cycles = 50,000,000 cycles ≈ 780ms at 64MHz.
    uint32_t timeout_loops = 2000000U;

    while (true) {
        *outset = CLK_MASK;
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        uint32_t val = (*in_reg >> PIN_DAT0) & 1U;
        *outclr = CLK_MASK;
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        
        if (val == 0) {
            return true;
        }

        if (--timeout_loops == 0) {
            LOG_ERR("eMMC: timeout waiting for data start bit");
            return false;
        }
    }
}

// ============================================================================
// High-Level eMMC Commands
// ============================================================================

bool EmmcDriver::cmd0_go_idle() {
    nrf_gpio_pin_set(PIN_CMD);
    clock_pulses(80);
    return send_command(0, 0x00000000, nullptr, 0);
}

bool EmmcDriver::cmd1_send_op_cond_once(bool* ready) {
    const uint32_t ocr_arg = 0x40FF8080;
    uint8_t resp[6];

    if (!send_command(1, ocr_arg, resp, 48)) {
        return false;
    }

    uint32_t ocr = r1_status(resp);
    if (ocr & 0x80000000) {
        bool high_capacity = (ocr & 0x40000000) != 0;
        LOG_DBG("eMMC CMD1: ready, high_capacity=%d", high_capacity);
        *ready = true;
    } else {
        *ready = false;
    }
    return true;
}

bool EmmcDriver::cmd2_all_send_cid() {
    uint8_t resp[17];
    if (!send_command(2, 0x00000000, resp, 136)) {
        return false;
    }

    uint8_t mid = resp[1];
    char product_name[7] = {};
    memcpy(product_name, &resp[3], 6);

    LOG_DBG("eMMC CID: MID=0x%02x Product=%s", mid, product_name);
    return true;
}

bool EmmcDriver::cmd3_set_relative_addr() {
    rca_ = 0x5A7E;
    uint32_t arg = ((uint32_t)rca_ << 16);

    uint8_t resp[6];
    if (!send_command(3, arg, resp, 48)) {
        return false;
    }

    LOG_DBG("eMMC CMD3: RCA=0x%04x assigned", rca_);
    return true;
}

bool EmmcDriver::cmd7_select_card() {
    uint8_t resp[6];
    uint32_t arg = ((uint32_t)rca_ << 16);

    // 100 attempts * 100µs = 10ms timeout
    for (int attempt = 0; attempt < 100; attempt++) {
        if (send_command(7, arg, resp, 48)) {
            LOG_DBG("eMMC CMD7: card selected (attempt %d)", attempt + 1);
            return true;
        }
        
        // Use a fast hardware wait. CMD7 usually succeeds instantly, 
        // yielding to the RTOS here is purely detrimental.
        k_busy_wait(100); 
    }

    LOG_ERR("eMMC CMD7: select failed");
    return false;
}

bool EmmcDriver::cmd8_send_ext_csd(uint8_t* buffer) {
    uint8_t resp[6];
    if (!send_command(8, 0x00000000, resp, 48)) {
        return false;
    }

    if (!wait_for_data_start()) {
        return false;
    }

    spim3_setup();
    if (!spim3_receive_block(buffer)) {
        spim3_teardown();
        return false;
    }
    spim3_teardown();

    return true;
}

bool EmmcDriver::cmd9_send_csd() {
    uint8_t resp[17];
    uint32_t arg = ((uint32_t)rca_ << 16);
    if (!send_command(9, arg, resp, 136)) {
        return false;
    }

    LOG_DBG("eMMC CMD9: CSD received");
    return true;
}

bool EmmcDriver::cmd13_send_status(uint32_t* status) {
    uint8_t resp[6];
    uint32_t arg = ((uint32_t)rca_ << 16);
    if (!send_command(13, arg, resp, 48)) {
        return false;
    }
    if (status) {
        *status = r1_status(resp);
    }
    return true;
}

bool EmmcDriver::cmd12_stop_transmission() {
    uint8_t resp[6];
    return send_command(12, 0x00000000, resp, 48);
}

bool EmmcDriver::cmd16_set_blocklen(uint32_t len) {
    uint8_t resp[6];
    if (!send_command(16, len, resp, 48)) {
        return false;
    }
    LOG_DBG("eMMC CMD16: block length set to %u", len);
    return true;
}

bool EmmcDriver::cmd17_read_single(uint32_t block_addr, uint8_t* buffer) {
    uint8_t resp[6];
    if (!send_command(17, block_addr, resp, 48)) {
        return false;
    }

    if (async_irq_connected_) {
        if (!wait_for_data_start_burst_sync()) {
            CLOCK_PULSES_FAST(16);
            return false;
        }
    } else {
        if (!wait_for_data_start()) {
            CLOCK_PULSES_FAST(16);
            return false;
        }
    }

    spim3_setup();
    if (!spim3_receive_block(buffer)) {
        spim3_teardown();
        CLOCK_PULSES_FAST(16);
        return false;
    }
    spim3_teardown();
    CLOCK_PULSES_FAST(16);
    return true;
}

bool EmmcDriver::cmd23_set_block_count(uint32_t count) {
    uint8_t resp[6];
    return send_command(23, count, resp, 48);
}

bool EmmcDriver::cmd18_read_multiple(uint32_t block_addr, uint8_t* buffer,
                                     uint32_t num_blocks) {
    uint8_t resp[6];
    if (!send_command(18, block_addr, resp, 48)) {
        return false;
    }

    spim3_init_once();

    for (uint32_t i = 0; i < num_blocks; i++) {
        if (!wait_for_data_start_burst_sync()) {
            spim3_teardown();
            LOG_ERR("eMMC: timeout waiting for multi-block start");
            return false;
        }

        spim3_enable();
        if (i + 1U < num_blocks) {
            if (!spim3_receive_block_direct(buffer + (i * BLOCK_SIZE))) {
                spim3_teardown();
                LOG_ERR("eMMC: sync multi-block DMA timeout");
                return false;
            }
        } else {
            if (!spim3_receive_block(buffer + (i * BLOCK_SIZE))) {
                spim3_teardown();
                LOG_ERR("eMMC: sync multi-block DMA timeout");
                return false;
            }
        }
    }

    spim3_teardown();
    CLOCK_PULSES_FAST(16);
    return true;
}

// ============================================================================
// Async IRQ / PWM / PPI setup — factored out of init() for SMF
// ============================================================================

void EmmcDriver::setup_async_irqs() {
    if (async_irq_connected_) {
        return;
    }

    g_async_emmc_instance = this;
    async_waiting_for_block_ = false;
    async_next_block_index_ = 0;
    async_success_ = false;

    // One-time SPIM3 round-trip to latch pin config.
    spim3_setup();
    spim3_teardown();

    NRF_PWM0->PSEL.OUT[0] =
        (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos) |
        (0U << PWM_PSEL_OUT_PORT_Pos) |
        (PIN_CLK << PWM_PSEL_OUT_PIN_Pos);
    for (int i = 1; i < 4; ++i) {
        NRF_PWM0->PSEL.OUT[i] =
            (PWM_PSEL_OUT_CONNECT_Disconnected << PWM_PSEL_OUT_CONNECT_Pos);
    }
    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    NRF_PWM0->COUNTERTOP = 8;
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->DECODER = 0;
    NRF_PWM0->SEQ[0].PTR = reinterpret_cast<uint32_t>(&async_pwm_seq_word_);
    NRF_PWM0->SEQ[0].CNT = 1;
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
    NRF_PWM0->EVENTS_STOPPED = 0;
    NRF_PWM0->INTENCLR = 0xFFFFFFFF;
    NRF_PWM0->ENABLE = 1;

    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NRF_GPIOTE->CONFIG[0] =
        (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
        (PIN_DAT0 << GPIOTE_CONFIG_PSEL_Pos) |
        (0U << GPIOTE_CONFIG_PORT_Pos) |
        (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);

    NRF_PPI->CH[0].EEP = reinterpret_cast<uint32_t>(&NRF_GPIOTE->EVENTS_IN[0]);
    NRF_PPI->CH[0].TEP = reinterpret_cast<uint32_t>(&NRF_PWM0->TASKS_STOP);
    NRF_PPI->FORK[0].TEP = 0;
    NRF_PPI->CHENSET = PPI_CHENSET_CH0_Set;

    IRQ_CONNECT(SPIM3_IRQn, 0, spim3_irq_thunk, nullptr, 0);
    IRQ_CONNECT(PWM0_IRQn, 0, pwm0_irq_thunk, nullptr, 0);
    irq_enable(SPIM3_IRQn);
    irq_enable(PWM0_IRQn);
    async_irq_connected_ = true;
}

// ============================================================================
// Public API
// ============================================================================

bool EmmcDriver::init() {
    if (!control_gpios_configured_) {
        LOG_ERR("eMMC control GPIOs are not configured");
        return false;
    }

    InitMachine m{};
    m.driver = this;
    smf::set_initial(m.ctx, init_states[INIT_POWER_ON]);

    int32_t rc;
    while ((rc = smf::run(m.ctx)) == 0) {
    }

    return rc > 0;
}

bool EmmcDriver::start_read_blocks_async(uint32_t block_addr,
                                         uint8_t* buffer,
                                         uint32_t num_blocks) {
    if (!initialized_ || buffer == nullptr || num_blocks == 0U) {
        return false;
    }

    AsyncReadState expected = AsyncReadState::Idle;
    if (!async_state_.compare_exchange_strong(expected,
                                              AsyncReadState::Busy,
                                              std::memory_order_acq_rel)) {
        return false;
    }

    async_block_addr_ = block_addr;
    async_buffer_ = buffer;
    async_num_blocks_ = num_blocks;
    async_next_block_index_ = 0;
    async_success_ = false;
    k_sem_reset(&async_done_sem_);
    async_reads_started_.fetch_add(1U, std::memory_order_relaxed);
    if (!wait_for_dat0_ready_fast(100000U)) {
        async_state_.store(AsyncReadState::Idle, std::memory_order_release);
        return false;
    }
    if (!cmd23_set_block_count(num_blocks)) {
        async_state_.store(AsyncReadState::Idle, std::memory_order_release);
        return false;
    }

    uint8_t resp[6];
    if (!send_command(18, block_addr, resp, 48)) {
        LOG_WRN("async CMD18 failed block=0x%08x blocks=%u", block_addr, num_blocks);
        async_state_.store(AsyncReadState::Idle, std::memory_order_release);
        return false;
    }

    LOG_DBG("async read started block=0x%08x blocks=%u", block_addr, num_blocks);
    start_async_wait_burst();
    return true;
}

bool EmmcDriver::poll_async_read_complete(bool* success) {
    const AsyncReadState state = async_state_.load(std::memory_order_acquire);
    if (state != AsyncReadState::Done) {
        return false;
    }

    LOG_DBG("async read complete blocks=%u success=%d", async_num_blocks_, async_success_ ? 1 : 0);
    finish_async_read(async_success_);
    if (success != nullptr) {
        *success = async_success_;
    }
    async_state_.store(AsyncReadState::Idle, std::memory_order_release);
    return true;
}

bool EmmcDriver::async_read_in_flight() const {
    return async_state_.load(std::memory_order_acquire) == AsyncReadState::Busy;
}

bool EmmcDriver::wait_for_async_read_signal(k_timeout_t timeout) {
    const int rc = k_sem_take(&async_done_sem_, timeout);
    if (rc != 0 && async_state_.load(std::memory_order_acquire) == AsyncReadState::Busy) {
        LOG_DBG("async wait timeout next=%u total=%u waiting=%d",
                async_next_block_index_,
                async_num_blocks_,
                async_waiting_for_block_ ? 1 : 0);
    }
    return rc == 0;
}

void EmmcDriver::abort_async_read() {
    const unsigned int irq_key = irq_lock();
    const AsyncReadState prior_state =
        async_state_.load(std::memory_order_acquire);
    async_waiting_for_block_ = false;
    NRF_PWM0->INTENCLR = 0xFFFFFFFF;
    NRF_PWM0->TASKS_STOP = 1;
    NRF_PWM0->EVENTS_STOPPED = 0;
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NRF_SPIM3->INTENCLR = 0xFFFFFFFF;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->EVENTS_END = 0;
    spim3_teardown();
    async_buffer_ = nullptr;
    async_num_blocks_ = 0;
    async_next_block_index_ = 0;
    async_success_ = false;
    async_state_.store(AsyncReadState::Idle, std::memory_order_release);
    k_sem_reset(&async_done_sem_);
    irq_unlock(irq_key);

    if (prior_state == AsyncReadState::Busy) {
        CLOCK_PULSES_FAST(16);
        (void)cmd12_stop_transmission();
        CLOCK_PULSES_FAST(16);
    }
}

bool EmmcDriver::read_sector(uint32_t sector_addr, uint8_t* buffer) {
    return read_blocks(sector_addr, buffer, BLOCKS_PER_SECTOR);
}

bool EmmcDriver::read_blocks_sync(uint32_t block_addr, uint8_t* buffer,
    uint32_t num_blocks) {
    if (!initialized_ || buffer == nullptr || num_blocks == 0) {
        return false;
    }
    if (num_blocks == 1U) {
        return cmd17_read_single(block_addr, buffer) ? true : false;
    }
    if (!cmd23_set_block_count(num_blocks)) {
        return false;
    }
    return cmd18_read_multiple(block_addr, buffer, num_blocks);
}

bool EmmcDriver::read_blocks(uint32_t block_addr, uint8_t* buffer,
    uint32_t num_blocks) {
    if (!initialized_) {
        LOG_ERR("eMMC: not initialized");
        return false;
    }

    if (buffer == nullptr || num_blocks == 0) {
        return false;
    }

    if (num_blocks == 1U) {
        if (!cmd17_read_single(block_addr, buffer)) {
            LOG_ERR("eMMC: CMD17 read at block 0x%08x failed", block_addr);
            return false;
        }
        return true;
    }

    // Multi-block: use the async pipeline for pipelined DMA.
    // Caller must provide (num_blocks * BLOCK_SIZE) + 2 bytes.
    if (!start_read_blocks_async(block_addr, buffer, num_blocks)) {
        // CAS failed — another async read is actively in flight.
        LOG_ERR("eMMC: async busy, cannot start new read at block 0x%08x", block_addr);
        return false;
    }

    if (!wait_for_async_read_signal(K_MSEC(2000))) {
        LOG_ERR("eMMC: async read timeout at block 0x%08x", block_addr);
        abort_async_read();
        return false;
    }

    bool success = false;
    poll_async_read_complete(&success);
    if (!success) {
        LOG_ERR("eMMC: async read at block 0x%08x failed", block_addr);
    }
    return success;
}

} // namespace hardware
