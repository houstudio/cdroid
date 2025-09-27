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
*/
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
