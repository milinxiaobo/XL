#pragma once

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

namespace xl::util {
    void fn(int n)
    {
        for (int i = 0; i < 5; ++i)
        {
            std::cout << "Thread 1 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    struct task {
        template <typename F, typename... Args>
        explicit task(F&& f, Args&&... args) {
            auto bind = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            using return_t = typename std::invoke_result<F, Args...>::type;
            auto packaged_task = std::make_shared<std::packaged_task<return_t()>>(bind);
            func = [packaged_task](){ (*packaged_task)(); };
        }
        std::function<void()> func;
    };
    struct thread {
        thread() {
            std::thread t(fn, 1);
            t.join();
        }
        static void test() {
            auto func = [](int i, int j){
                std::cout << "\n" << i + j << "\n";
            };
            task a(func, 1, 2);
            a.func();
            thread t;
            t.push_back(func, 1, 2);
        }
        std::vector<task> m_task_list;
        template <typename F, typename... Args>
        void push_back(F&& f, Args&&... args) {
            m_task_list.push_back(task(f, args...));
        }
    };
    struct thread_pool {

    };
}
