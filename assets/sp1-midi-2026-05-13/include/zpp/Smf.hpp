#pragma once

#include <zephyr/smf.h>

#include <cstdint>

namespace zpp::smf {

using Context = ::smf_ctx;
using State = ::smf_state;

enum class RunResult : uint8_t {
    Handled = SMF_EVENT_HANDLED,
    Propagate = SMF_EVENT_PROPAGATE,
};

template <typename Obj,
          void (*Entry)(Obj*) = nullptr,
          RunResult (*Run)(Obj*) = nullptr,
          void (*Exit)(Obj*) = nullptr>
struct StateDef {
    static void entry_adapter(void* obj) {
        if constexpr (Entry != nullptr) {
            Entry(static_cast<Obj*>(obj));
        }
    }

    static ::smf_state_result run_adapter(void* obj) {
        if constexpr (Run != nullptr) {
            return static_cast<::smf_state_result>(Run(static_cast<Obj*>(obj)));
        }

        return SMF_EVENT_HANDLED;
    }

    static void exit_adapter(void* obj) {
        if constexpr (Exit != nullptr) {
            Exit(static_cast<Obj*>(obj));
        }
    }

    static constexpr State create(const State* parent = nullptr,
                                  const State* initial = nullptr) {
        return SMF_CREATE_STATE(Entry != nullptr ? &entry_adapter : nullptr,
                                Run != nullptr ? &run_adapter : nullptr,
                                Exit != nullptr ? &exit_adapter : nullptr,
                                parent,
                                initial);
    }
};

class Machine {
public:
    explicit Machine(Context& ctx)
        : ctx_(&ctx) {
    }

    void set_initial(const State& state) {
        smf_set_initial(ctx_, &state);
    }

    void set_state(const State& state) {
        smf_set_state(ctx_, &state);
    }

    void set_state(const State* state) {
        smf_set_state(ctx_, state);
    }

    void terminate(int32_t value) {
        smf_set_terminate(ctx_, value);
    }

    int32_t run() {
        return smf_run_state(ctx_);
    }

    const State* current_leaf() const {
        return smf_get_current_leaf_state(ctx_);
    }

    const State* current_executing() const {
        return smf_get_current_executing_state(ctx_);
    }

    Context* native_handle() {
        return ctx_;
    }

    const Context* native_handle() const {
        return ctx_;
    }

private:
    Context* ctx_ = nullptr;
};

}  // namespace zpp::smf
