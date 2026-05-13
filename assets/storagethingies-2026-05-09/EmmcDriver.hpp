#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include <hal/nrf_gpio.h>
#include <zephyr/kernel.h>
#include <atomic>
#include <cstdint>
#include <zephyr/toolchain.h>

namespace hardware {

/**
 * eMMC driver for THGBMNG5D1LBAIL (Toshiba 4GB, JEDEC v5.0, 1-bit mode).
 *
 * Uses GPIO bit-banging for eMMC command protocol (CMD line) and
 * SPIM3 at 32 MHz for high-speed data reads (DAT0 line).
 *
 * The initialization sequence is driven by Zephyr's State Machine Framework
 * (SMF).  Each eMMC command is a distinct state; the machine terminates with
 * 0 on success or a negative errno on failure.
 *
 * Pin mapping:
 *   P0.06 = CLK   (GPIO during commands, SPIM3 SCK during data reads)
 *   P0.07 = DAT0  (GPIO during commands, SPIM3 MISO during data reads)
 *   P0.08 = CMD   (GPIO always)
 *   reset / supply are owned by the eMMC DT node
 *
 * Reference: Tim Knapen's eMMC driver implementation for Stem Player.
 * See JESD84-B50 for eMMC 5.0 protocol specification.
 */
class EmmcDriver {
public:
    EmmcDriver();
    void configure_control_gpios(const gpio_dt_spec& supply_pin,
                                 const gpio_dt_spec& reset_pin);

    /// Run the full init sequence.  Returns true on success.
    bool init();

    /**
     * Read a sector (16 blocks x 512 bytes = 8192 bytes) from the eMMC.
     * @param sector_addr Block address (in 512-byte blocks, must be 16-aligned)
     * @param buffer Output buffer, must be at least 8192 bytes
     * @return true on success
     */
    bool read_sector(uint32_t sector_addr, uint8_t* buffer);

    /**
     * Read one or more 512-byte blocks from the eMMC.
     * @param block_addr Starting block address
     * @param buffer Output buffer, must be at least num_blocks * 512 bytes
     * @param num_blocks Number of 512-byte blocks to read
     * @return true on success
     */
    bool read_blocks_sync(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
    bool read_blocks(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
    bool start_read_blocks_async(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
    bool poll_async_read_complete(bool* success);
    bool async_read_in_flight() const;
    bool wait_for_async_read_signal(k_timeout_t timeout);
    void abort_async_read();
    uint32_t async_reads_started() const {
        return async_reads_started_.load(std::memory_order_relaxed);
    }
    uint32_t async_reads_completed() const {
        return async_reads_completed_.load(std::memory_order_relaxed);
    }
    uint32_t async_block_irqs() const {
        return async_block_irqs_.load(std::memory_order_relaxed);
    }
    uint32_t async_pwm_stops() const {
        return async_pwm_stops_.load(std::memory_order_relaxed);
    }
    uint32_t async_wait_bursts() const {
        return async_wait_bursts_.load(std::memory_order_relaxed);
    }
    static void spim3_irq_handler(const void* arg);
    static void pwm0_irq_handler(const void* arg);

    uint32_t capacity_blocks() const { return capacity_blocks_; }
    uint16_t rca() const { return rca_; }
    bool is_initialized() const { return initialized_; }

    // ---- SMF init state machine -------------------------------------------

    /// States for the eMMC initialization sequence.
    enum InitState : uint8_t {
        INIT_POWER_ON,
        INIT_CMD0,
        INIT_CMD1_POLL,
        INIT_CMD2,
        INIT_CMD3,
        INIT_CMD9,
        INIT_CMD7,
        INIT_CMD8,
        INIT_CMD16,
        INIT_VERIFY,
        INIT_SETUP_ASYNC,
        INIT_STATE_COUNT,
    };

    /// Context object whose first member is smf_ctx (required by SMF).
    struct InitMachine {
        smf_ctx ctx;
        EmmcDriver* driver;
        int cmd1_attempts;
        alignas(4) uint8_t ext_csd[512];
    };

    /// The state table — public so callers can inspect the current init state.
    static const smf_state init_states[INIT_STATE_COUNT];

private:
    // --- SMF init state callbacks (static members for private access) ---
    static void         smf_power_on_entry(void* o);
    static smf_state_result smf_power_on_run(void* o);
    static void         smf_cmd1_poll_entry(void* o);
    static smf_state_result smf_cmd0_run(void* o);
    static smf_state_result smf_cmd1_poll_run(void* o);
    static smf_state_result smf_cmd2_run(void* o);
    static smf_state_result smf_cmd3_run(void* o);
    static smf_state_result smf_cmd9_run(void* o);
    static smf_state_result smf_cmd7_run(void* o);
    static smf_state_result smf_cmd8_run(void* o);
    static smf_state_result smf_cmd16_run(void* o);
    static smf_state_result smf_verify_run(void* o);
    static smf_state_result smf_setup_async_run(void* o);

    // eMMC card state
    uint16_t rca_ = 0;
    uint32_t capacity_blocks_ = 0;
    bool initialized_ = false;

    // --- Pin definitions ---
    static constexpr uint32_t PIN_CLK  = 6;   // P0.06
    static constexpr uint32_t PIN_DAT0 = 7;   // P0.07
    static constexpr uint32_t PIN_CMD  = 8;   // P0.08
    static constexpr uint32_t CLK_MASK = 1U << PIN_CLK;
    static constexpr uint32_t CMD_MASK = 1U << PIN_CMD;
    static constexpr uint32_t DAT0_MASK = 1U << PIN_DAT0;
    static constexpr uint32_t CMD_OUTPUT_CNF =
        (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
        (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
        (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos) |
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
    static constexpr uint32_t CMD_INPUT_PULLUP_CNF =
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    static constexpr uint16_t BLOCKS_PER_SECTOR = 16;
    static constexpr uint16_t BLOCK_SIZE = 512;
    struct gpio_dt_spec supply_pin_{};
    struct gpio_dt_spec reset_pin_{};
    bool control_gpios_configured_ = false;

    // --- GPIO command layer (bit-bang) ---
    void pin_init();
    void clock_pulse();
    void clock_pulses(int n);
    void send_bits(uint64_t data, int nbits);
    uint64_t receive_bits(int nbits);

    // CRC7 for eMMC commands (polynomial x^7 + x^3 + 1)
    static uint8_t crc7(const uint8_t* data, size_t len);

    // Send an eMMC command and receive response.
    // resp_buf receives raw response bytes (excluding start/transmission bits).
    // resp_bits: expected response length (48 for R1/R3, 136 for R2, 0 for no response).
    bool send_command(uint8_t cmd_index, uint32_t argument,
                      uint8_t* resp_buf, int resp_bits);

    // --- SPIM3 data layer ---
    void spim3_init_once();
    void spim3_enable();
    void spim3_disable();
    void spim3_setup();
    void spim3_set_miso_pin(uint32_t pin);
    bool spim3_receive_bytes(uint8_t* buffer, size_t length);
    bool spim3_receive_block(uint8_t* buffer);
    bool spim3_receive_block_direct(uint8_t* buffer);
    void spim3_teardown();
    bool wait_for_data_start_burst_sync();
    bool wait_for_data_start();
    void start_async_block_receive();
    void start_async_wait_burst();
    void finish_async_read(bool success);

    // --- High-level eMMC protocol ---
    bool cmd0_go_idle();
    bool cmd1_send_op_cond_once(bool* ready);
    bool cmd2_all_send_cid();
    bool cmd3_set_relative_addr();
    bool cmd7_select_card();
    bool cmd8_send_ext_csd(uint8_t* buffer);
    bool cmd9_send_csd();
    bool cmd13_send_status(uint32_t* status);
    bool cmd12_stop_transmission();
    bool cmd16_set_blocklen(uint32_t len);
    bool cmd17_read_single(uint32_t block_addr, uint8_t* buffer);
    bool cmd18_read_multiple(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
    bool cmd23_set_block_count(uint32_t count);

    // Async read IRQ state — unchanged from previous implementation.
    enum class AsyncReadState : uint8_t {
        Idle = 0,
        Busy,
        Done,
    };

    bool async_irq_connected_ = false;
    bool async_waiting_for_block_ = false;
    uint32_t async_block_addr_ = 0;
    uint8_t* async_buffer_ = nullptr;
    uint32_t async_num_blocks_ = 0;
    uint32_t async_next_block_index_ = 0;
    bool async_success_ = false;
    bool spim3_configured_ = false;
    alignas(4) uint8_t async_block_buffer_[BLOCK_SIZE + 2] = {};
    alignas(4) uint16_t async_pwm_seq_word_ = 0x8004;
    std::atomic<AsyncReadState> async_state_{AsyncReadState::Idle};
    std::atomic<uint32_t> async_reads_started_{0};
    std::atomic<uint32_t> async_reads_completed_{0};
    std::atomic<uint32_t> async_block_irqs_{0};
    std::atomic<uint32_t> async_pwm_stops_{0};
    std::atomic<uint32_t> async_wait_bursts_{0};
    struct k_sem async_done_sem_;

    void setup_async_irqs();
    __ramfunc void execute_fast_blast(uint32_t w0, uint32_t w1);

};

} // namespace hardware
