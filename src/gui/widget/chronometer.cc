#include <widget/chronometer.h>
#include <systemclock.h>
namespace cdroid{

Chronometer::Chronometer(int w,int h):TextView(std::string(),w,h){
    mTickRunnable=nullptr;
}

Chronometer::Chronometer(Context*ctx,const AttributeSet&atts):TextView(ctx,atts){
    mTickRunnable=nullptr;
}
    
void Chronometer::setCountDown(bool countDown) {
    mCountDown = countDown;
    updateText(SystemClock::elapsedRealtime());
}

bool Chronometer::isCountDown()const{
    return mCountDown;
}

bool Chronometer::isTheFinalCountDown()const{
    /*getContext().startActivity(
        new Intent(Intent.ACTION_VIEW, Uri.parse("https://youtu.be/9jK-NcRmVcw"))
                .addCategory(Intent.CATEGORY_BROWSABLE)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_DOCUMENT
                        | Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT));*/
    return true;
}


void Chronometer::setBase(long base){
    mBase = base;
    dispatchChronometerTick();
    updateText(SystemClock::elapsedRealtime());
}

long Chronometer::getBase()const {
    return mBase;
}

void Chronometer::setFormat(const std::string& format) {
    mFormat = format;
    /*if (format != nullptr && mFormatBuilder == nullptr) {
        mFormatBuilder = new StringBuilder(format.length() * 2);
    }*/
}


std::string Chronometer::getFormat()const{
    return mFormat;
}

void Chronometer::setOnChronometerTickListener(OnChronometerTickListener listener) {
    mOnChronometerTickListener = listener;
}


Chronometer::OnChronometerTickListener Chronometer::getOnChronometerTickListener()const{
    return mOnChronometerTickListener;
}

void Chronometer::start() {
    mStarted = true;
    updateRunning();
}

void Chronometer::stop() {
    mStarted = false;
    updateRunning();
}

void Chronometer::setStarted(bool started) {
    mStarted = started;
    updateRunning();
}

void Chronometer::updateText(long now) {
    mNow = now;
    long seconds = mCountDown ? mBase - now : now - mBase;
    seconds /= 1000;
    bool negative = false;
    if (seconds < 0) {
        seconds = -seconds;
        negative = true;
    }
    std::string text ;//= DateUtils.formatElapsedTime(mRecycle, seconds);
    if (negative) {
        //text = getResources().getString(R.string.negative_duration, text);
    }

    /*if (mFormat != nullptr) {
        Locale loc = Locale.getDefault();
        if (mFormatter == nullptr || !loc.equals(mFormatterLocale)) {
            mFormatterLocale = loc;
            mFormatter = new Formatter(mFormatBuilder, loc);
        }
        mFormatBuilder.setLength(0);
        mFormatterArgs[0] = text;

        mFormatter.format(mFormat, mFormatterArgs);
        text = mFormatBuilder.toString();
    }*/
    setText(text);
}

void Chronometer::updateRunning() {
    bool running = mVisible && mStarted && isShown();
    if(mTickRunnable==nullptr)
        mTickRunnable=std::bind(&Chronometer::tickRunner,this);
    if (running != mRunning) {
        if (running) {
            updateText(SystemClock::elapsedRealtime());
            dispatchChronometerTick();
            postDelayed(mTickRunnable, 1000);
        } else {
            removeCallbacks(mTickRunnable);
        }
        mRunning = running;
    }
}

void Chronometer::tickRunner(){
    if (mRunning) {
        updateText(SystemClock::elapsedRealtime());
        dispatchChronometerTick();
        postDelayed(mTickRunnable, 1000);
    }
}

void Chronometer::dispatchChronometerTick() {
    if (mOnChronometerTickListener != nullptr) {
        mOnChronometerTickListener(*this);
    }
}

static constexpr int MIN_IN_SEC = 60;
static constexpr int HOUR_IN_SEC = MIN_IN_SEC*60;
static constexpr int SECOND_IN_MILLIS=1000;
std::string Chronometer::formatDuration(long ms) {
    int duration = (int) (ms / SECOND_IN_MILLIS);
    if (duration < 0) {
        duration = -duration;
    }

    int h = 0;
    int m = 0;

    if (duration >= HOUR_IN_SEC) {
        h = duration / HOUR_IN_SEC;
        duration -= h * HOUR_IN_SEC;
    }
    if (duration >= MIN_IN_SEC) {
        m = duration / MIN_IN_SEC;
        duration -= m * MIN_IN_SEC;
    }
    int s = duration;

    /*ArrayList<Measure> measures = new ArrayList<Measure>();
    if (h > 0) {
        measures.add(new Measure(h, MeasureUnit.HOUR));
    }
    if (m > 0) {
        measures.add(new Measure(m, MeasureUnit.MINUTE));
    }
    measures.add(new Measure(s, MeasureUnit.SECOND));

    return MeasureFormat.getInstance(Locale.getDefault(), FormatWidth.WIDE)
                .formatMeasures(measures.toArray(new Measure[measures.size()]));*/
    return "";
}

std::string Chronometer::getContentDescription()const{
    return formatDuration(mNow - mBase);
}


}
