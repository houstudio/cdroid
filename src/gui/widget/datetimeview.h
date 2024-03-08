#ifndef __DATE_TIME_VIEW_H__
#define __DATE_TIME_VIEW_H__
namespace cdroid{

class DateTimeView public:TextView{
private:
    static constexpr int SHOW_TIME = 0;
    static constexpr int SHOW_MONTH_DAY_YEAR = 1;
private:
    long mTimeMillis;
    // The LocalDateTime equivalent of mTimeMillis but truncated to minute, i.e. no seconds / nanos.
    LocalDateTime mLocalTime;

    int mLastDisplay = -1;
    DateFormat mLastFormat;

    long mUpdateTimeMillis;
    //static final ThreadLocal<ReceiverInfo> sReceiverInfo = new ThreadLocal<ReceiverInfo>();
    std::string mNowText;
    bool mShowRelativeTime;
private:
    void updateRelativeTime();
    long computeNextMidnight(TimeZone timeZone);
protected:
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void update();
public:
    DateTimeView(Context* context,const AttributeSet& attrs);
    void setTime(long timeMillis);
    void setShowRelativeTime(bool showRelativeTime);
    void setVisibility(int visibility)override;
};

}/*endof namespace*/
#endif
