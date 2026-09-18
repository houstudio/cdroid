#ifndef CDROID_ESPRESSO_QUEUEINTERROGATOR_H
#define CDROID_ESPRESSO_QUEUEINTERROGATOR_H

/*
 * android.support.test.espresso.base.QueueInterrogator — isolates the nasty
 * details of touching the message queue.
 *
 * AOSP reaches into android.os.MessageQueue with reflection: mMessages gives
 * the head (EMPTY / BARRIER / head.when vs now+15ms), next() pops a message.
 * CDROID ports the queue itself, so the reflection becomes:
 *  - head interrogation: MessageQueue::peek() — the exact AOSP check
 *    (EMPTY / BARRIER / head.when vs LOOKAHEAD_MILLIS), no caller-side probe.
 *  - next() → MessageQueue::nextDue(), the non-blocking due-pop.
 */

#include <core/looper.h>
#include <core/message.h>
#include <core/messagequeue.h>

namespace cdroid {
namespace espresso {

class QueueInterrogator {
public:
    enum class QueueState { EMPTY, TASK_DUE_SOON, TASK_DUE_LONG, BARRIER };

    /** AOSP QueueInterrogator.LOOKAHEAD_MILLIS. */
    static constexpr int64_t LOOKAHEAD_MILLIS = 15;

    explicit QueueInterrogator(Looper* interrogatedLooper);

    /**
     * May be called from any thread.
     * AOSP determineQueueState: reflect the head — EMPTY when null, BARRIER
     * when target == null, else head.when vs LOOKAHEAD_MILLIS.
     */
    QueueState determineQueueState() const;

    /** Pops the next due message (AOSP: MessageQueue.next() by reflection). */
    Message* getNextMessage();

private:
    void initializeQueue();

    Looper* mInterrogatedLooper;
    MessageQueue* mInterrogatedQueue = nullptr;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_QUEUEINTERROGATOR_H*/
