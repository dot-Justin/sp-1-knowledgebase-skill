#pragma once

/// @file Smf.hpp
/// @brief Lightweight C++ facade for Zephyr's State Machine Framework (SMF).
///
/// SMF is a C library that requires the user object's first member to be
/// `smf_ctx`.  This header provides:
///
///   smf::ctx<T>(void*)  — recover the typed object inside SMF callbacks
///   smf::set_initial()  — type-safe wrappers around the C API
///   smf::set_state()
///   smf::terminate()
///   smf::run()
///   smf::state()        — constexpr state builder (flat or hierarchical)
///
/// Usage:
///
///   struct MyMachine {
///       smf_ctx ctx;            // MUST be first member
///       int my_data = 0;
///   };
///
///   static enum smf_state_result my_run(void* o) {
///       auto& m = smf::ctx<MyMachine>(o);
///       m.my_data++;
///       smf::set_state(m.ctx, next_state);
///       return SMF_EVENT_HANDLED;
///   }
///
///   // State table — flat:
///   constexpr smf_state states[] = {
///       smf::state(my_entry, my_run, my_exit),
///   };
///
///   // State table — hierarchical (needs CONFIG_SMF_ANCESTOR_SUPPORT):
///   constexpr smf_state states[] = {
///       smf::state(parent_entry, parent_run, parent_exit, nullptr, &child),
///       smf::state(child_entry, child_run, child_exit, &parent, nullptr),
///   };

#include <zephyr/smf.h>
#include <cstdint>

namespace smf {

/// Recover the concrete state machine object from the opaque `void*`
/// that Zephyr SMF passes to entry / run / exit callbacks.
///
/// Requirement: `T` is standard-layout and `smf_ctx ctx` is its first member.
/// This mirrors the `SMF_CTX` macro but returns a typed C++ reference.
template <typename T>
inline T& ctx(void* o) {
    return *reinterpret_cast<T*>(o);
}

/// Build an smf_state at compile time.
///
/// Flat usage:     smf::state(entry, run, exit)
/// Hierarchical:   smf::state(entry, run, exit, &parent, &initial_child)
///
/// Pass nullptr for any callback/pointer you don't need.
constexpr smf_state state(
    void (*entry)(void*),
    smf_state_result (*run)(void*),
    void (*exit)(void*)
#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
    , const smf_state* parent = nullptr
#endif
#ifdef CONFIG_SMF_INITIAL_TRANSITION
    , const smf_state* initial = nullptr
#endif
) {
    return {
        .entry = entry,
        .run = run,
        .exit = exit,
#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
        .parent = parent,
#endif
#ifdef CONFIG_SMF_INITIAL_TRANSITION
        .initial = initial,
#endif
    };
}

/// Set the initial state of a machine.
inline void set_initial(smf_ctx& c, const smf_state& s) {
    smf_set_initial(&c, &s);
}

/// Transition to a new state (call from entry or run, never exit).
inline void set_state(smf_ctx& c, const smf_state& s) {
    smf_set_state(&c, &s);
}

/// Terminate the state machine.  The value is returned by `run()`.
inline void terminate(smf_ctx& c, int32_t val) {
    smf_set_terminate(&c, val);
}

/// Run one iteration.  Returns 0 while running, non-zero when terminated.
inline int32_t run(smf_ctx& c) {
    return smf_run_state(&c);
}

} // namespace smf
