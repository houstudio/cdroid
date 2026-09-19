// Ported from AOSP system/core/libutils/tests/Looper_test.cpp — the official
// native Looper test suite. CDROID's Looper is a line-by-line port of
// libutils/Looper.cpp, so this file keeps the original 28 cases and semantics.
// Adaptations (everything else is verbatim):
//   - every case runs on a PRIVATE Looper (new Looper(true)), never the shared
//     main looper, so poll timing cannot be perturbed by Choreographer frames;
//   - TestHelpers.h's Pipe / DelayedTask and StopWatch are re-created locally
//     (std::thread + usleep; SystemClock::uptimeMillis deltas);
//   - Message has no Message(what) ctor here, so a makeMessage() helper sets
//     .what; StubMessageHandler records .what values instead of Message copies;
//   - StubMessageHandler::handleMessage takes Message& (not const Message&);
//   - sendMessageAtTime's clock is SystemClock::uptimeMillis (AOSP uses
//     systemTime(SYSTEM_TIME_MONOTONIC)) — the base the ported Looper uses.
// Original: Copyright 2010 The Android Open Source Project (Apache 2.0)
#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/systemclock.h>
#include <unistd.h>
#include <time.h>
#include <thread>
#include <vector>

#if defined(__linux__)||defined(__unix__)

// # of milliseconds to fudge stopwatch measurements
#define TIMING_TOLERANCE_MS 25

namespace cdroid{

enum {
    MSG_TEST1 = 1,
    MSG_TEST2 = 2,
    MSG_TEST3 = 3,
    MSG_TEST4 = 4,
};

// --- TestHelpers.h re-creation ---

class StopWatch {
    int64_t mStart;
public:
    explicit StopWatch(const char*) : mStart(SystemClock::uptimeMillis()) { }
    void reset() { mStart = SystemClock::uptimeMillis(); }
    int32_t elapsedTimeMillis() const { return int32_t(SystemClock::uptimeMillis() - mStart); }
};

class Pipe {
public:
    int receiveFd;
    int sendFd;

    // ASSERT_* expands to `return;`, illegal in a ctor — a failed pipe leaves
    // both fds at -1 and the very first addFd/pollOnce assertion will flag it.
    Pipe() : receiveFd(-1), sendFd(-1) {
        int fds[2];
        if (::pipe(fds) == 0) {
            receiveFd = fds[0];
            sendFd = fds[1];
        }
    }

    ~Pipe() {
        if (receiveFd != -1) ::close(receiveFd);
        if (sendFd != -1) ::close(sendFd);
    }

    // Writes a byte to the send end; returns 0 on success (AOSP status_t OK).
    int writeSignal() {
        char b = 'W';
        ssize_t n = ::write(sendFd, &b, sizeof(b));
        return n == sizeof(b) ? 0 : -1;
    }

    // Reads the byte back; returns 0 on success (AOSP status_t OK).
    int readSignal() {
        char b;
        ssize_t n = ::read(receiveFd, &b, sizeof(b));
        return n == sizeof(b) ? 0 : -1;
    }
};

class DelayedTask {
    int mDelayMillis;
    std::thread mThread;
public:
    DelayedTask(int delayMillis) : mDelayMillis(delayMillis) { }
    virtual ~DelayedTask() {
        if (mThread.joinable()) mThread.join();
    }
    void run() {
        mThread = std::thread([this] {
            usleep(mDelayMillis * 1000);
            doTask();
        });
    }
protected:
    virtual void doTask() = 0;
};

class DelayedWake : public DelayedTask {
    Looper* mLooper;
public:
    DelayedWake(int delayMillis, Looper* looper) :
        DelayedTask(delayMillis), mLooper(looper) {
    }
protected:
    void doTask() override {
        mLooper->wake();
    }
};

class DelayedWriteSignal : public DelayedTask {
    Pipe* mPipe;
public:
    DelayedWriteSignal(int delayMillis, Pipe* pipe) :
        DelayedTask(delayMillis), mPipe(pipe) {
    }
protected:
    void doTask() override {
        mPipe->writeSignal();
    }
};

class CallbackHandler {
public:
    void setCallback(Looper* looper, int fd, int events) {
        looper->addFd(fd, 0, events, staticHandler, this);
    }
protected:
    virtual ~CallbackHandler() { }
    virtual int handler(int fd, int events) = 0;
private:
    static int staticHandler(int fd, int events, void* data) {
        return static_cast<CallbackHandler*>(data)->handler(fd, events);
    }
};

class StubCallbackHandler : public CallbackHandler {
public:
    int nextResult;
    int callbackCount;

    int fd;
    int events;

    StubCallbackHandler(int nextResult) : nextResult(nextResult),
        callbackCount(0), fd(-1), events(-1) {
    }
protected:
    int handler(int fd, int events) override {
        callbackCount += 1;
        this->fd = fd;
        this->events = events;
        return nextResult;
    }
};

class StubMessageHandler : public MessageHandler {
public:
    std::vector<int> messages; // records .what (AOSP stores Message copies)

    void handleMessage(Message& message) override {
        messages.push_back(message.what);
    }
};

static Message makeMessage(int what) {
    Message msg;
    msg.what = what;
    return msg;
}

class LooperTest : public testing::Test {
protected:
    Looper* mLooper;

    void SetUp() override {
        mLooper = new Looper(true /*allowNonCallbacks*/);
    }

    void TearDown() override {
        delete mLooper;
    }
};

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndNotAwoken_WaitsForTimeout) {
    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndAwokenBeforeWaiting_ImmediatelyReturns) {
    mLooper->wake();

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because wake() was called before waiting";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be POLL_WAKE because loop was awoken";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndAwokenWhileWaiting_PromptlyReturns) {
    DelayedWake delayedWake(100, mLooper);
    delayedWake.run();

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal wake delay";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be POLL_WAKE because loop was awoken";
}

TEST_F(LooperTest, PollOnce_WhenZeroTimeoutAndNoRegisteredFDs_ImmediatelyReturns) {
    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT";
}

TEST_F(LooperTest, PollOnce_WhenZeroTimeoutAndNoSignalledFDs_ImmediatelyReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT";
    EXPECT_EQ(0, handler.callbackCount)
            << "callback should not have been invoked because FD was not signalled";
}

TEST_F(LooperTest, PollOnce_WhenZeroTimeoutAndSignalledFD_ImmediatelyInvokesCallbackAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    ASSERT_EQ(0, pipe.writeSignal());
    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked exactly once";
    EXPECT_EQ(pipe.receiveFd, handler.fd)
            << "callback should have received pipe fd as parameter";
    EXPECT_EQ(Looper::EVENT_INPUT, handler.events)
            << "callback should have received EVENT_INPUT as events";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndNoSignalledFDs_WaitsForTimeoutAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT";
    EXPECT_EQ(0, handler.callbackCount)
            << "callback should not have been invoked because FD was not signalled";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndSignalledFDBeforeWaiting_ImmediatelyInvokesCallbackAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    pipe.writeSignal();
    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked exactly once";
    EXPECT_EQ(pipe.receiveFd, handler.fd)
            << "callback should have received pipe fd as parameter";
    EXPECT_EQ(Looper::EVENT_INPUT, handler.events)
            << "callback should have received EVENT_INPUT as events";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndSignalledFDWhileWaiting_PromptlyInvokesCallbackAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);
    DelayedWriteSignal delayedWriteSignal(100, &pipe);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);
    delayedWriteSignal.run();

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal signal delay";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked exactly once";
    EXPECT_EQ(pipe.receiveFd, handler.fd)
            << "callback should have received pipe fd as parameter";
    EXPECT_EQ(Looper::EVENT_INPUT, handler.events)
            << "callback should have received EVENT_INPUT as events";
}

TEST_F(LooperTest, PollOnce_WhenCallbackAddedThenRemoved_CallbackShouldNotBeInvoked) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);
    pipe.writeSignal(); // would cause FD to be considered signalled
    mLooper->removeFd(pipe.receiveFd);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal timeout because FD was no longer registered";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT";
    EXPECT_EQ(0, handler.callbackCount)
            << "callback should not be invoked";
}

TEST_F(LooperTest, PollOnce_WhenCallbackReturnsFalse_CallbackShouldNotBeInvokedAgainLater) {
    Pipe pipe;
    StubCallbackHandler handler(false);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    // First loop: Callback is registered and FD is signalled.
    pipe.writeSignal();

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal zero because FD was already signalled";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked";

    // Second loop: Callback is no longer registered and FD is signalled.
    pipe.writeSignal();

    stopWatch.reset();
    result = mLooper->pollOnce(0);
    elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal zero because timeout was zero";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should not be invoked this time";
}

TEST_F(LooperTest, PollOnce_WhenNonCallbackFdIsSignalled_ReturnsIdent) {
    const int expectedIdent = 5;
    void* expectedData = this;

    Pipe pipe;

    pipe.writeSignal();
    // NULL callback is ambiguous between the two addFd overloads — pin the
    // plain-function-pointer one (AOSP passes NULL against a single signature).
    mLooper->addFd(pipe.receiveFd, expectedIdent, Looper::EVENT_INPUT,
            (Looper_callbackFunc) nullptr, expectedData);

    StopWatch stopWatch("pollOnce");
    int fd;
    int events;
    void* data;
    int result = mLooper->pollOnce(100, &fd, &events, &data);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(expectedIdent, result)
            << "pollOnce result should be the ident of the FD that was signalled";
    EXPECT_EQ(pipe.receiveFd, fd)
            << "pollOnce should have returned the received pipe fd";
    EXPECT_EQ(Looper::EVENT_INPUT, events)
            << "pollOnce should have returned EVENT_INPUT as events";
    EXPECT_EQ(expectedData, data)
            << "pollOnce should have returned the data";
}

TEST_F(LooperTest, PollOnce_WhenCallbackAddedTwice_OnlySecondCallbackShouldBeInvoked) {
    Pipe pipe;
    StubCallbackHandler handler1(true);
    StubCallbackHandler handler2(true);

    handler1.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);
    handler2.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT); // replace it
    pipe.writeSignal(); // would cause FD to be considered signalled

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    ASSERT_EQ(0, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because FD was already signalled";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(0, handler1.callbackCount)
            << "original handler callback should not be invoked because it was replaced";
    EXPECT_EQ(1, handler2.callbackCount)
            << "replacement handler callback should be invoked";
}

TEST_F(LooperTest, AddFd_WhenCallbackAdded_ReturnsOne) {
    Pipe pipe;
    int result = mLooper->addFd(pipe.receiveFd, 0, Looper::EVENT_INPUT,
            (Looper_callbackFunc) nullptr, NULL);

    EXPECT_EQ(1, result)
            << "addFd should return 1 because FD was added";
}

TEST_F(LooperTest, AddFd_WhenIdentIsNegativeAndCallbackIsNull_ReturnsError) {
    Pipe pipe;
    int result = mLooper->addFd(pipe.receiveFd, -1, Looper::EVENT_INPUT,
            (Looper_callbackFunc) nullptr, NULL);

    EXPECT_EQ(-1, result)
            << "addFd should return -1 because arguments were invalid";
}

TEST_F(LooperTest, AddFd_WhenNoCallbackAndAllowNonCallbacksIsFalse_ReturnsError) {
    Pipe pipe;
    Looper* looper = new Looper(false /*allowNonCallbacks*/);
    int result = looper->addFd(pipe.receiveFd, 0, 0, (Looper_callbackFunc) nullptr, NULL);
    delete looper;

    EXPECT_EQ(-1, result)
            << "addFd should return -1 because arguments were invalid";
}

TEST_F(LooperTest, RemoveFd_WhenCallbackNotAdded_ReturnsZero) {
    int result = mLooper->removeFd(1);

    EXPECT_EQ(0, result)
            << "removeFd should return 0 because FD not registered";
}

TEST_F(LooperTest, RemoveFd_WhenCallbackAddedThenRemovedTwice_ReturnsOnceFirstTimeAndReturnsZeroSecondTime) {
    Pipe pipe;
    StubCallbackHandler handler(false);
    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    // First time.
    int result = mLooper->removeFd(pipe.receiveFd);

    EXPECT_EQ(1, result)
            << "removeFd should return 1 first time because FD was registered";

    // Second time.
    result = mLooper->removeFd(pipe.receiveFd);

    EXPECT_EQ(0, result)
            << "removeFd should return 0 second time because FD was no longer registered";
}

TEST_F(LooperTest, SendMessage_WhenOneMessageIsEnqueue_ShouldInvokeHandlerDuringNextPoll) {
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessage(handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    delete handler;
}

TEST_F(LooperTest, SendMessage_WhenMultipleMessagesAreEnqueued_ShouldInvokeHandlersInOrderDuringNextPoll) {
    StubMessageHandler* handler1 = new StubMessageHandler();
    StubMessageHandler* handler2 = new StubMessageHandler();
    mLooper->sendMessage(handler1, makeMessage(MSG_TEST1));
    mLooper->sendMessage(handler2, makeMessage(MSG_TEST2));
    mLooper->sendMessage(handler1, makeMessage(MSG_TEST3));
    mLooper->sendMessage(handler1, makeMessage(MSG_TEST4));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(3), handler1->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler1->messages[0])
            << "handled message";
    EXPECT_EQ(MSG_TEST3, handler1->messages[1])
            << "handled message";
    EXPECT_EQ(MSG_TEST4, handler1->messages[2])
            << "handled message";
    EXPECT_EQ(size_t(1), handler2->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST2, handler2->messages[0])
            << "handled message";
    delete handler1;
    delete handler2;
}

TEST_F(LooperTest, SendMessageDelayed_WhenSentToTheFuture_ShouldInvokeHandlerAfterDelayTime) {
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessageDelayed(100 /*ms*/, handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "first poll should end quickly because next message timeout was computed";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be POLL_WAKE due to wakeup";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no message handled yet";

    result = mLooper->pollOnce(1000);
    elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "second poll should end around the time of the delayed message dispatch";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";

    result = mLooper->pollOnce(100);
    elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(100 + 100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "third poll should timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT because there were no messages left";
    delete handler;
}

TEST_F(LooperTest, SendMessageDelayed_WhenSentToThePast_ShouldInvokeHandlerDuringNextPoll) {
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessageDelayed(-1000 /*ms*/, handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    delete handler;
}

TEST_F(LooperTest, SendMessageDelayed_WhenSentToThePresent_ShouldInvokeHandlerDuringNextPoll) {
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessageDelayed(0 /*ms*/, handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    delete handler;
}

TEST_F(LooperTest, SendMessageAtTime_WhenSentToTheFuture_ShouldInvokeHandlerAfterDelayTime) {
    nsecs_t now = SystemClock::uptimeMillis();
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessageAtTime(now + 100, handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "first poll should end quickly because next message timeout was computed";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be POLL_WAKE due to wakeup";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no message handled yet";

    result = mLooper->pollOnce(1000);
    elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "second poll should end around the time of the delayed message dispatch";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";

    result = mLooper->pollOnce(100);
    elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(100 + 100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "third poll should timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT because there were no messages left";
    delete handler;
}

TEST_F(LooperTest, SendMessageAtTime_WhenSentToThePast_ShouldInvokeHandlerDuringNextPoll) {
    nsecs_t now = SystemClock::uptimeMillis();
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessageAtTime(now - 1000, handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    delete handler;
}

TEST_F(LooperTest, SendMessageAtTime_WhenSentToThePresent_ShouldInvokeHandlerDuringNextPoll) {
    nsecs_t now = SystemClock::uptimeMillis();
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessageAtTime(now, handler, makeMessage(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0])
            << "handled message";
    delete handler;
}

TEST_F(LooperTest, RemoveMessage_WhenRemovingAllMessagesForHandler_ShouldRemoveThoseMessage) {
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessage(handler, makeMessage(MSG_TEST1));
    mLooper->sendMessage(handler, makeMessage(MSG_TEST2));
    mLooper->sendMessage(handler, makeMessage(MSG_TEST3));
    mLooper->removeMessages(handler);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was sent so looper was awoken";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be POLL_WAKE because looper was awoken";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no messages to handle";

    result = mLooper->pollOnce(0);

    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT because there was nothing to do";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no messages to handle";
    delete handler;
}

TEST_F(LooperTest, RemoveMessage_WhenRemovingSomeMessagesForHandler_ShouldRemoveThoseMessage) {
    StubMessageHandler* handler = new StubMessageHandler();
    mLooper->sendMessage(handler, makeMessage(MSG_TEST1));
    mLooper->sendMessage(handler, makeMessage(MSG_TEST2));
    mLooper->sendMessage(handler, makeMessage(MSG_TEST3));
    mLooper->sendMessage(handler, makeMessage(MSG_TEST4));
    mLooper->removeMessages(handler, MSG_TEST3);
    mLooper->removeMessages(handler, MSG_TEST1);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = stopWatch.elapsedTimeMillis();

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was sent so looper was awoken";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be POLL_CALLBACK because two messages were sent";
    EXPECT_EQ(size_t(2), handler->messages.size())
            << "no messages to handle";
    EXPECT_EQ(MSG_TEST2, handler->messages[0])
            << "handled message";
    EXPECT_EQ(MSG_TEST4, handler->messages[1])
            << "handled message";

    result = mLooper->pollOnce(0);

    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be POLL_TIMEOUT because there was nothing to do";
    EXPECT_EQ(size_t(2), handler->messages.size())
            << "no more messages to handle";
    delete handler;
}

}//endof namespace cdroid

#endif /* __linux__ || __unix__ */
