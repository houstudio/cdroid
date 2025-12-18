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
#include <widget/chronometer.h>
#include <systemclock.h>
namespace cdroid{

DECLARE_WIDGET(Chronometer)

Chronometer::Chronometer(int w,int h):TextView(std::string(),w,h){
    init();
}

Chronometer::Chronometer(Context*ctx,const AttributeSet&atts)
  :TextView(ctx,atts){
    init();
    setFormat(atts.getString("format",mFormat));
    setCountDown(atts.getBoolean("countDown",false));
    mColonBlinking = atts.getBoolean("colonBlinking",mColonBlinking);
}

void Chronometer::init(){
    mBase = SystemClock::elapsedRealtime();
    mStarted = false;
    mCountDown = false;
    mColonBlinking = false;
    mFormat = "MM:SS";
    updateText(mBase);
    mTickRunnable = [this](){tickRunner();};
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
                .addCategory(Intent::CATEGORY_BROWSABLE)
                .addFlags(Intent::FLAG_ACTIVITY_NEW_DOCUMENT
                        | Intent::FLAG_ACTIVITY_LAUNCH_ADJACENT));*/
    return true;
}


void Chronometer::setBase(int64_t base){
    mBase = base;
    dispatchChronometerTick();
    updateText(SystemClock::elapsedRealtime());
}

int64_t Chronometer::getBase()const {
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

void Chronometer::setOnChronometerTickListener(const OnChronometerTickListener& listener) {
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

void Chronometer::updateText(int64_t now) {
    mNow = now;
    int64_t seconds = mCountDown ? mBase - now : now - mBase;
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

    if (1) {
        /*Locale loc = Locale.getDefault();
        if (mFormatter == nullptr || !loc.equals(mFormatterLocale)) {
            mFormatterLocale = loc;
            mFormatter = new Formatter(mFormatBuilder, loc);
        }
        mFormatBuilder.setLength(0);
        mFormatterArgs[0] = text;

        mFormatter.format(mFormat, mFormatterArgs);
        text = mFormatBuilder.toString();*/
        text = mFormat;
        auto pos = text.find("SS");
        char stm[32];
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
    }
    setText(text);
}

void Chronometer::updateRunning() {
    const bool running = mVisible && mStarted && isShown();
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
std::string Chronometer::formatDuration(int64_t ms) {
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

std::string Chronometer::getAccessibilityClassName()const{
    return "Chronometer";
}

}
