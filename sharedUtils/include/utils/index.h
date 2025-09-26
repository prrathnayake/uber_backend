#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace utils {

class SingletonLogger {
public:
    enum Level {
        DEBUG = 0,
        INFO,
        WARNING,
        ERROR
    };

    static SingletonLogger &instance();

    void logMeta(Level level,
                 const std::string &message,
                 const char *file,
                 int line,
                 const char *function);

private:
    SingletonLogger() = default;
    ~SingletonLogger() = default;

    std::string levelToString(Level level) const;

    std::mutex mutex_;
};

class ThreadPool {
public:
    static ThreadPool &instance(std::size_t threadCount = std::thread::hardware_concurrency());

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stopRequested_)
            {
                throw std::runtime_error("ThreadPool has been stopped");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return res;
    }

    void shutdown();

private:
    explicit ThreadPool(std::size_t threads);
    ~ThreadPool();

    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopRequested_{false};
};

} // namespace utils

