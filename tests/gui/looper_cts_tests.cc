// Ported from AOSP CTS LooperTest.java (android.os.Looper).
//
// CDROID Looper is a port of AOSP native libutils Looper (epoll/eventfd + MessageHandler) with a
// Java-flavored veneer (prepare/myLooper/getMainLooper/getQueue/loop/loopOnce). The CTS cases that
// exercise Java-only surface — quit(), isCurrentThread(), setMessageLogging(Printer), dump(),
// toString(), Looper.myQueue() static, and prepare()/prepareMainLooper() throwing on re-prepare —
// have no CDROID counterpart and are skipped (SUCCEED + comment).
//
// testLoop is adapted: CTS's blocking Looper.loop() (exited via quit() in the runnable) becomes
// a single loopOnce()/pollOnce() pump that processes the at-time-0 message, since CDROID's loop()
// has no quit() hook in the test harness.
//
// Original: cts/tests/tests/os/src/android/os/cts/LooperTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <core/looper.h>
#include <core/handler.h>
#include <core/message.h>
#include <core/messagequeue.h>
#include <thread>

using namespace cdroid;

TEST(CtsLooperTest, testGetMainLooper) {
    // GUIEnvironment/App prepares the main looper.
    EXPECT_NE(nullptr, Looper::getMainLooper());
}

TEST(CtsLooperTest, testMyLooper) {
    // CTS runs this on a fresh thread: myLooper()==null before prepare, non-null after.
    bool preNull = false, postNonNull = false;
    std::thread t([&](){
        preNull = (Looper::myLooper() == nullptr);
        Looper::prepare(false);
        postNonNull = (Looper::myLooper() != nullptr);
    });
    t.join();
    EXPECT_TRUE(preNull);
    EXPECT_TRUE(postNonNull);
}

TEST(CtsLooperTest, testGetQueue) {
    bool ok = false;
    std::thread t([&](){
        Looper::prepare(false);
        Looper* lp = Looper::myLooper();
        MessageQueue* q = lp->getQueue();
        ok = (q != nullptr);
    });
    t.join();
    EXPECT_TRUE(ok);
}

TEST(CtsLooperTest, testLoop) {
    bool runCalled = false;
    std::thread t([&](){
        Looper::prepare(false);
        Looper* lp = Looper::myLooper();
        Handler h(lp);
        Runnable run = [&runCalled](){ runCalled = true; };
        Message* msg = Message::obtain(&h, run);
        h.sendMessageAtTime(msg, 0);
        EXPECT_FALSE(runCalled);
        // CTS uses blocking loop() exited by quit() in the runnable; CDROID pumps one iteration.
        lp->pollOnce(100);
        EXPECT_TRUE(runCalled);
    });
    t.join();
    EXPECT_TRUE(runCalled);
}

// CDROID Looper has no isCurrentThread() — skipped.
TEST(CtsLooperTest, testIsCurrentThread) { SUCCEED(); }
// CDROID Looper has no myQueue() static (use myLooper()->getQueue()) — skipped.
TEST(CtsLooperTest, testMyQueue) { SUCCEED(); }
// CDROID prepare()/prepareMainLooper() do not throw on re-prepare — skipped.
TEST(CtsLooperTest, testPrepare) { SUCCEED(); }
TEST(CtsLooperTest, testPrepareMainLooper) { SUCCEED(); }
// CDROID Looper has no quit() in the Java sense — skipped (loop() exit path differs).
TEST(CtsLooperTest, testQuit) { SUCCEED(); }
// CDROID Looper has no setMessageLogging(Printer) — skipped.
TEST(CtsLooperTest, testSetMessageLogging) { SUCCEED(); }
// CDROID Looper has no toString() — skipped.
TEST(CtsLooperTest, testToString) { SUCCEED(); }
// CDROID Looper has no dump(Printer, prefix) — skipped.
TEST(CtsLooperTest, testDump) { SUCCEED(); }
