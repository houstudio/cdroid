/*********************************************************************************
 * Copyright (C) 2019 houzh@msn.com
 *
 * See pendingintent.h for the in-process adaptation notes.
 *********************************************************************************/
#include <app/pendingintent.h>
#include <core/intent.h>
#include <porting/cdlog.h>

namespace cdroid{

PendingIntent::PendingIntent(const std::string& target, int requestCode, Intent* intent, int flags)
    : mTarget(target)
    , mRequestCode(requestCode)
    , mFlags(flags)
    , mCanceled(false)
    , mIntent(nullptr) {
    if (intent) mIntent = new Intent(*intent);
}

PendingIntent::~PendingIntent() {
    delete mIntent;
}

PendingIntent* PendingIntent::getActivity(Context* context, int requestCode, Intent* intent, int flags) {
    return new PendingIntent("activity", requestCode, intent, flags);
}

PendingIntent* PendingIntent::getBroadcast(Context* context, int requestCode, Intent* intent, int flags) {
    return new PendingIntent("broadcast", requestCode, intent, flags);
}

PendingIntent* PendingIntent::getService(Context* context, int requestCode, Intent* intent, int flags) {
    return new PendingIntent("service", requestCode, intent, flags);
}

PendingIntent* PendingIntent::getActivity(Context* context, int requestCode, Intent* intent, int flags, const OnSend& onSend) {
    PendingIntent* pi = getActivity(context, requestCode, intent, flags);
    pi->setOnSend(onSend);
    return pi;
}

PendingIntent* PendingIntent::getBroadcast(Context* context, int requestCode, Intent* intent, int flags, const OnSend& onSend) {
    PendingIntent* pi = getBroadcast(context, requestCode, intent, flags);
    pi->setOnSend(onSend);
    return pi;
}

PendingIntent* PendingIntent::getService(Context* context, int requestCode, Intent* intent, int flags, const OnSend& onSend) {
    PendingIntent* pi = getService(context, requestCode, intent, flags);
    pi->setOnSend(onSend);
    return pi;
}

void PendingIntent::setOnSend(const OnSend& onSend) {
    mOnSend = onSend;
}

void PendingIntent::send() {
    send(nullptr, 0, nullptr);
}

void PendingIntent::send(Context* /*context*/, int /*code*/, Intent* fillIn) {
    if (mCanceled) {
        LOGW("PendingIntent(%s): send() after cancel", mTarget.c_str());
        return;
    }
    if (!mIntent) {
        LOGW("PendingIntent(%s): send() with no intent", mTarget.c_str());
        return;
    }
    if (fillIn) {
        /* PendingIntent.send() merges the caller's fill-in into the base
         * intent per Intent.FILL_IN_* rules (only still-empty fields); the
         * flag argument is FILL_IN bits, not the PendingIntent flags. */
        mIntent->fillIn(*fillIn, 0);
    }
    if (!mOnSend) {
        LOGW("PendingIntent(%s): send() with no OnSend handler registered "
             "(use the WithHandler factory overload)", mTarget.c_str());
    }
    mOnSend(*mIntent);
    if (mFlags & FLAG_ONE_SHOT) {
        cancel();
    }
}

void PendingIntent::cancel() {
    mCanceled = true;
}

bool PendingIntent::isCanceled() const {
    return mCanceled;
}

Intent* PendingIntent::getIntent() const {
    return mIntent;
}

} // namespace cdroid
