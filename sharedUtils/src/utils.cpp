#include "../include/utils/index.h"

#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <system_error>
#include <iostream>
#include <sstream>
#include <sys/file.h>
#include <unistd.h>

#include "../include/utils/time.h"

namespace utils {

namespace {
std::string levelToString(SingletonLogger::Level level)
{
    switch (level)
    {
    case SingletonLogger::DEBUG:
        return "DEBUG";
    case SingletonLogger::INFO:
        return "INFO";
    case SingletonLogger::WARNING:
        return "WARNING";
    case SingletonLogger::ERROR:
    default:
        return "ERROR";
    }
}
} // namespace

std::mutex SingletonLogger::logMutex_;

SingletonLogger &SingletonLogger::instance(const std::string &logFilePath)
{
    static SingletonLogger logger(logFilePath);
    return logger;
}

SingletonLogger::SingletonLogger(const std::string &logFilePath)
    : logFilePath_(logFilePath)
{
    namespace fs = std::filesystem;

    fs::path logPath(logFilePath_);

    std::error_code ec;
    const bool exists = fs::exists(logPath, ec);
    if (ec)
    {
        std::cerr << "SingletonLogger: failed to inspect existing log file at '" << logFilePath_
                  << "': " << ec.message() << std::endl;
        ec.clear();
    }

    if (exists)
    {
        fs::remove(logPath, ec);
        if (ec)
        {
            std::cerr << "SingletonLogger: failed to remove existing log file at '" << logFilePath_
                      << "': " << ec.message() << std::endl;
            ec.clear();
        }
    }

    if (logPath.has_parent_path())
    {
        fs::create_directories(logPath.parent_path(), ec);
        if (ec)
        {
            std::cerr << "SingletonLogger: failed to create log directory '" << logPath.parent_path().string()
                      << "': " << ec.message() << std::endl;
        }
    }
}

SingletonLogger::~SingletonLogger() = default;

void SingletonLogger::log(Level level, const std::string &message)
{
    const std::string formatted = Time::logTime() + " [ " + levelToString(level) + " ] " + message;
    printLog(formatted);
    printConsole(level, formatted);
}

void SingletonLogger::logMeta(Level level,
                              const std::string &message,
                              const char *file,
                              int line,
                              const char *function)
{
    std::ostringstream metadata;
    metadata << file << ':' << line << " in " << function;

    const std::string formatted = Time::logTime() + " [ " + levelToString(level) + " ] " + message + " | " + metadata.str();
    printLog(formatted);
    printConsole(level, formatted);
}

void SingletonLogger::printLog(const std::string &message)
{
    std::lock_guard<std::mutex> lock(logMutex_);

    int fd = open(logFilePath_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        std::perror("SingletonLogger open");
        return;
    }

    if (flock(fd, LOCK_EX) == -1)
    {
        std::perror("SingletonLogger flock");
        close(fd);
        return;
    }

    const std::string line = message + '\n';
    if (write(fd, line.c_str(), line.size()) == -1)
    {
        std::perror("SingletonLogger write");
    }

    flock(fd, LOCK_UN);
    close(fd);
}

void SingletonLogger::printConsole(Level level, const std::string &message)
{
#ifdef DEBUG_MODE
    constexpr bool debugEnabled = true;
#else
    constexpr bool debugEnabled = false;
#endif

    switch (level)
    {
    case Level::ERROR:
        std::cout << "\033[1;31m[ERROR] " << message << "\033[0m" << std::endl;
        break;
    case Level::WARNING:
        std::cout << "\033[1;33m[WARNING] " << message << "\033[0m" << std::endl;
        break;
    case Level::INFO:
        std::cout << "[INFO] " << message << std::endl;
        break;
    case Level::DEBUG:
        if (debugEnabled)
        {
            std::cout << "\033[1;36m[DEBUG] " << message << "\033[0m" << std::endl;
        }
        break;
    default:
        std::cout << message << std::endl;
        break;
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

