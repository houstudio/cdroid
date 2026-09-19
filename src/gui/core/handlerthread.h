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
#ifndef __CDROID_HANDLERTHREAD_H__
#define __CDROID_HANDLERTHREAD_H__
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <core/looper.h>

namespace cdroid{

class Handler;

/**
 * Port of android.os.HandlerThread — a thread that has a {@link Looper},
 * so you can create {@link Handler}s that run on it (and post work to it
 * the usual way).
 *
 * Carrier seam: java.lang.Thread becomes std::thread (name via
 * pthread_setname_np); the priority ctor keeps the AOSP face but has no
 * effect — the Process.setThreadPriority machinery is not ported.
 * C++ lifetime seam: AOSP marks the thread daemon and lets GC reclaim the
 * object; here the destructor quits the looper and joins, so a destroyed
 * HandlerThread never leaves a thread behind (leak the instance instead
 * for process-lifetime daemon behavior).
 */
class HandlerThread{
private:
    std::string mName;
    int mPriority;                       // recorded only (no Process port)
    std::thread mThread;
    std::mutex mLock;                    // the 'this' monitor
    std::condition_variable mCv;
    Looper* mLooper = nullptr;           // guarded by mLock until published
    Handler* mThreadHandler = nullptr;   // guarded by mLock (getThreadHandler)
    std::atomic<bool> mStarted{false};   // Thread.start() called
    std::atomic<bool> mAlive{false};     // run() not finished yet
    int mTid = -1;                       // Process.myTid(), Linux tid
public:
    HandlerThread(const std::string& name);
    HandlerThread(const std::string& name, int priority);
    virtual ~HandlerThread();

    // Thread.start(): spawns the looper thread. Call before getLooper().
    void start();

    /**
     * This method returns the Looper associated with this thread. If this
     * thread not been started or has died, this method will return null.
     * If this thread has been started, this method will block until the
     * looper has been initialized.
     */
    Looper* getLooper();

    /**
     * @return a shared {@link Handler} that posts to this thread's looper
     * (the @hide getThreadHandler; owned by this HandlerThread).
     */
    Handler* getThreadHandler();

    /**
     * Ask the currently running looper to quit.  If the thread has not
     * been started or has finished (isAlive() returns false), or the
     * looper has already asked to quit, false is returned.
     */
    bool quit();

    /**
     * Ask the currently running looper to quit safely.  Rather than
     * dropping pending messages in the message queue, this method ensures
     * all pending messages dispatched before the looper terminates, but
     * tries its best to deliver them on time (drains due messages only).
     */
    bool quitSafely();

    bool isAlive() const { return mAlive; }

protected:
    /**
     * Call back method that can be explicitly overridden if needed to
     * execute some setup before Looper loops.
     */
    virtual void onLooperPrepared();

private:
    void run();   // Thread.run() — the java.lang.Thread body
};

}/*endof namespace*/
#endif/*__CDROID_HANDLERTHREAD_H__*/
