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
#include <core/queuedwork.h>
#include <core/handler.h>
#include <core/handlerthread.h>
#include <core/message.h>

#include <list>
#include <mutex>

namespace cdroid{

namespace { // file-local, the AOSP statics

/** Delay for delayed runnables, as big as possible but low enough to be
 * barely perceivable (QueuedWork.DELAY, ms). */
static constexpr long DELAY = 100;

struct State {
    // sLock: guards everything below plus the lazily created handler pair.
    std::mutex sLock;
    // Used to make sure that only one thread is processing work items at a
    // time. This means that they are processed in the order added. Separate
    // from sLock as it is held the whole time while work is processed.
    std::mutex sProcessingWork;

    std::list<Runnable> sFinishers;   // addFinisher/removed, run by waitToFinish
    std::list<Runnable> sWork;        // work queued via queue()
    bool sCanDelay = true;            // if new work can be delayed or not
};

static State& state() {
    // Leaked on purpose (the JVM-daemon role): the queued-work thread
    // outlives every caller. A normally-destroyed static would run its
    // dtor under the parked thread at process exit.
    static State* s = new State();
    return *s;
}

// The lazily created sHandler/sHandlerThread pair (AOSP statics). The
// objects are owned by these slots; quitSafely() releases them (the
// resetHandler role). Never destroyed at process exit — daemon pattern.
static Handler*& handlerSlot() { static Handler* s = nullptr; return s; }
static HandlerThread*& threadSlot() { static HandlerThread* s = nullptr; return s; }

static void processPendingWorkImpl();   // defined below the handler

class QueuedWorkHandler : public Handler {
public:
    static constexpr int MSG_RUN = 1;

    explicit QueuedWorkHandler(Looper* looper) : Handler(looper) {}

    void handleMessage(Message& msg) override {
        if (msg.what == MSG_RUN) {
            processPendingWorkImpl();
        }
    }
};

// processPendingWork's body (the class method is public API; this twin
// serves the anonymous-namespace handler above).
static void processPendingWorkImpl() {
    State& s = state();
    std::lock_guard<std::mutex> processing(s.sProcessingWork);

    std::list<Runnable> work;
    {
        std::lock_guard<std::mutex> lk(s.sLock);
        work = std::move(s.sWork);
        s.sWork.clear();
        // Remove all msg-s as all work will be processed now.
        if (handlerSlot()) handlerSlot()->removeMessages(QueuedWorkHandler::MSG_RUN);
    }

    for (Runnable& w : work) {
        if (w) w();
    }
}

// Lazily create the handler on a separate thread (the getHandler() role).
static Handler* getHandler() {
    State& s = state();
    std::lock_guard<std::mutex> lk(s.sLock);
    if (handlerSlot() == nullptr) {
        HandlerThread* handlerThread = new HandlerThread("queued-work-looper");
        handlerThread->start();
        threadSlot() = handlerThread;
        handlerSlot() = new QueuedWorkHandler(handlerThread->getLooper());
    }
    return handlerSlot();
}

} // anonymous namespace

void QueuedWork::addFinisher(const Runnable& finisher) {
    State& s = state();
    std::lock_guard<std::mutex> lk(s.sLock);
    s.sFinishers.push_back(finisher);
}

void QueuedWork::removeFinisher(const Runnable& finisher) {
    State& s = state();
    std::lock_guard<std::mutex> lk(s.sLock);
    // CallbackBase identity: copies of one Runnable share their functor, so
    // == finds the registration to erase (AOSP LinkedList#remove).
    s.sFinishers.remove_if([&](const Runnable& r) { return r == finisher; });
}

void QueuedWork::waitToFinish() {
    State& s = state();

    {
        std::lock_guard<std::mutex> lk(s.sLock);
        // Cancel any pending run message (handlerRemoveMessages(MSG_RUN)).
        if (handlerSlot() != nullptr) handlerSlot()->removeMessages(QueuedWorkHandler::MSG_RUN);
        // We should not delay any work as this might delay the finishers.
        s.sCanDelay = false;
    }

    processPendingWorkImpl();

    while (true) {
        Runnable finisher;
        {
            std::lock_guard<std::mutex> lk(s.sLock);
            if (s.sFinishers.empty()) break;
            finisher = s.sFinishers.front();
            s.sFinishers.pop_front();
        }
        if (finisher) finisher();
    }
    s.sCanDelay = true;
}

void QueuedWork::queue(const Runnable& work, bool shouldDelay) {
    Handler* handler = getHandler();

    State& s = state();
    std::lock_guard<std::mutex> lk(s.sLock);
    s.sWork.push_back(work);

    if (shouldDelay && s.sCanDelay) {
        handler->sendEmptyMessageDelayed(QueuedWorkHandler::MSG_RUN, DELAY);
    } else {
        handler->sendEmptyMessage(QueuedWorkHandler::MSG_RUN);
    }
}

bool QueuedWork::hasPendingWork() {
    State& s = state();
    std::lock_guard<std::mutex> lk(s.sLock);
    return !s.sWork.empty();
}

void QueuedWork::quitSafely() {
    // The resetHandler role (AOSP test-only there, CDROID exit hook here):
    // drain, then tear the handler pair down. Pointers are taken out of the
    // slots before joining — the dying thread's work may still take sLock,
    // and join() must not happen under it.
    processPendingWorkImpl();
    Handler* handler = nullptr;
    HandlerThread* thread = nullptr;
    {
        State& s = state();
        std::lock_guard<std::mutex> lk(s.sLock);
        handler = handlerSlot();
        thread = threadSlot();
        handlerSlot() = nullptr;
        threadSlot() = nullptr;
    }
    if (thread == nullptr) return;
    thread->quitSafely();   // wake the looper, drain due messages, quit
    delete thread;          // joins; run() deletes the Looper (handlers detach)
    // The handler was detached by ~Looper's onLooperDestroyed sweep — a
    // no-op delete.
    delete handler;
}

void QueuedWork::processPendingWork() {
    processPendingWorkImpl();
}

}/*endof namespace*/
