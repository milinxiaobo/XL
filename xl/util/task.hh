#pragma once

#include <iostream>
#include <functional>
#include <memory>
#include <future>
#include <utility>

class Task {
public:
    // 核心：利用可变参数模板接收任意函数 F 和参数 Args
    template <typename F, typename... Args>
    explicit Task(F&& f, Args&&... args) {
        // 1. 自动推导函数的返回值类型
        using ReturnType = typename std::invoke_result<F, Args...>::type;

        // 2. 将函数和绑定后的参数打包进 packaged_task
        // std::bind 用来把参数和函数绑定在一起，变成一个不需要参数的闭包
        auto bound_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto pkg_task   = std::make_shared<std::packaged_task<ReturnType()>>(bound_func);

        // 3. 获取与该任务关联的 future，用于后续提取返回值
        result_future = std::move(pkg_task->get_future());

        // 4. 类型擦除：用一个 void() 的 lambda 包裹住这个 task，存入类的成员变量中
        func_wrapper = [pkg_task]() {
            (*pkg_task)();
        };
    }

    // 执行任务
    void execute() {
        if (func_wrapper) {
            func_wrapper();
        }
    }

    // 获取返回值（注意：模板函数不能是虚函数，通常直接写在类内）
    template <typename T>
    std::future<T> get_future() {
        // 将通用的 std::any 或保存的 future 转换为具体类型的 future
        return std::move(std::any_cast<std::future<T>&>(result_future));
    }

    // 为了简化演示，如果不需要复杂的泛型 get_future，
    // 我们可以在外部直接拿到 future，类内部只负责执行。具体看下面的测试代码。
    std::function<void()> func_wrapper;
};
