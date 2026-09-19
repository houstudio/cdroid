#include <app/espresso/queueinterrogator.h>
#include <core/systemclock.h>

namespace cdroid {
namespace espresso {

QueueInterrogator::QueueInterrogator(Looper* interrogatedLooper)
    : mInterrogatedLooper(interrogatedLooper) {
    initializeQueue();
}

void QueueInterrogator::initializeQueue() {
    // Espresso runs on the main thread (AOSP captures the queue by posting a
    // one-shot handler task to the interrogated looper when off-thread).
    mInterrogatedQueue = mInterrogatedLooper->getQueue();
}

QueueInterrogator::QueueState QueueInterrogator::determineQueueState() const {
    // AOSP reflect the head: EMPTY / BARRIER (target == null) / when vs LOOKAHEAD.
    Message* head = mInterrogatedQueue->peek();
    if (head == nullptr) {
        // no messages pending - AT ALL!
        return QueueState::EMPTY;
    }
    if (head->target == nullptr) {
        return QueueState::BARRIER;
    }
    return head->when - SystemClock::uptimeMillis() <= LOOKAHEAD_MILLIS
            ? QueueState::TASK_DUE_SOON : QueueState::TASK_DUE_LONG;
}

Message* QueueInterrogator::getNextMessage() {
    return mInterrogatedQueue->nextDue();
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
