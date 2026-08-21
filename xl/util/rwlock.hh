#pragma once
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <shared_mutex>

namespace xl::util {
    struct rwlock {
        /// std::atomic<std::int64_t> state_{0};
        std::atomic<std::ptrdiff_t> stat{0};
        void rlock() {
            auto curr = stat.load(std::memory_order_relaxed);
            while (curr < 0 || !stat.compare_exchange_weak(curr, curr+1, std::memory_order_acquire, std::memory_order_relaxed)) {
                stat.wait(-1, std::memory_order_relaxed);
                curr = stat.load(std::memory_order_relaxed);
            }
        }
        void unrlock() {
            if (stat.fetch_sub(1, std::memory_order_release) == 1) {
                stat.notify_all();
            }
        }
        void wlock() {
            int expected = 0;
            while (!stat.compare_exchange_weak(expected, -1, std::memory_order_acquire, std::memory_order_relaxed)) {
                stat.wait(expected, std::memory_order_relaxed);
                expected = 0;
            }
        }
        void unwlock() {
            stat.store(0, std::memory_order_release);
            stat.notify_all();
        }
    };
}



class ImprovedAtomicRWLock {
private:
    std::atomic<int> state_{0};

public:
    void lock_shared() {
        int current = state_.load(std::memory_order_relaxed);
        while (current < 0 || !state_.compare_exchange_weak(current, current + 1,
                                                            std::memory_order_acquire,
                                                            std::memory_order_relaxed)) {
            // C++20 阻塞机制：如果状态一直是 -1，线程就挂起休眠，不再空转死循环！
            state_.wait(-1, std::memory_order_relaxed);
            current = state_.load(std::memory_order_relaxed);
        }
    }

    void unlock_shared() {
        if (state_.fetch_sub(1, std::memory_order_release) == 1) {
            // 如果减完后读计数变成了 0，说明它是最后一个读线程，唤醒可能在等待的写线程
            state_.notify_all();
        }
    }

    void lock() {
        int expected = 0;
        while (!state_.compare_exchange_weak(expected, -1,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed)) {
            // 如果期望值不是 0（可能 >0 或 == -1），就挂起等待通知
            state_.wait(expected, std::memory_order_relaxed);
            expected = 0;
        }
    }

    void unlock() {
        state_.store(0, std::memory_order_release);
        state_.notify_all(); // 唤醒所有在等待的读或写线程
    }
};

#include <atomic>
#include <thread>

class AtomicRWLock {
private:
    // 0: 未上锁, >0: 读锁计数, -1: 写锁锁定
    std::atomic<int> state_{0};

public:
    AtomicRWLock() = default;

    // ===== 读锁操作 =====
    void lock_shared() {
        while (true) {
            int current = state_.load(std::memory_order_relaxed);

            // 如果当前有写锁（state_ == -1），继续自旋等待
            if (current < 0) {
                std::this_thread::yield(); // 让出 CPU 时间片，避免死循环占满 CPU
                continue;
            }

            // 尝试将读计数加 1
            // CAS 逻辑：如果当前值依然是 current，就把它变成 current + 1
            if (state_.compare_exchange_weak(current, current + 1,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed)) {
                break; // 成功获取读锁
            }
            // 如果 CAS 失败，说明中途有其他线程改变了 state_，循环重试
        }
    }

    void unlock_shared() {
        // 原子地将读计数减 1
        state_.fetch_sub(1, std::memory_order_release);
    }

    // ===== 写锁操作 =====
    void lock() {
        while (true) {
            int expected = 0; // 写锁要求当前必须完全空闲（0）

            // CAS 逻辑：如果当前是 0，就把它变成 -1（代表写锁）
            if (state_.compare_exchange_weak(expected, -1,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed)) {
                break; // 成功获取写锁
            }

            std::this_thread::yield(); // 失败则让出 CPU，继续自旋
        }
    }

    void unlock() {
        // 直接将状态恢复为 0
        state_.store(0, std::memory_order_release);
    }
};

#include <iostream>
#include <shared_mutex> // 核心头文件
#include <mutex>
#include <string>
#include <map>
#include <thread>

class ConfigCenter {
private:
    std::map<std::string, std::string> settings_;
    mutable std::shared_mutex rw_mtx_; // 定义一个读写锁

public:
    // 读操作：使用 std::shared_lock
    std::string getSetting(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(rw_mtx_); // 加【读锁/共享锁】

        auto it = settings_.find(key);
        if (it != settings_.end()) {
            return it->second;
        }
        return "";
    } // 离开作用域自动释放读锁

    // 写操作：使用 std::unique_lock
    void setSetting(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(rw_mtx_); // 加【写锁/独占锁】

        settings_[key] = value;
    } // 离开作用域自动释放写锁
};
