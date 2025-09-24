#ifndef __CIRCUAR_PROGRESS_LAYOUT_H__
#define __CIRCUAR_PROGRESS_LAYOUT_H__
#include <widget/framelayout.h>
namespace cdroid{
class CircularProgressDrawable;
class CircularProgressLayoutController;
class CircularProgressLayout:public FrameLayout {
private:
    static constexpr long DEFAULT_UPDATE_INTERVAL = 1000 / 60;
    static constexpr float DEFAULT_ROTATION = 0.75f;

    CircularProgressDrawable* mProgressDrawable;
    CircularProgressLayoutController* mController;

    float mStartingRotation = DEFAULT_ROTATION;
    long mTotalTime;
private:
    std::vector<int> getColorListFromResources(const std::string& arrayResId);
protected:
    void onLayout(bool changed, int left, int top, int width, int heigt) override;
    void onDetachedFromWindow() override;
public:
    DECLARE_UIEVENT(void,OnTimerFinishedListener,CircularProgressLayout&);

    CircularProgressLayout(Context* context,const AttributeSet& attrs);
    ~CircularProgressLayout()override;

    void setBackgroundColor(int color) override;
    int getBackgroundColor();

    CircularProgressDrawable* getProgressDrawable();

    void setIndeterminate(bool indeterminate);
    bool isIndeterminate() const;

    void setTotalTime(long totalTime);
    long getTotalTime() const;

    void startTimer();
    void stopTimer();
    bool isTimerRunning() const;

    void setStartingRotation(float rotation);
    float getStartingRotation() const;

    void setStrokeWidth(float strokeWidth);
    float getStrokeWidth() const;

    void setColorSchemeColors(const std::vector<int>& colors);
    std::vector<int> getColorSchemeColors() const;

    OnTimerFinishedListener getOnTimerFinishedListener()const;
    void setOnTimerFinishedListener(const OnTimerFinishedListener& listener);
};
}/*endof namespace*/
#endif/*__CIRCUAR_PROGRESS_LAYOUT_H__*/
