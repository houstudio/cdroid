#ifndef __SHARED_QUEUE_H__
#define __SHARED_QUEUE_H__

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

template<typename T>
class shared_queue {
    std::queue<T> queue_;
    mutable std::mutex m_;
    std::condition_variable data_cond_;
    shared_queue& operator=(const shared_queue&); // c++11 feature not yet in vs2010 = delete;
    shared_queue(const shared_queue& other); // c++11 feature not yet in vs2010 = delete;
  public:
    shared_queue() {}

    void push(T item) {
        m_.lock();
        queue_.push(item);
        m_.unlock();
        data_cond_.notify_one();
    }

    bool try_and_pop(T& popped_item) {
        std::lock_guard<std::mutex> lock(m_);
        if(queue_.empty())
            return false;
        popped_item=std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool wait_and_pop(T& popped_item,int timeout=-1) {
        std::unique_lock<std::mutex> lock(m_); // note: unique_lock is needed for std::condition_variable::wait
        if(data_cond_.wait_for(lock,std::chrono::milliseconds(timeout),[this]() {
            return !queue_.empty();
        })) {
            popped_item=std::move(queue_.front());
            queue_.pop();
            return true;
        }
        return false;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_);
        return queue_.empty();
    }

    unsigned size() const {
        std::lock_guard<std::mutex> lock(m_);
        return queue_.size();
    }
    void clear() {
        std::lock_guard<std::mutex> lock(m_);
        while(queue_.size())queue_.pop();
    }
};

#endif
