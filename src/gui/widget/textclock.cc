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
#include <widget/internal_R.h>
#include <widget/textclock.h>
#include <widget/framework_styleable.h>
#include <core/assets.h>
#include <core/systemclock.h>
#include <content/dateformat.h>
#include <utils/textutils.h>
#include <climits>
#include <cstdio>
#include <ctime>

namespace cdroid{
using namespace cdroid::internal;
#if 0
private class FormatChangeObserver extends ContentObserver {
    public FormatChangeObserver(Handler handler) {
        super(handler);
    }
    public void onChange(bool selfChange) override{
        chooseFormat();
        onTimeChanged();
    }
    public void onChange(bool selfChange, Uri uri) override{
        chooseFormat();
        onTimeChanged();
    }
};

private final BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
    public void onReceive(Context context, Intent intent) override{
        if (mStopTicking) {
            return; // Test disabled the clock ticks
        }
        if (mTimeZone == null && Intent.ACTION_TIMEZONE_CHANGED.equals(intent.getAction())) {
            final String timeZone = intent.getStringExtra(Intent.EXTRA_TIMEZONE);
            createTime(timeZone);
        } else if (!mShouldRunTicker && Intent.ACTION_TIME_CHANGED.equals(intent.getAction())) {
            return;
        }
        onTimeChanged();
    }
};
#endif

DECLARE_WIDGET(TextClock)

namespace {
// TimeZone.getDefault() analog: the host's local UTC offset (TZ env), in
// seconds — what a wall clock should display when no zone is set.
int localZoneOffsetSeconds() {
    const time_t now = ::time(nullptr);
    return (int)localtime(&now)->tm_gmtoff;
}

// Parse a TimeZone custom ID ("GMT+08:00", "GMT-8") into offset seconds;
// returns INT_MIN when the string is not a custom ID. Olson IDs
// ("America/Los_Angeles") cannot be resolved without tzdata — callers fall
// back to the local zone (documented gap).
int parseCustomZoneSeconds(const std::string& id) {
    if (id.compare(0, 3, "GMT") != 0 && id.compare(0, 3, "gmt") != 0) return INT_MIN;
    const size_t sign = 3;
    if (sign >= id.size() || (id[sign] != '+' && id[sign] != '-')) return INT_MIN;
    int hours = 0, minutes = 0;
    int consumed = 0;
    if (sscanf(id.c_str() + sign + 1, "%d%n", &hours, &consumed) != 1) return INT_MIN;
    size_t rest = sign + 1 + consumed;
    if (rest < id.size() && id[rest] == ':') {
        if (sscanf(id.c_str() + rest + 1, "%d", &minutes) != 1) return INT_MIN;
    }
    const int total = hours * 3600 + minutes * 60;
    return id[sign] == '-' ? -total : total;
}
} // namespace

TextClock::TextClock(Context*ctx)
    :TextClock(ctx,nullptr){}

TextClock::TextClock(Context* context,const AttributeSet* attrs):TextClock(context,attrs,0){}

TextClock::TextClock(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :TextView(context, pAttrs, defStyleAttr){
    // AOSP reads the attributes first, then init() runs chooseFormat() on
    // them (init-then-read would clobber the locale defaults with empty
    // strings when the attributes are absent).
    // Phase 2: TypedArray (binary AXML typed resolution). ta=null → text XML fallback.
    auto ta = context->obtainStyledAttributes(pAttrs, R::styleable::TextClock, defStyleAttr);

    mFormat12 = ta->getString(R::styleable::TextClock_format12Hour);
    mFormat24 = ta->getString(R::styleable::TextClock_format24Hour);
    mTimeZone = ta->getString(R::styleable::TextClock_timeZone);

    init();
}

void TextClock::init() {
    mHasSeconds = false;
    mRegistered = false;
    mStopTicking= false;
    mShouldRunTicker = true;
    mShowCurrentUserTime = false;
    if (mFormat12.empty()) {
        mFormat12 = getBestDateTimePattern("hm");
    }
    if (mFormat24.empty()) {
        mFormat24 = getBestDateTimePattern("Hm");
    }
    mClockEventDelegate = nullptr;//new ClockEventDelegate(getContext());
    mTicker = [this](){doTick();};
    createTime(mTimeZone);
    chooseFormat();
}

void TextClock::doTick() {
    removeCallbacks(mTicker);
    if (mStopTicking || !mShouldRunTicker) {
        return; // Test disabled the clock ticks
    }
    onTimeChanged();
#if 0
    Instant now = mTime.toInstant();
    ZoneId zone = mTime.getTimeZone().toZoneId();

    ZonedDateTime nextTick;
    if (mHasSeconds) {
        nextTick = now.atZone(zone).plusSeconds(1).withNano(0);
    } else {
        nextTick = now.atZone(zone).plusMinutes(1).withSecond(0).withNano(0);
    }

    int64_t millisUntilNextTick = Duration.between(now, nextTick.toInstant()).toMillis();
    if (millisUntilNextTick <= 0) {
        // This should never happen, but if it does, then tick again in a second.
        millisUntilNextTick = 1000;
    }
    postDelayed(mTicker, millisUntilNextTick);
#else
    const int64_t now = mTime.getTimeInMillis();
    int64_t nextSecondMillis;
    if(mHasSeconds){
        nextSecondMillis = (now / 1000 + 1) * 1000;
    }else{
        nextSecondMillis = (now / 60000 +1) * 60000;
    }
    long millisUntilNextTick = long(nextSecondMillis - now);
    if(millisUntilNextTick <= 0){
        millisUntilNextTick = 1000;
    }
    postDelayed(mTicker,millisUntilNextTick);
#endif
}

void TextClock::createTime(const std::string& timeZone) {
    // AOSP: TimeZone.getDefault() when unset, else TimeZone.getTimeZone(id).
    // CDROID has no TimeZone engine: custom "GMT±hh[:mm]" IDs parse to a raw
    // offset; anything else (unset or an Olson ID) uses the host's local zone.
    if (!timeZone.empty()) {
        const int custom = parseCustomZoneSeconds(timeZone);
        if (custom != INT_MIN) {
            mTime.setTimeZone(custom);
            return;
        }
        LOGW("TextClock: time zone '%s' is not a custom GMT id; using the local zone",
             timeZone.c_str());
    }
    mTime.setTimeZone(localZoneOffsetSeconds());
}

std::string TextClock::getFormat12Hour() const{
    return mFormat12;
}

void TextClock::setFormat12Hour(const std::string& format) {
    mFormat12 = format;
    chooseFormat();
    onTimeChanged();
}

void TextClock::setContentDescriptionFormat12Hour(const std::string& format) {
    mDescFormat12 = format;
    chooseFormat();
    onTimeChanged();
}

std::string TextClock::getFormat24Hour() const{
    return mFormat24;
}

void TextClock::setFormat24Hour(const std::string& format) {
    mFormat24 = format;
    chooseFormat();
    onTimeChanged();
}

void TextClock::setContentDescriptionFormat24Hour(const std::string& format) {
    mDescFormat24 = format;
    chooseFormat();
    onTimeChanged();
}

void TextClock::setShowCurrentUserTime(bool showCurrentUserTime) {
    mShowCurrentUserTime = showCurrentUserTime;
    chooseFormat();
    onTimeChanged();
    unregisterObserver();
    registerObserver();
}

void TextClock::setClockEventDelegate(ClockEventDelegate* delegate) {
    LOGI_IF(!mRegistered, "Clock events already registered");
    mClockEventDelegate = delegate;
}

void TextClock::refreshTime() {
    onTimeChanged();
    invalidate();
}

bool TextClock::is24HourModeEnabled()const {
    if (mShowCurrentUserTime) {
        return DateFormat::is24HourFormat(getContext());//, ActivityManager.getCurrentUser()
    } else {
        return DateFormat::is24HourFormat(getContext());
    }
}

std::string TextClock::getTimeZone() const{
    return mTimeZone;
}

void TextClock::setTimeZone(const std::string& timeZone) {
    mTimeZone = timeZone;
    createTime(timeZone);
    onTimeChanged();
}

std::string TextClock::getFormat() const{
    return mFormat;
}

static std::string abc(const std::string& a, const std::string& b, const std::string& c) {
    return a.empty() ? (b.empty() ? c : b) : a;
}

void TextClock::chooseFormat() {
    const bool format24Requested = is24HourModeEnabled();

    if (format24Requested) {
        mFormat = abc(mFormat24, mFormat12, getBestDateTimePattern("Hm"));
        mDescFormat = abc(mDescFormat24, mDescFormat12, mFormat);
    } else {
        mFormat = abc(mFormat12, mFormat24, getBestDateTimePattern("hm"));
        mDescFormat = abc(mDescFormat12, mDescFormat24, mFormat);
    }

    const bool hadSeconds = mHasSeconds;
    mHasSeconds = DateFormat::hasSeconds(mFormat);

    if (mShouldRunTicker && (hadSeconds != mHasSeconds)) {
        mTicker();
    }
}

const std::string TextClock::getBestDateTimePattern(const std::string& skeleton) {
    // AOSP: DateTimePatternGenerator.getInstance(
    //       getContext().getResources().getConfiguration().getLocales().get(0))
    //       .getBestPattern(skeleton). CDROID has no DTPG; the content
    //       DateFormat serves the "hm"/"Hm" skeletons from the i18n engine's
    //       per-locale hour+minute pattern pools.
    return DateFormat::getBestDateTimePattern(Locale::getDefault(), skeleton);
}

void TextClock::onAttachedToWindow() {
    TextView::onAttachedToWindow();

    if (!mRegistered) {
        mRegistered = true;
        //mClockEventDelegate->registerTimeChangeReceiver(mIntentReceiver, getHandler());
        registerObserver();
        createTime(mTimeZone);
    }
}

void TextClock::onVisibilityAggregated(bool isVisible) {
    TextView::onVisibilityAggregated(isVisible);

    if (!mShouldRunTicker && isVisible) {
        mShouldRunTicker = true;
        mTicker();
    } else if (mShouldRunTicker && !isVisible) {
        mShouldRunTicker = false;
        removeCallbacks(mTicker);
    }
}

void TextClock::onDetachedFromWindow() {
    TextView::onDetachedFromWindow();
    if (mRegistered) {
        //mClockEventDelegate->unregisterTimeChangeReceiver(mIntentReceiver);
        unregisterObserver();
        mRegistered = false;
    }
}

void TextClock::disableClockTick() {
    mStopTicking = true;
}

void TextClock::registerObserver() {
    /*if (mRegistered) {
        if (mFormatChangeObserver == null) {
            mFormatChangeObserver = new FormatChangeObserver(getHandler());
        }
        mClockEventDelegate->registerFormatChangeObserver(mFormatChangeObserver, userHandle);
    }*/
}

void TextClock::unregisterObserver() {
    /*if (mFormatChangeObserver != null) {
        mClockEventDelegate->unregisterFormatChangeObserver(mFormatChangeObserver);
    }*/
}

void TextClock::onTimeChanged() {
    // AOSP: DateFormat.format(mFormat, mTime) — the formatter adopts the
    // calendar's zone (set by createTime for the attr/local zone).
    mTime.setTimeInMillis(SystemClock::currentTimeMillis());
    setText(DateFormat::format(mFormat, mTime));
    setContentDescription(DateFormat::format(mDescFormat, mTime));
}

/*void TextClock::encodeProperties(ViewHierarchyEncoder stream) {
    TextView::encodeProperties(stream);

    std::string s = getFormat12Hour();
    stream.addProperty("format12Hour", s == null ? null : s.toString());

    s = getFormat24Hour();
    stream.addProperty("format24Hour", s == null ? null : s.toString());
    stream.addProperty("format", mFormat == null ? null : mFormat.toString());
    stream.addProperty("hasSeconds", mHasSeconds);
}

public static class ClockEventDelegate {
    private final Context mContext;
    public ClockEventDelegate(Context context) {
        mContext = context;
    }

    public void registerTimeChangeReceiver(BroadcastReceiver receiver, Handler handler) {
        final IntentFilter filter = new IntentFilter();

        filter.addAction(Intent.ACTION_TIME_CHANGED);
        filter.addAction(Intent.ACTION_TIMEZONE_CHANGED);

        mContext.registerReceiverAsUser(receiver, myUserHandle(), filter, null, handler);
    }

    public void unregisterTimeChangeReceiver(BroadcastReceiver receiver) {
        mContext.unregisterReceiver(receiver);
    }

    public void registerFormatChangeObserver(ContentObserver observer, int userHandle) {
        Uri uri = Settings.System.getUriFor(Settings.System.TIME_12_24);
        mContext.getContentResolver().registerContentObserver(uri, true, observer, userHandle);
    }

    public void unregisterFormatChangeObserver(ContentObserver observer) {
        mContext.getContentResolver().unregisterContentObserver(observer);
    }
};*/
}/*endof namespace*/
