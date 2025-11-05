#include <widget/textclock.h>
#include <core/systemclock.h>

namespace cdroid{
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

TextClock::TextClock(Context* context,const AttributeSet& attrs)
    :TextView(context, attrs){

    mFormat12 = attrs.getString("format12Hour");
    mFormat24 = attrs.getString("format24Hour");
    mTimeZone = attrs.getString("timeZone");

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
    mTicker = std::bind(&TextClock::doTick,this);
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
#endif
}

void TextClock::createTime(const std::string& timeZone) {
#if 0
    TimeZone tz = null;
    if (timeZone == null) {
        tz = TimeZone.getDefault();
        // Note that mTimeZone should always be null if timeZone is.
    } else {
        tz = TimeZone.getTimeZone(timeZone);
        try {
            // Try converting this TZ to a zoneId to make sure it's valid. This
            // performs a different set of checks than TimeZone.getTimeZone so
            // we can avoid exceptions later when we do need this conversion.
            tz.toZoneId();
        } catch (DateTimeException ex) {
            // If we're here, the user supplied timezone is invalid, so reset
            // mTimeZone to something sane.
            tz = TimeZone.getDefault();
            mTimeZone = tz.getID();
        }
    }
    mTime = Calendar.getInstance(tz);
#endif
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
        return false;//DateFormat.is24HourFormat(getContext(), ActivityManager.getCurrentUser());
    } else {
        return false;//DateFormat.is24HourFormat(getContext());
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
    auto pos = mFormat.find_last_of(":")!=std::string::npos;
    mHasSeconds =(pos!=std::string::npos)&&((mFormat[pos+1]=='s')||(mFormat[pos+1]=='S'));
    //mHasSeconds=DateFormat.hasSeconds(mFormat);

    if (mShouldRunTicker && (hadSeconds != mHasSeconds)) {
        mTicker();
    }
}

const std::string TextClock::getBestDateTimePattern(const std::string& skeleton) {
    /*DateTimePatternGenerator dtpg = DateTimePatternGenerator.getInstance(
            getContext().getResources().getConfiguration().locale);
    return dtpg.getBestPattern(skeleton);*/return "";
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
    mTime.setTimeInMillis(SystemClock::currentTimeMillis());
    //setText(DateFormat.format(mFormat, mTime));
    //setContentDescription(DateFormat.format(mDescFormat, mTime));
    std::string text = mFormat;
    char stm[64];
    int64_t seconds=SystemClock::currentTimeMillis()/1000;
    auto pos = text.find("SS");
    const bool mColonBlinking=true;
    sprintf(stm,"%02d%c%02d",int(seconds/60),((((seconds%60)%2==0)||mColonBlinking==false)?':':' '),int(seconds%60));
    if(pos!=std::string::npos){
        sprintf(stm,"%02d",int(seconds%60));
        text.replace(pos,2,stm);
        if(mColonBlinking&&(seconds%2==0)){
            text.replace(pos-1,1," ");
        }
        pos=text.find("MM");
        if(pos!=std::string::npos){
            sprintf(stm,"%02d",int((seconds%3600)/60));
            text.replace(pos,2,stm);
        }
        pos=text.find("H");
        if(pos!=std::string::npos){
            sprintf(stm,"%d",int(seconds/3600));
            text.replace(pos,1+(text[pos+1]=='H'),stm);
        }
    }else{
        text = stm;
    }
    setText(text);
}

/*void TextClock::encodeProperties(ViewHierarchyEncoder stream) {
    TextView::encodeProperties(stream);

    std::string s = getFormat12Hour();
    stream.addProperty("format12Hour", s == null ? null : s.toString());

    s = getFormat24Hour();
    stream.addProperty("format24Hour", s == null ? null : s.toString());
    stream.addProperty("format", mFormat == null ? null : mFormat.toString());
    stream.addProperty("hasSeconds", mHasSeconds);
}*/

/*public static class ClockEventDelegate {
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
