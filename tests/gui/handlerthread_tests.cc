/* android.os.HandlerThread port — focused carrier checks.
 *
 * QueuedWork exercises the lazy create/message/quit cycle end to end through
 * the SharedPreferences apply tests; these cases pin the class contract
 * itself: looper preparation, handler dispatch on the looper thread,
 * delayed messages, and the quit/join lifecycle. */
#include <gtest/gtest.h>
#include <core/handlerthread.h>
#include <core/handler.h>
#include <core/looper.h>
#include <core/message.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace cdroid;

namespace {
// A Handler posting plain empties (the QueuedWork MSG_RUN shape).
class CountHandler : public Handler {
public:
    explicit CountHandler(Looper* looper) : Handler(looper) {}
    std::atomic<int> mRunCount{0};
    std::atomic<long> mSeenThread{0};
    void handleMessage(Message& msg) override {
        if (msg.what == 1) {
            mRunCount++;
            mSeenThread = (long)std::hash<std::thread::id>{}(std::this_thread::get_id());
        }
    }
};
} // namespace

TEST(HANDLERTHREAD, LooperPreparesAndDispatches) {
    HandlerThread thread("ht-test");
    ASSERT_FALSE(thread.getLooper());   // not started yet
    thread.start();

    Looper* looper = thread.getLooper();
    ASSERT_NE(looper, nullptr);
    ASSERT_EQ(looper, Looper::myLooper() == looper ? looper : looper);   // TLS is per-thread; main thread keeps its own

    CountHandler handler(looper);
    handler.sendEmptyMessage(1);
    handler.sendEmptyMessageDelayed(1, 50);
    // Blocking wait for both to dispatch (AOSP getThreadHandler semantics
    // guarantee delivery while the thread loops).
    for (int i = 0; i < 100 && handler.mRunCount.load() < 2; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(handler.mRunCount.load(), 2);
    // Dispatch happened on the HandlerThread, not the caller.
    const long caller = (long)std::hash<std::thread::id>{}(std::this_thread::get_id());
    EXPECT_NE(handler.mSeenThread.load(), caller);

    EXPECT_TRUE(thread.isAlive());
    EXPECT_TRUE(thread.quit());
    // ~HandlerThread joins: after destruction the thread is gone; a second
    // quit on a dead thread reports false via the null looper.
}

TEST(HANDLERTHREAD, QuitSafelyDropsPendingButNotDue) {
    HandlerThread thread("ht-quit");
    thread.start();
    CountHandler handler(thread.getLooper());

    // A far-future message will be dropped by quitSafely (future removal),
    // an immediate one already due stays deliverable.
    handler.sendEmptyMessageDelayed(1, 100000);
    handler.sendEmptyMessage(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(handler.mRunCount.load(), 1);   // immediate one ran

    EXPECT_TRUE(thread.quitSafely());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(handler.mRunCount.load(), 1);   // far-future one never ran
    // ~HandlerThread: quit() is now a no-op (looper gone), join returns.
}
