#include "../../include/kafka/index.h"

#include <map>
#include <chrono>

namespace kafka {
namespace {
struct TopicState {
    std::deque<std::string> messages;
    std::mutex mutex;
    std::condition_variable cv;
    bool closed{false};
};

class InMemoryBroker {
public:
    static InMemoryBroker &instance()
    {
        static InMemoryBroker broker;
        return broker;
    }

    void publish(const std::string &topic, const std::string &message)
    {
        auto state = getOrCreate(topic);
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->messages.push_back(message);
        }
        state->cv.notify_one();
    }

    std::string consume(const std::string &topic, bool stopRequested)
    {
        auto state = getOrCreate(topic);
        std::unique_lock<std::mutex> lock(state->mutex);
        state->cv.wait_for(lock, std::chrono::milliseconds(50), [&]() {
            return !state->messages.empty() || state->closed || stopRequested;
        });

        if (!state->messages.empty())
        {
            std::string value = std::move(state->messages.front());
            state->messages.pop_front();
            return value;
        }

        return {};
    }

    void close(const std::string &topic)
    {
        auto state = getOrCreate(topic);
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->closed = true;
        }
        state->cv.notify_all();
    }

private:
    std::shared_ptr<TopicState> getOrCreate(const std::string &topic)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = topics_.find(topic);
        if (it != topics_.end())
        {
            if (auto state = it->second.lock())
            {
                return state;
            }
        }
        auto state = std::make_shared<TopicState>();
        topics_[topic] = state;
        return state;
    }

    std::mutex mutex_;
    std::map<std::string, std::weak_ptr<TopicState>> topics_;
};
} // namespace

KafkaProducer::KafkaProducer(std::string bootstrapServers)
    : bootstrapServers_(std::move(bootstrapServers)),
      logger_(utils::SingletonLogger::instance())
{
    logger_.logMeta(utils::SingletonLogger::INFO,
                    "KafkaProducer connected to " + bootstrapServers_,
                    __FILE__,
                    __LINE__,
                    __func__);
}

void KafkaProducer::produceMessages(const std::string &topic, const std::string &message)
{
    logger_.logMeta(utils::SingletonLogger::DEBUG,
                    "Producing message to topic " + topic + ": " + message,
                    __FILE__,
                    __LINE__,
                    __func__);
    InMemoryBroker::instance().publish(topic, message);
}

KafkaConsumer::KafkaConsumer(std::string bootstrapServers, std::string topic)
    : bootstrapServers_(std::move(bootstrapServers)),
      topic_(std::move(topic)),
      logger_(utils::SingletonLogger::instance())
{
    logger_.logMeta(utils::SingletonLogger::INFO,
                    "KafkaConsumer subscribed to " + topic_ + " via " + bootstrapServers_,
                    __FILE__,
                    __LINE__,
                    __func__);
}

KafkaConsumer::~KafkaConsumer()
{
    stopConsumeMessages();
}

std::string KafkaConsumer::consumeMessage()
{
    return InMemoryBroker::instance().consume(topic_, stopRequested_);
}

void KafkaConsumer::stopConsumeMessages()
{
    if (!stopRequested_)
    {
        stopRequested_ = true;
        InMemoryBroker::instance().close(topic_);
    }
}

} // namespace kafka

