/*********************************************************************************
 * Copyright (C) 2019 houzh@msn.com
 *
 * C++ port of android.app.PendingIntent -- in-process adaptation.
 *
 * Upstream PendingIntent is a binder token held by the system so a target
 * package can be launched later even after the scheduling process is gone
 * (see PendingIntent.java). CDROID is single process with no system_server
 * and no BroadcastReceiver/Activity dispatch, so there is nothing for a bare
 * token to deliver into. The factory surface (getActivity/getBroadcast/
 * getService, flag constants, cancel, getIntent) is ported 1:1; delivery
 * instead invokes the OnSend callback given to the WithHandler factory
 * overloads (the CDROID seam -- upstream parcels the Intent across binder).
 *
 * Ownership: the caller owns the PendingIntent (raw pointer); AlarmManager
 * only borrows it. Identity for AlarmManager::cancel() is the pointer.
 *********************************************************************************/
#ifndef __CDROID_APP_PENDINGINTENT_H__
#define __CDROID_APP_PENDINGINTENT_H__

#include <core/callbackbase.h>
#include <string>

namespace cdroid{

class Context;
class Intent;

class PendingIntent{
public:
    /** Flag bits (PendingIntent.java), values match upstream. */
    enum {
        FLAG_IMMUTABLE     = 1<<26,
        FLAG_MUTABLE       = 1<<25,
        FLAG_UPDATE_CURRENT= 1<<27,
        FLAG_CANCEL_CURRENT= 1<<28,
        FLAG_NO_CREATE     = 1<<29,
        FLAG_ONE_SHOT      = 1<<30,
    };

    /** In-process stand-in for "the system fires the target component": the
     *  callback invoked by send(); the argument is the wrapped Intent, filled
     *  in with the extras passed to send() (Intent::fillIn semantics). */
    using OnSend = CallbackBase<void, Intent&>;

    /** Upstream factories (PendingIntent.java:575+/449+/521+). Without an
     *  OnSend the returned PendingIntent::send() logs and no-ops -- attach one
     *  via the WithHandler overloads or setOnSend(). */
    static PendingIntent* getActivity(Context* context, int requestCode, Intent* intent, int flags);
    static PendingIntent* getBroadcast(Context* context, int requestCode, Intent* intent, int flags);
    static PendingIntent* getService(Context* context, int requestCode, Intent* intent, int flags);

    /** CDROID seam: factories that also register the in-process delivery
     *  target (see OnSend above). */
    static PendingIntent* getActivity(Context* context, int requestCode, Intent* intent, int flags, const OnSend& onSend);
    static PendingIntent* getBroadcast(Context* context, int requestCode, Intent* intent, int flags, const OnSend& onSend);
    static PendingIntent* getService(Context* context, int requestCode, Intent* intent, int flags, const OnSend& onSend);

    ~PendingIntent();

    /** Attach / replace the delivery target. */
    void setOnSend(const OnSend& onSend);

    /** IntentSender.send(): dispatch to the registered OnSend. FLAG_ONE_SHOT
     *  cancels the PendingIntent after one send, as upstream. */
    void send();
    /** Variant honoring the fillIn intent (merged into the base intent per
     *  the PendingIntent flags, via Intent::fillIn). */
    void send(Context* context, int code, Intent* fillIn);

    /** Permanently cancel: further send() calls no-op. */
    void cancel();
    bool isCanceled() const;

    /** The wrapped intent (owned clone of the one passed to the factory). */
    Intent* getIntent() const;

    /** Kind of target component the factory named (upstream keeps this in
     *  the system; exposed here for logging/diagnostics). */
    const std::string& getTargetPackage() const { return mTarget; }

private:
    PendingIntent(const std::string& target, int requestCode, Intent* intent, int flags);

    std::string mTarget;      /* "activity" / "broadcast" / "service" */
    int mRequestCode;
    int mFlags;
    bool mCanceled;
    Intent* mIntent;          /* owned clone */
    OnSend mOnSend;
};

} // namespace cdroid
#endif/*__CDROID_APP_PENDINGINTENT_H__*/
