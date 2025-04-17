#ifndef REALSENSE_CAMERA_MESSAGE_SYNC_H_
#define REALSENSE_CAMERA_MESSAGE_SYNC_H_

#include <cstddef>

#include <mutex>
#include <queue>
#include <memory>
#include <functional>

namespace realsense_camera {

template <typename MessageT>
class MessageQuque
{
public:
void push(std::unique_ptr<MessageT> msg)
{
    std::lock_guard<std::mutex> m(m_mutext_);
    message_queue_.push(std::move(msg));
}

std::unique_ptr<MessageT> pop()
{
    std::lock_guard<std::mutex> m(m_mutext_);
    if(!message_queue_.empty())
    {
        std::unique_ptr<MessageT> msg = std::move(message_queue_.front());
        message_queue_.pop();
        return std::move(msg);
    }

    return std::unique_ptr<MessageT>();
}

private:
std::queue<std::unique_ptr<MessageT>> message_queue_;

std::mutex m_mutext_;
};

class MessageSync
{
public:
explicit MessageSync( float sync_threshold = 50);

private:
float sync_threshold_;

};
}

#endif