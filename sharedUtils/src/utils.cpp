#include "../include/utils/index.h"

namespace utils {

namespace {
constexpr const char *levelToColor(SingletonLogger::Level level)
{
    switch (level)
    {
    case SingletonLogger::DEBUG:
        return "\033[36m"; // Cyan
    case SingletonLogger::INFO:
        return "\033[32m"; // Green
    case SingletonLogger::WARNING:
        return "\033[33m"; // Yellow
    case SingletonLogger::ERROR:
    default:
        return "\033[31m"; // Red
    }
}

constexpr const char *resetColor()
{
    return "\033[0m";
}
} // namespace

SingletonLogger &SingletonLogger::instance()
{
    static SingletonLogger logger;
    return logger;
}

void SingletonLogger::logMeta(Level level,
                              const std::string &message,
                              const char *file,
                              int line,
                              const char *function)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << '[' << levelToString(level) << "] " << file << ':' << line << " (" << function << ") - " << message;
    std::cout << levelToColor(level) << oss.str() << resetColor() << std::endl;
}

std::string SingletonLogger::levelToString(Level level) const
{
    switch (level)
    {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
    default:
        return "ERROR";
    }
}

ThreadPool &ThreadPool::instance(std::size_t threadCount)
{
    static ThreadPool pool(threadCount == 0 ? std::thread::hardware_concurrency() : threadCount);
    return pool;
}

ThreadPool::ThreadPool(std::size_t threads)
{
    if (threads == 0)
    {
        threads = 1;
    }

    for (std::size_t i = 0; i < threads; ++i)
    {
        workers_.emplace_back([this]() { workerLoop(); });
    }
}

ThreadPool::~ThreadPool()
{
    shutdown();
}

void ThreadPool::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stopRequested_)
        {
            return;
        }
        stopRequested_ = true;
    }
    cv_.notify_all();

    for (auto &worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
    workers_.clear();
}

void ThreadPool::workerLoop()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stopRequested_ || !tasks_.empty(); });
            if (stopRequested_ && tasks_.empty())
            {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

} // namespace utils

