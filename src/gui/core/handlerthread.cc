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
#include <core/handlerthread.h>
#include <core/handler.h>

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace cdroid{

HandlerThread::HandlerThread(const std::string& name)
    : HandlerThread(name, 0 /* Process.THREAD_PRIORITY_DEFAULT */) {
}

HandlerThread::HandlerThread(const std::string& name, int priority)
    : mName(name), mPriority(priority) {
}

HandlerThread::~HandlerThread() {
    // C++ seam (AOSP: daemon thread + GC). Stop the looper and join so a
    // destroyed HandlerThread never leaves a thread behind; run() deletes
    // the Looper on the looper thread itself, detaching handlers first.
    if (mThread.joinable()) {
        quit();
        mThread.join();
    }
    // ~Handler is a no-op once the Looper detached it (onLooperDestroyed).
    delete mThreadHandler;
}

void HandlerThread::start() {
    std::lock_guard<std::mutex> lk(mLock);
    if (mStarted) return;
    mStarted = true;
    mAlive = true;
    mThread = std::thread(&HandlerThread::run, this);
}

void HandlerThread::run() {
    mTid = (int)syscall(SYS_gettid);
#if defined(__linux__)
    prctl(PR_SET_NAME, mName.c_str(), 0, 0, 0);   // Thread(name)
#endif

    Looper* looper = Looper::prepare(0);
    {
        std::lock_guard<std::mutex> lk(mLock);
        mLooper = looper;
        mCv.notify_all();
    }
    // Process.setThreadPriority(mPriority) — not ported, recorded only.
    onLooperPrepared();
    looper->loop();
    mTid = -1;
    mAlive = false;

    // The thread-local slot dies with this thread; hand the Looper its
    // explicit death (deletes it here, detaching every registered handler).
    Looper::setForThread(nullptr);

    // Wake getLooper() waiters that raced with the shutdown (AOSP waiters
    // observe !isAlive and leave with null).
    std::lock_guard<std::mutex> lk(mLock);
    mLooper = nullptr;
    mCv.notify_all();
}

Looper* HandlerThread::getLooper() {
    if (!mStarted || !mAlive) return nullptr;

    // If the thread has been started, wait until the looper has been created.
    std::unique_lock<std::mutex> lk(mLock);
    mCv.wait(lk, [this] { return mLooper != nullptr || !mAlive; });
    return mAlive ? mLooper : nullptr;
}

Handler* HandlerThread::getThreadHandler() {
    std::lock_guard<std::mutex> lk(mLock);
    if (mThreadHandler == nullptr && mLooper != nullptr) {
        mThreadHandler = new Handler(mLooper);
    }
    return mThreadHandler;
}

bool HandlerThread::quit() {
    Looper* looper = getLooper();
    if (looper == nullptr) return false;
    looper->quit();
    return true;
}

bool HandlerThread::quitSafely() {
    Looper* looper = getLooper();
    if (looper == nullptr) return false;
    looper->quitSafely();
    return true;
}

void HandlerThread::onLooperPrepared() {
}

}/*endof namespace*/
