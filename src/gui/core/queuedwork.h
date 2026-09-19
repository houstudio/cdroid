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
#ifndef __CDROID_QUEUEDWORK_H__
#define __CDROID_QUEUEDWORK_H__
#include <core/callbackbase.h>
// CDROID placement note: AOSP keeps this in android.app, but it is a generic
// process-wide outstanding-work mechanism (its own javadoc says so); here it
// lives with the core runtime infrastructure, one layer below its users.

namespace cdroid{

/**
 * Port of android.app.QueuedWork — internal utility class to keep track of
 * process-global work that's outstanding and hasn't been finished yet.
 *
 * New work will be {@link #queue queued}.
 *
 * It is possible to add 'finisher'-runnables that are {@link #waitToFinish
 * guaranteed to be run}. This is used to make sure the work has been finished.
 *
 * This was created for writing SharedPreference edits out asynchronously so
 * we'd have a mechanism to wait for the writes in Activity.onPause and similar
 * places, but we may use this mechanism for other things in the future.
 *
 * The queued asynchronous work is performed on a separate, dedicated thread
 * — a {@link HandlerThread}("queued-work-looper") driven through
 * {@link Handler} messages, exactly the AOSP carrier
 * (sendEmptyMessageDelayed(MSG_RUN, 100) coalesces bursts of apply() edits
 * into one drain).
 */
class QueuedWork{
public:
    using Runnable = cdroid::Runnable;

    /**
     * Add a finisher-runnable to wait for {@link #queue asynchronously
     * processed work}.
     *
     * Note that this doesn't actually start it running. This is just a
     * scratch set for callers doing async work to keep updated with what's
     * in-flight. The only time these Runnables are run is from
     * {@link #waitToFinish}.
     *
     * Removal is by callback identity (CallbackBase's shared-functor
     * pointer), the AOSP LinkedList#remove(object-equals) role.
     */
    static void addFinisher(const Runnable& finisher);

    /**
     * Remove a previously {@link #addFinisher added} finisher-runnable.
     */
    static void removeFinisher(const Runnable& finisher);

    /**
     * Trigger queued work to be processed immediately. The queued work is
     * processed on a separate thread asynchronous. While doing that run and
     * process all finishers on this thread. The finishers can be implemented
     * in a way to check whether the queued work is finished.
     *
     * Is called on the CDROID exit path (App::exit / ~App) — the stand-in
     * for AOSP's Activity.onPause / after BroadcastReceiver's onReceive
     * checkpoints, so async work is never lost.
     */
    static void waitToFinish();

    /**
     * Queue a work-runnable for processing asynchronously.
     *
     * @param work The new runnable to process
     * @param shouldDelay If the work can be delayed (100 ms) to coalesce
     */
    static void queue(const Runnable& work, bool shouldDelay);

    /**
     * @return True iff there is any {@link #queue async work queued}.
     */
    static bool hasPendingWork();

    /**
     * CDROID seam: park the worker thread after a final drain. AOSP only
     * tears the handler down in tests (resetHandler); CDROID has no
     * process-wide GC teardown, so the exit path stops the thread explicitly
     * (the thread is detached — quitSafely only asks it to leave, it must
     * not be waited on from a static destructor).
     */
    static void quitSafely();
private:
    QueuedWork() = delete;   // static-only utility, per AOSP (all-static class)
    static void processPendingWork();
};

}/*endof namespace*/
#endif/*__CDROID_QUEUEDWORK_H__*/
