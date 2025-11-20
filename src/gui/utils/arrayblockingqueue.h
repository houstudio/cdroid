/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __ARRAY_BLOCKING_QUEUE_H__
#define __ARRAY_BLOCKING_QUEUE_H__
#include <queue>
#include <mutex>
#include <condition_variable>

namespace cdroid{

template<typename T>
class ArrayBlockingQueue {
private:
    std::queue<T> mQueue;
    std::mutex mMutex;
    std::condition_variable mNotEmpty;
    std::condition_variable mNotFull;
    size_t mCapacity;
public:
    explicit ArrayBlockingQueue(size_t capacity) : mCapacity(capacity) {
        if (mCapacity == 0) {
            throw std::invalid_argument("Capacity must be greater than zero");
        }
    }

    void add(const T& item) {
        std::unique_lock<std::mutex> lock(mMutex);
        if (mQueue.size() >= mCapacity) {
            throw std::runtime_error("Queue is full");
        }
        mQueue.push(item);
        mNotEmpty.notify_one();
    }

    void put(const T& item) {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotFull.wait(lock, [this] { return mQueue.size() < mCapacity; });
        mQueue.push(item);
        lock.unlock();
        mNotEmpty.notify_one();
    }

    T take() {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotEmpty.wait(lock, [this] { return !mQueue.empty(); });
        T item = mQueue.front();
        mQueue.pop();
        lock.unlock();
        mNotFull.notify_one();
        return item;
    }

    bool offer(const T& item) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.size() >= mCapacity) {
            return false;
        }
        mQueue.push(item);
        mNotEmpty.notify_one();
        return true;
    }

    bool try_take(T& item) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty()) {
            return false;
        }
        item = mQueue.front();
        mQueue.pop();
        mNotFull.notify_one();
        return true;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.size();
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.empty();
    }

    bool isFull() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.size() == mCapacity;
    }
};
}/*endof namespace*/
#endif/*__ARRAY_BLOCKING_QUEUE_H__*/
