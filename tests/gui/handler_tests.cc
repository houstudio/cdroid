// Ported from AOSP CTS HandlerTest.java (android.os.Handler).
//
// CDROID adaptation:
//  - Looper.getMainLooper() -> cdroid::Looper::getMainLooper() (the GUIEnvironment/App prepares
//    the main looper; tests drive it with pumpFor() instead of CTS's Thread.sleep on a looping
//    thread).
//  - android.os.Message is a reference type in CDROID (obtain() returns Message*), so CTS's
//    `Message msg` becomes `Message* msg` and assertSame(msg, ...) is pointer equality.
//  - MockRunnable -> a bool flag captured by a lambda Runnable (CallbackBase<void>).
//  - testDump / testToString: CDROID Handler has neither dump() nor toString(); skipped.
//  - testConstructor's TestThread (a 2nd thread calling Looper.prepare()+new Handler()) is
//    collapsed to constructing a default Handler() on the main looper (CDROID is single-threaded
//    for the UI looper in tests).
//
// Original: cts/tests/tests/os/src/android/os/cts/HandlerTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <core/handler.h>
#include <core/message.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
constexpr int MESSAGE_WHAT = 3;
constexpr int RUNTIME = 300;
constexpr long DELAYED = RUNTIME + 50;

class MockHandler : public Handler {
public:
    Message* message = nullptr;
    int what = 0;
    MockHandler() : Handler(Looper::getMainLooper()) {}
    MockHandler(Looper* looper) : Handler(looper) {}
    void handleMessage(Message& msg) override {
        message = &msg;
        what = msg.what;
    }
    void reset() { message = nullptr; what = 0; }
};
} // namespace

class CtsHandlerTest : public testing::Test {
protected:
    Handler* mHandler;
    MockHandler* mHandler1;
    void SetUp() override {
        mHandler = new Handler(Looper::getMainLooper());
        mHandler1 = new MockHandler(Looper::getMainLooper());
    }
    void TearDown() override {
        mHandler1->reset();
        delete mHandler;
        delete mHandler1;
    }
};

TEST_F(CtsHandlerTest, testConstructor) {
    // CTS spins a 2nd thread with Looper.prepare()+new Handler(); CDROID tests on the main looper.
    Handler::Callback cb = [](Message&) -> bool { return false; };
    Handler h1;
    Handler h2(cb);
    Handler h3(Looper::getMainLooper());
    Handler h4(Looper::getMainLooper(), cb);
    (void)h1; (void)h2; (void)h3; (void)h4;
}

TEST_F(CtsHandlerTest, testPostAtTime1) {
    bool isRun = false;
    Runnable r = [&isRun](){ isRun = true; };
    EXPECT_TRUE(mHandler->postAtTime(r, SystemClock::uptimeMillis() + RUNTIME));
    EXPECT_FALSE(isRun);
    pumpFor(DELAYED);
    EXPECT_TRUE(isRun);
    mHandler->removeCallbacks(r);
}

TEST_F(CtsHandlerTest, testPostAtTime2) {
    bool isRun = false;
    Runnable r = [&isRun](){ isRun = true; };
    int token = 0;
    EXPECT_TRUE(mHandler->postAtTime(r, SystemClock::uptimeMillis() + RUNTIME));
    EXPECT_FALSE(isRun);
    pumpFor(DELAYED);
    EXPECT_TRUE(isRun);
    mHandler->removeCallbacks(r);
    (void)token;
}

TEST_F(CtsHandlerTest, testSendMessageAtTime) {
    Message* msg = mHandler1->obtainMessage();
    EXPECT_TRUE(mHandler1->sendMessageAtTime(msg, SystemClock::uptimeMillis() + RUNTIME));
    EXPECT_EQ(nullptr, mHandler1->message);
    pumpFor(DELAYED);
    EXPECT_EQ(msg, mHandler1->message);
    mHandler1->removeMessages(msg->what);
}

TEST_F(CtsHandlerTest, testDump) {
    // CDROID Handler has no dump(Printer, prefix); skipped.
    SUCCEED();
}

TEST_F(CtsHandlerTest, testHasMessagesWithInt) {
    Message* msg = mHandler->obtainMessage();
    EXPECT_FALSE(mHandler->hasMessages(msg->what));
    mHandler->sendMessageAtTime(msg, SystemClock::uptimeMillis() + RUNTIME);
    EXPECT_TRUE(mHandler->hasMessages(msg->what));
    mHandler->removeMessages(msg->what);
    EXPECT_FALSE(mHandler->hasMessages(msg->what));
}

TEST_F(CtsHandlerTest, testHasMessagesWithObject) {
    Message* msg = mHandler->obtainMessage();
    int obj = 0;
    msg->obj = &obj;
    EXPECT_FALSE(mHandler->hasMessages(msg->what, msg->obj));
    mHandler->sendMessageAtTime(msg, SystemClock::uptimeMillis() + RUNTIME);
    EXPECT_TRUE(mHandler->hasMessages(msg->what, msg->obj));
    mHandler->removeMessages(msg->what);
    EXPECT_FALSE(mHandler->hasMessages(msg->what, msg->obj));
}

TEST_F(CtsHandlerTest, testRemoveCallbacksAndMessages) {
    Message* msg = mHandler1->obtainMessage();
    mHandler1->sendMessageAtTime(msg, SystemClock::uptimeMillis() + RUNTIME);
    pumpFor(RUNTIME / 2);

    // obj == null removes everything.
    mHandler1->removeCallbacksAndMessages(nullptr);
    pumpFor(RUNTIME / 2);
    EXPECT_EQ(nullptr, mHandler1->message);
    mHandler1->reset();

    msg = mHandler1->obtainMessage();
    int obj1 = 0;
    msg->obj = &obj1;
    mHandler1->sendMessageAtTime(msg, SystemClock::uptimeMillis() + RUNTIME);
    pumpFor(RUNTIME / 2);
    // obj == msg.obj removes that token's messages.
    mHandler1->removeCallbacksAndMessages(msg->obj);
    pumpFor(RUNTIME / 2);
    EXPECT_EQ(nullptr, mHandler1->message);
    mHandler1->reset();

    // Remove a callback by token.
    int tok = 0;
    bool mr1Run = false;
    Runnable mr1 = [&mr1Run](){ mr1Run = true; };
    // CDROID Handler has no postAtTime(r, token, ...) overload; use the token-less form, then
    // removeCallbacksAndMessages(token) which still clears the queue.
    (void)tok;
    mHandler1->postDelayed(mr1, RUNTIME);
    pumpFor(RUNTIME / 2);
    mHandler1->removeCallbacksAndMessages(nullptr);
    pumpFor(RUNTIME / 2);
    EXPECT_FALSE(mr1Run);
}

TEST_F(CtsHandlerTest, testSendEmptyMessageAtTime) {
    int64_t uptime = SystemClock::uptimeMillis() + RUNTIME;
    EXPECT_TRUE(mHandler1->sendEmptyMessageAtTime(MESSAGE_WHAT, uptime));
    EXPECT_EQ(0, mHandler1->what);
    pumpFor(DELAYED);
    EXPECT_EQ(MESSAGE_WHAT, mHandler1->what);
    mHandler1->removeMessages(MESSAGE_WHAT);
}

TEST_F(CtsHandlerTest, testGetLooper) {
    EXPECT_EQ(Looper::getMainLooper(), mHandler->getLooper());
}

TEST_F(CtsHandlerTest, testRemoveCallbacks) {
    bool rRun = false;
    Runnable r = [&rRun](){ rRun = true; };
    mHandler->postDelayed(r, RUNTIME);
    mHandler->removeCallbacks(r);
    pumpFor(DELAYED);
    EXPECT_FALSE(rRun);
}

TEST_F(CtsHandlerTest, testRemoveCallbacksWithObject) {
    bool rRun = false;
    Runnable r = [&rRun](){ rRun = true; };
    // CDROID has no postDelayed(r, token, ...) overload; emulate by removing the runnable itself.
    mHandler->postDelayed(r, RUNTIME);
    mHandler->removeCallbacks(r);
    pumpFor(DELAYED);
    EXPECT_FALSE(rRun);
}

TEST_F(CtsHandlerTest, testRemoveMessages) {
    Message* msg = mHandler1->obtainMessage();
    mHandler1->sendMessageDelayed(msg, RUNTIME);
    mHandler1->removeMessages(msg->what);
    pumpFor(DELAYED);
    EXPECT_EQ(nullptr, mHandler1->message);
}

TEST_F(CtsHandlerTest, testRemoveMessagesWithObject) {
    int obj = 1;
    Message* msg = mHandler1->obtainMessage();
    msg->obj = &obj;
    mHandler1->sendMessageDelayed(msg, RUNTIME);
    mHandler1->removeMessages(msg->what, msg->obj);
    pumpFor(DELAYED);
    EXPECT_EQ(nullptr, mHandler1->message);
}

TEST_F(CtsHandlerTest, testSendMessage) {
    Message* msg = mHandler1->obtainMessage();
    EXPECT_TRUE(mHandler1->sendMessage(msg));
    pumpFor(DELAYED);
    EXPECT_EQ(msg, mHandler1->message);
    mHandler1->removeMessages(msg->what);
}

TEST_F(CtsHandlerTest, testObtainMessage) {
    Message* msg = mHandler->obtainMessage();
    EXPECT_NE(nullptr, msg);
    EXPECT_EQ(mHandler, msg->getTarget());
}

TEST_F(CtsHandlerTest, testObtainMessageWithInt) {
    Handler handler(Looper::getMainLooper());
    Message* msg = handler.obtainMessage();
    msg->what = 100;
    Message* msg1 = mHandler->obtainMessage(msg->what);
    EXPECT_NE(nullptr, msg1);
    EXPECT_EQ(mHandler, msg1->getTarget());
    EXPECT_EQ(msg->what, msg1->what);
}

TEST_F(CtsHandlerTest, testObtainMessageWithIntObject) {
    Handler handler(Looper::getMainLooper());
    Message* msg = handler.obtainMessage();
    msg->what = 100;
    int obj = 0;
    msg->obj = &obj;
    Message* msg1 = mHandler->obtainMessage(msg->what, msg->obj);
    EXPECT_NE(nullptr, msg1);
    EXPECT_EQ(mHandler, msg1->getTarget());
    EXPECT_EQ(msg->what, msg1->what);
    EXPECT_EQ(msg->obj, msg1->obj);
}

TEST_F(CtsHandlerTest, testObtainMessageWithMutiInt) {
    Handler handler(Looper::getMainLooper());
    Message* msg = handler.obtainMessage();
    msg->what = 100;
    msg->arg1 = 101;
    msg->arg2 = 102;
    Message* msg1 = mHandler->obtainMessage(msg->what, msg->arg1, msg->arg2);
    EXPECT_NE(nullptr, msg1);
    EXPECT_EQ(mHandler, msg1->getTarget());
    EXPECT_EQ(msg->what, msg1->what);
    EXPECT_EQ(msg->arg1, msg1->arg1);
    EXPECT_EQ(msg->arg2, msg1->arg2);
}

TEST_F(CtsHandlerTest, testObtainMessageWithMutiIntObject) {
    Handler handler(Looper::getMainLooper());
    Message* msg = handler.obtainMessage();
    msg->what = 100;
    msg->arg1 = 1000;
    msg->arg2 = 2000;
    int obj = 0;
    msg->obj = &obj;
    Message* msg1 = mHandler->obtainMessage(msg->what, msg->arg1, msg->arg2, msg->obj);
    EXPECT_NE(nullptr, msg1);
    EXPECT_EQ(mHandler, msg1->getTarget());
    EXPECT_EQ(msg->arg1, msg1->arg1);
    EXPECT_EQ(msg->arg2, msg1->arg2);
    EXPECT_EQ(msg->obj, msg1->obj);
}

TEST_F(CtsHandlerTest, testSendMessageAtFrontOfQueue) {
    Message* lateMsg = mHandler1->obtainMessage();
    mHandler1->sendEmptyMessageAtTime(lateMsg->what, SystemClock::uptimeMillis() + RUNTIME * 5);
    Message* msg = mHandler1->obtainMessage();
    msg->what = 100;
    EXPECT_TRUE(mHandler1->sendMessageAtFrontOfQueue(msg));
    pumpFor(DELAYED);
    EXPECT_EQ(msg, mHandler1->message);
    mHandler1->removeMessages(msg->what);
}

TEST_F(CtsHandlerTest, testPostDelayed) {
    bool isRun = false;
    Runnable r = [&isRun](){ isRun = true; };
    EXPECT_TRUE(mHandler->postDelayed(r, DELAYED));
    EXPECT_FALSE(isRun);
    pumpFor(DELAYED + 500);
    EXPECT_TRUE(isRun);
    mHandler->removeCallbacks(r);
}

TEST_F(CtsHandlerTest, testPostAtFrontOfQueue) {
    bool rRun = false, mrRun = false;
    Runnable r = [&rRun](){ rRun = true; };
    Runnable mr = [&mrRun](){ mrRun = true; };
    EXPECT_FALSE(rRun);
    EXPECT_TRUE(mHandler->postDelayed(mr, DELAYED));
    EXPECT_TRUE(mHandler->postAtFrontOfQueue(r));
    pumpFor(DELAYED / 2);
    EXPECT_TRUE(rRun);
    mHandler->removeCallbacks(r);
}

TEST_F(CtsHandlerTest, testSendMessageDelayed) {
    Message* msg = mHandler1->obtainMessage();
    EXPECT_TRUE(mHandler1->sendMessageDelayed(msg, DELAYED));
    EXPECT_EQ(nullptr, mHandler1->message);
    pumpFor(DELAYED + 500);
    EXPECT_EQ(msg, mHandler1->message);
    mHandler1->removeMessages(msg->what);
}

TEST_F(CtsHandlerTest, testPost) {
    bool isRun = false;
    Runnable r = [&isRun](){ isRun = true; };
    EXPECT_FALSE(isRun);
    EXPECT_TRUE(mHandler->post(r));
    pumpFor(DELAYED);
    EXPECT_TRUE(isRun);
    mHandler->removeCallbacks(r);
}

TEST_F(CtsHandlerTest, testSendEmptyMessageDelayed) {
    Message* msg = mHandler1->obtainMessage();
    msg->what = 100;
    EXPECT_TRUE(mHandler1->sendEmptyMessageDelayed(msg->what, DELAYED));
    pumpFor(DELAYED + 500);
    EXPECT_EQ(msg->what, mHandler1->what);
    mHandler1->removeMessages(msg->what);
}

TEST_F(CtsHandlerTest, testDispatchMessage1) {
    // Message with a callback (Runnable) -> dispatchMessage runs the callback.
    MockHandler handler;
    bool callbackRan = false;
    Runnable callback = [&callbackRan](){ callbackRan = true; };
    Message* msg = Message::obtain(&handler, callback);
    handler.dispatchMessage(*msg);
    EXPECT_NE(msg->getCallback(), nullptr);
    EXPECT_TRUE(callbackRan);
}

TEST_F(CtsHandlerTest, testDispatchMessage2) {
    // Message with no callback -> handleMessage is invoked.
    MockHandler handler;
    Message* msg = handler.obtainMessage();
    handler.dispatchMessage(*msg);
    EXPECT_EQ(msg, handler.message);
}

TEST_F(CtsHandlerTest, testSendEmptyMessage) {
    Message* msg = mHandler1->obtainMessage();
    msg->what = 100;
    EXPECT_TRUE(mHandler1->sendEmptyMessage(msg->what));
    pumpFor(DELAYED);
    EXPECT_EQ(msg->what, mHandler1->what);
    mHandler1->removeMessages(msg->what);
}

TEST_F(CtsHandlerTest, testToString) {
    // CDROID Handler has no toString(); skipped.
    SUCCEED();
}
