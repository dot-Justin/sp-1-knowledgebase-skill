#pragma once

#include <zephyr/kernel.h>

namespace zpp {

class Mutex {
public:
    Mutex() {
        k_mutex_init(&mutex_);
    }

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    k_mutex* native_handle() {
        return &mutex_;
    }

    const k_mutex* native_handle() const {
        return &mutex_;
    }

    int lock(k_timeout_t timeout = K_FOREVER) {
        return k_mutex_lock(&mutex_, timeout);
    }

    int unlock() {
        return k_mutex_unlock(&mutex_);
    }

private:
    struct k_mutex mutex_{};
};

class LockGuard {
public:
    explicit LockGuard(struct k_mutex& mutex, k_timeout_t timeout = K_FOREVER)
        : mutex_(&mutex),
          locked_(k_mutex_lock(mutex_, timeout) == 0) {
    }

    explicit LockGuard(Mutex& mutex, k_timeout_t timeout = K_FOREVER)
        : LockGuard(*mutex.native_handle(), timeout) {
    }

    ~LockGuard() {
        if (locked_) {
            (void)k_mutex_unlock(mutex_);
        }
    }

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    bool locked() const {
        return locked_;
    }

private:
    struct k_mutex* mutex_ = nullptr;
    bool locked_ = false;
};

class Semaphore {
public:
    Semaphore(unsigned int initial_count = 0U, unsigned int limit = 1U) {
        k_sem_init(&sem_, initial_count, limit);
    }

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    k_sem* native_handle() {
        return &sem_;
    }

    const k_sem* native_handle() const {
        return &sem_;
    }

    int take(k_timeout_t timeout = K_FOREVER) {
        return k_sem_take(&sem_, timeout);
    }

    void give() {
        k_sem_give(&sem_);
    }

    void reset() {
        k_sem_reset(&sem_);
    }

private:
    struct k_sem sem_{};
};

class Timer {
public:
    Timer(k_timer_expiry_t expiry = nullptr, k_timer_stop_t stop = nullptr) {
        k_timer_init(&timer_, expiry, stop);
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    k_timer* native_handle() {
        return &timer_;
    }

    const k_timer* native_handle() const {
        return &timer_;
    }

    void start(k_timeout_t duration, k_timeout_t period = K_NO_WAIT) {
        k_timer_start(&timer_, duration, period);
    }

    void stop() {
        k_timer_stop(&timer_);
    }

    uint32_t status_sync() {
        return k_timer_status_sync(&timer_);
    }

private:
    struct k_timer timer_{};
};

class DelayableWork {
public:
    explicit DelayableWork(k_work_handler_t handler) {
        k_work_init_delayable(&work_, handler);
    }

    DelayableWork(const DelayableWork&) = delete;
    DelayableWork& operator=(const DelayableWork&) = delete;

    k_work_delayable* native_handle() {
        return &work_;
    }

    const k_work_delayable* native_handle() const {
        return &work_;
    }

    int schedule(k_timeout_t delay) {
        return k_work_schedule(&work_, delay);
    }

    int reschedule(k_timeout_t delay) {
        return k_work_reschedule(&work_, delay);
    }

    int cancel() {
        return k_work_cancel_delayable(&work_);
    }

private:
    struct k_work_delayable work_{};
};

}  // namespace zpp
