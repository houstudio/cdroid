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
#ifndef __CIRCULAR_PROGRESS_DRAWABLE_H__
#define __CIRCULAR_PROGRESS_DRAWABLE_H__
#include <drawable/drawable.h>
#include <animation/animator.h>
namespace cdroid{
class CircularProgressDrawable:public Drawable ,public Animatable {
private:
    static constexpr float CENTER_RADIUS_LARGE = 11.f;
    static constexpr float STROKE_WIDTH_LARGE = 3.f;
    static constexpr int ARROW_WIDTH_LARGE = 12;
    static constexpr int ARROW_HEIGHT_LARGE = 6;


    static constexpr float CENTER_RADIUS = 7.5f;
    static constexpr float STROKE_WIDTH = 2.5f;
    static constexpr int ARROW_WIDTH = 10;
    static constexpr int ARROW_HEIGHT = 5;

    static constexpr float COLOR_CHANGE_OFFSET = 0.75f;
    static constexpr float SHRINK_OFFSET = 0.5f;

    static constexpr int ANIMATION_DURATION = 1332;

    static constexpr float GROUP_FULL_ROTATION = 1080.f / 5.f;
    static constexpr float MAX_PROGRESS_ARC = 0.8f;
    static constexpr float MIN_PROGRESS_ARC = 0.01f;

    static constexpr float RING_ROTATION = 1.f - (MAX_PROGRESS_ARC - MIN_PROGRESS_ARC);
private:
    class Ring;
    Ring* mRing;
    Context*mContext;
    Animator* mAnimator;
    float mRotation;
    float mRotationCount;
    bool mFinishing;
    
    void setSizeParameters(float centerRadius, float strokeWidth, float arrowWidth,float arrowHeight);
    void setRotation(float rotation);
    float getRotation() const;
    int evaluateColorChange(float fraction, int startValue, int endValue);
    void applyFinishTranslation(float interpolatedTime, Ring* ring);
    void setupAnimators();
public:
    static constexpr int LARGE = 0;
    static constexpr int DEFAULT = 1;
public:
    CircularProgressDrawable(Context* context);
    ~CircularProgressDrawable()override;

    void setStyle(int size);

    float getStrokeWidth() const;
    void setStrokeWidth(float strokeWidth);

    float getCenterRadius() const;
    void setCenterRadius(float centerRadius);

    void setStrokeCap(int strokeCap);
    int getStrokeCap() const;

    float getArrowWidth() const;

    float getArrowHeight() const;
    void setArrowDimensions(float width, float height);

    bool getArrowEnabled() const;
    void setArrowEnabled(bool show);

    float getArrowScale() const;
    void setArrowScale(float scale);
    /**
     * Returns the start trim for the progress spinner arc
     *
     * @return start trim from [0..1]
     */
    float getStartTrim() const;
    float getEndTrim() const;
    /**
     * Sets the start and end trim for the progress spinner arc. 0 corresponds to the geometric
     * angle of 0 degrees (3 o'clock on a watch) and it increases clockwise, coming to a full circle
     * at 1.
     *
     * @param start starting position of the arc from [0..1]
     * @param end ending position of the arc from [0..1]
     */
    void setStartEndTrim(float start, float end);

    /**
     * Sets the amount of rotation to apply to the progress spinner.
     *
     * @param rotation rotation from [0..1]
     */
    float getProgressRotation() const;
    void setProgressRotation(float rotation);

    int getBackgroundColor() const;
    void setBackgroundColor(int color);

    std::vector<int> getColorSchemeColors() const;

    void setColorSchemeColors(const std::vector<int>&colors);

    void draw(Canvas& canvas) override;

    void setAlpha(int alpha) override;
    int getAlpha() const override;

    void setColorFilter(ColorFilter* colorFilter) override;

    int getOpacity() override;

    bool isRunning() override;

    void start() override;

    void stop() override;

    void updateRingColor(float interpolatedTime, Ring* ring);

    void applyTransformation(float interpolatedTime, Ring* ring, bool lastFrame);
};

class CircularProgressDrawable::Ring {
    
    RectF mTempBounds;
    /*Paint mPaint = new Paint();
    Paint mArrowPaint = new Paint();
    Paint mCirclePaint = new Paint();*/
    int mRingCap;
    int mCircleColor;

    float mStartTrim;
    float mEndTrim;
    float mRotation;
    float mStrokeWidth;

    std::vector<int> mColors;
    // mColorIndex represents the offset into the available mColors that the
    // progress circle should currently display. As the progress circle is
    // animating, the mColorIndex moves by one to the next available color.
    int mColorIndex;
    float mStartingStartTrim;
    float mStartingEndTrim;
    float mStartingRotation;
    float mArrowScale;
    float mRingCenterRadius;
    std::unique_ptr<Path> mArrow;
    int mArrowWidth;
    int mArrowHeight;
    int mAlpha;
    int mCurrentColor;
    bool mShowArrow;
public:
    Ring();

    void setArrowDimensions(float width, float height);

    void setStrokeCap(int strokeCap);
    int getStrokeCap()const;

    float getArrowWidth() const;
    float getArrowHeight() const;

    void draw(Canvas& c,const Rect& bounds);
    void drawTriangle(Canvas& c, float startAngle, float sweepAngle,const RectF& bounds);

    void setColors(const std::vector<int>&colors);
    std::vector<int> getColors() const;
    void setColor(int color);

    void setBackgroundColor(int color);
    int getBackgroundColor() const;

    void setColorIndex(int index);
    int getNextColor() const;
    int getNextColorIndex() const;

    void goToNextColor();

    void setColorFilter(ColorFilter* filter);

    void setAlpha(int alpha);
    int getAlpha() const;

    void setStrokeWidth(float strokeWidth);
    float getStrokeWidth() const;

    void setStartTrim(float startTrim);
    float getStartTrim() const;

    float getStartingStartTrim() const;
    float getStartingEndTrim() const;

    int getStartingColor() const;
    void setEndTrim(float endTrim);
    float getEndTrim() const;

    void setRotation(float rotation);
    float getRotation() const;

    void setCenterRadius(float centerRadius);
    float getCenterRadius() const;

    void setShowArrow(bool show);
    bool getShowArrow() const;

    void setArrowScale(float scale);
    float getArrowScale() const;

    float getStartingRotation() const;

    void storeOriginals();
    void resetOriginals();
};
}/*endof namespace*/
#endif/*__CIRCULAR_PROGRESS_DRAWABLE_H__*/
