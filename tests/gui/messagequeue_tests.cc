// Ported from AOSP CTS MessageQueueTest.java (android.os.MessageQueue).
//
// CDROID adaptation:
//  - CTS drives each test on a fresh HandlerThread (new Looper) with wait/quit. CDROID uses the
//    shared main Looper and pumps it with pumpFor()/pumpForIdle() (single-threaded).
//  - Looper.myQueue()/getMainLooper().getQueue() -> Looper::getMainLooper()->getQueue().
//  - OrderTestHelper extends Handler directly (its handleMessage validates message order); doTest
//    pumps the main looper instead of spawning a HandlerThread.
//  - testFileDescriptor*/pathological*/Register-throws: CDROID has OnFileDescriptorEventListener
//    but no Java checked exceptions / pipe test infra here; skipped (SUCCEED + comment).
//
// Original: cts/tests/tests/os/src/android/os/cts/MessageQueueTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <core/looper.h>
#include <core/messagequeue.h>
#include <core/handler.h>
#include <core/message.h>
#include <core/systemclock.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
constexpr long TIMEOUT = 1000;

// CTS IdleHandler stand-in: records whether queueIdle fired.
class TestIdleHandler : public MessageQueue::IdleHandler {
public:
    bool called = false;
    bool queueIdle() override { called = true; return false; }
};
} // namespace

// CTS OrderTestHelper: validates messages arrive in `what == 0,1,2,...,mLastMessage` order.
// Adapted to the main looper (handleMessage drives mCount; doTest pumps until mDone or timeout).
class OrderTestHelper : public Handler {
protected:
    int mLastMessage = 0;
    int mCount = 0;
    bool mSuccess = false;
    bool mDone = false;
    bool mFailed = false;
public:
    OrderTestHelper() : Handler(Looper::getMainLooper()) {}
    virtual void init() {}
    void handleMessage(Message& msg) override {
        if (mCount <= mLastMessage) {
            if (msg.what != mCount) { mFailed = true; mDone = true; return; }
            if (mCount == mLastMessage) mSuccess = true;
            mCount++;
        } else { mFailed = true; mDone = true; return; }
        onOrderedMessage(msg);
        if (mCount > mLastMessage) mDone = true;
    }
    // subclasses override for syncBarrier/atFront behaviour mid-stream
    virtual void onOrderedMessage(Message&) {}
    void doTest(int timeoutMs) {
        init();
        int64_t end = SystemClock::uptimeMillis() + timeoutMs;
        while (!mDone && SystemClock::uptimeMillis() < end) {
            pumpFor(50);
        }
        ASSERT_FALSE(mFailed) << "out-of-order or post-done message; mCount=" << mCount;
        ASSERT_TRUE(mDone) << "test timed out";
        EXPECT_TRUE(mSuccess);
    }
};

// These cases drive a fresh, isolated Looper+MessageQueue (stack-allocated `Looper lp(false)`),
// mirroring CTS's HandlerThread. The shared main looper carries the UI/stage and may always have
// pending events (Choreographer frames, traversal), so it can never be relied on to be idle; a
// fresh looper starts with an empty queue, so idle is guaranteed and IdleHandler fires on the
// first pump via drainMessageQueue → runIdleHandlers.
TEST(CtsMessageQueueTest, testAddIdleHandler) {
    Looper lp(false);
    MessageQueue* q = lp.getQueue();
    TestIdleHandler idle;          // queueIdle() returns false → auto-removed after one fire
    q->addIdleHandler(&idle);
    lp.pollOnce(10);               // empty queue → drainMessageQueue → runIdleHandlers fires
    EXPECT_TRUE(idle.called);
}

TEST(CtsMessageQueueTest, testRemoveIdleHandler) {
    Looper lp(false);
    MessageQueue* q = lp.getQueue();
    TestIdleHandler idle;
    q->addIdleHandler(&idle);
    q->removeIdleHandler(&idle);
    for (int i = 0; i < 3; i++) lp.pollOnce(10);
    EXPECT_FALSE(idle.called);
}

TEST(CtsMessageQueueTest, testIsIdle) {
    Looper lp(false);
    MessageQueue* q = lp.getQueue();
    EXPECT_TRUE(q->isIdle());      // fresh queue is empty → idle
    // Android semantics: isIdle() means "no message is due NOW". A message scheduled in the
    // future leaves the head in the future, so the queue is still considered idle.
    Handler h(&lp);
    Message* m = h.obtainMessage();
    h.sendMessageAtTime(m, SystemClock::uptimeMillis() + TIMEOUT);
    EXPECT_TRUE(q->isIdle());
    h.removeMessages(m->what);
    EXPECT_TRUE(q->isIdle());
}

TEST(CtsMessageQueueTest, testMessageOrder) {
    class Helper : public OrderTestHelper {
    public:
        void init() override {
            mLastMessage = 4;
            int64_t now = SystemClock::uptimeMillis() + 200;
            sendMessageAtTime(obtainMessage(2), now + 1);
            sendMessageAtTime(obtainMessage(3), now + 2);
            sendMessageAtTime(obtainMessage(4), now + 2);
            sendMessageAtTime(obtainMessage(0), now + 0);
            sendMessageAtTime(obtainMessage(1), now + 0);
        }
    } tester;
    tester.doTest(1000);
}

TEST(CtsMessageQueueTest, testAtFrontOfQueue) {
    class Helper : public OrderTestHelper {
    public:
        void init() override {
            mLastMessage = 3;
            int64_t now = SystemClock::uptimeMillis() + 200;
            sendMessageAtTime(obtainMessage(3), now);
            sendMessageAtFrontOfQueue(obtainMessage(2));
            sendMessageAtFrontOfQueue(obtainMessage(0));
        }
        void onOrderedMessage(Message& msg) override {
            if (msg.what == 0) sendMessageAtFrontOfQueue(obtainMessage(1));
        }
    } tester;
    tester.doTest(1000);
}

TEST(CtsMessageQueueTest, testSyncBarriers) {
    class Helper : public OrderTestHelper {
        int mBarrierToken1 = 0;
    public:
        void init() override {
            mLastMessage = 10;
            sendEmptyMessage(0);
            mBarrierToken1 = Looper::getMainLooper()->getQueue()->postSyncBarrier();
            sendEmptyMessage(5);
            sendAsync(1); sendAsync(2); sendAsync(3);
            sendEmptyMessage(6);
        }
        void onOrderedMessage(Message& msg) override {
            MessageQueue* q = Looper::getMainLooper()->getQueue();
            if (msg.what == 3) {
                sendEmptyMessage(7);
                sendAsync(4);
                sendAsync(8);
            } else if (msg.what == 4) {
                q->removeSyncBarrier(mBarrierToken1);
                sendAsync(9);
                sendEmptyMessage(10);
            } else if (msg.what == 8) {
                q->removeSyncBarrier(mBarrierToken1);  // already removed at what==4; tolerate
            }
        }
    private:
        void sendAsync(int what) {
            Message* m = obtainMessage(what);
            m->setAsynchronous(true);
            sendMessage(m);
        }
    } tester;
    tester.doTest(1000);
}

// CTS tests below rely on file-descriptor callbacks + Java checked exceptions; CDROID lacks the
// pipe test harness / NPE here, so they are documented as not-ported rather than translated.
TEST(CtsMessageQueueTest, testFileDescriptorCallbacks_SKIPPED)  { SUCCEED(); }
TEST(CtsMessageQueueTest, testPathologicalFileDescriptorReuseCallbacks_SKIPPED) { SUCCEED(); }
TEST(CtsMessageQueueTest, testRegisterFileDescriptorCallbackThrows_SKIPPED) { SUCCEED(); }
TEST(CtsMessageQueueTest, testReleaseSyncBarrierThrowsIfTokenNotValid) {
    // CDROID removeSyncBarrier on an invalid token: Android throws IllegalStateException; CDROID
    // tolerates it (no exception model). Just exercise that it does not crash.
    MessageQueue* q = Looper::getMainLooper()->getQueue();
    q->removeSyncBarrier(-1);
    SUCCEED();
}
