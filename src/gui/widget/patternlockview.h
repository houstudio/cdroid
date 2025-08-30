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
#ifndef __PATTERN_LOCK_VIEW_H__
#define __PATTERN_LOCK_VIEW_H__
#include <view/view.h>
#include <limits>
#include <cfloat>
namespace cdroid{

class PatternLockView:public View{
private:
    static constexpr int DEFAULT_PATTERN_DOT_COUNT = 3;
    static constexpr bool PROFILE_DRAWING = false;
    static constexpr int MILLIS_PER_CIRCLE_ANIMATING = 700;

    // Amount of time (in millis) spent to animate a dot
    static constexpr int DEFAULT_DOT_ANIMATION_DURATION = 190;
    // Amount of time (in millis) spent to animate a path ends
    static constexpr int DEFAULT_PATH_END_ANIMATION_DURATION = 100;
    // This can be used to avoid updating the display for very small motions or noisy panels
    static constexpr float DEFAULT_DRAG_THRESHOLD = 0.0f;    
public:
    enum AspectRatio {
        // Width and height will be same. Minimum of width and height
        ASPECT_RATIO_SQUARE = 0,
        // Width will be fixed. The height will be the minimum of width and height
        ASPECT_RATIO_WIDTH_BIAS = 1,
        // Height will be fixed. The width will be the minimum of width and height
        ASPECT_RATIO_HEIGHT_BIAS = 2
    };
    enum PatternViewMode {
        /**
         * This state represents a correctly drawn pattern by the user. The color of the path and
         * the dots both would be changed to this color.
         * <p>
         * (NOTE - Consider showing this state in a friendly color)
         */
        CORRECT = 0,
        /**
         * Automatically draw the pattern for demo or tutorial purposes.
         */
        AUTO_DRAW = 1,
        /**
         * This state represents a wrongly drawn pattern by the user. The color of the path and
         * the dots both would be changed to this color.
         * <p>
         * (NOTE - Consider showing this state in an attention-seeking color)
         */
        WRONG = 2
    };
    class DotState {
    public:
        float mScale = 1.0f;
        float mTranslateY = 0.0f;
        float mAlpha = 1.0f;
        float mSize;
        float mLineEndX = FLT_MIN;//std::numeric_limits<float>::min();//Float.MIN_VALUE;
        float mLineEndY = FLT_MIN;//std::numeric_limits<float>::min();//Float.MIN_VALUE;
        ValueAnimator* mLineAnimator;
    };
    class Dot{
    private:
	friend PatternLockView;
	PatternLockView*mPLV;
	int mRow;
        int mColumn;
	Dot(PatternLockView*,int row, int column);
    public:
        int getId();
        int getRow();
        int getColumn();
    };
    struct PatternLockViewListener {
        CallbackBase<void> onStarted;
        CallbackBase<void,const std::vector<Dot*>&> onProgress;
        CallbackBase<void,const std::vector<Dot*>&> onComplete;
        CallbackBase<void> onCleared;
    };
private:
    std::vector<std::vector<Dot*>>mDots;
    std::vector<std::vector<DotState*>> mDotStates;
    int mPatternSize;
    bool mDrawingProfilingStarted = false;
    int64_t mAnimatingPeriodStart;
    float mHitFactor = 0.6f;

    // Made static so that the static inner class can use it
    int  mDotCount;

    bool mAspectRatioEnabled;
    int  mAspectRatio;
    int  mNormalStateColor;
    int  mWrongStateColor;
    int  mCorrectStateColor;
    int  mPathWidth;
    int  mDotNormalSize;
    int  mDotSelectedSize;
    int  mDotAnimationDuration;
    int  mPathEndAnimationDuration;

    //Paint mDotPaint;
    //Paint mPathPaint;

    std::vector<PatternLockViewListener> mPatternListeners;
    // The pattern represented as a list of connected {@link Dot}
    std::vector<Dot*> mPattern;

    /**
     * Lookup table for the dots of the pattern we are currently drawing.
     * This will be the dots of the complete pattern unless we are animating,
     * in which case we use this to hold the dots we are drawing for the in
     * progress animation.
     */
    std::vector< std::vector<bool> > mPatternDrawLookup;

    float mInProgressX = -1;
    float mInProgressY = -1;

    int mPatternViewMode = CORRECT;
    bool mInputEnabled = true;
    bool mInStealthMode = false;
    bool mEnableHapticFeedback = true;
    bool mPatternInProgress = false;

    float mViewWidth;
    float mViewHeight;

    Rect mInvalidate;
    Rect mTempInvalidateRect;

    //Interpolator* mFastOutSlowInInterpolator;
    Interpolator* mLinearOutSlowInInterpolator;
private:
    void initView();
    int resolveMeasured(int measureSpec, int desired);
    void notifyPatternProgress();
    void notifyPatternStarted();
    void notifyPatternDetected();
    void notifyPatternCleared();
    void resetPattern();
    void notifyListenersStarted();
    void notifyListenersProgress(const std::vector<Dot*>& pattern);
    void notifyListenersComplete(const std::vector<Dot*>& pattern);
    void notifyListenersCleared();
    void clearPatternDrawLookup();
    Dot* detectAndAddHit(float x, float y);
    void addCellToPattern(Dot* newDot);
    void startDotSelectedAnimation(Dot* dot);
    void startLineEndAnimation(DotState state,float startX, float startY,float targetX,float targetY);
    void startSizeAnimation(float start, float end, long duration, Interpolator* interpolator,DotState state,Runnable endRunnable);
    Dot* checkForNewHit(float x, float y);
    int getRowHit(float y);
    int getColumnHit(float x);
    void handleActionMove(MotionEvent& event);
    void sendAccessEvent(int resId);
    void handleActionUp(MotionEvent& event);
    void cancelLineAnimations();
    void handleActionDown(MotionEvent& event);
    float getCenterXForColumn(int column);
    float getCenterYForRow(int row);
    float calculateLastSegmentAlpha(float x, float y, float lastX,float lastY);
    int getCurrentColor(bool partOfPattern);
    void drawCircle(Canvas& canvas, float centerX, float centerY,float size, bool partOfPattern, float alpha);
    Dot* of(int row, int column);
    Dot* of(int id);
    void checkRange(int row, int column);    
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onDraw(Canvas& canvas)override;
    void onSizeChanged(int width, int height, int oldWidth, int oldHeight)override; 
public:
    PatternLockView(int,int);
    PatternLockView(Context* context,const AttributeSet& attrs);
    ~PatternLockView();
    bool onHoverEvent(MotionEvent& event)override;
    bool onTouchEvent(MotionEvent& event)override;
    std::vector<Dot*> getPattern();
    int getPatternViewMode();
    bool isInStealthMode();
    bool isTactileFeedbackEnabled();
    bool isInputEnabled();
    int getDotCount();
    bool isAspectRatioEnabled();
    int getAspectRatio();
    int getNormalStateColor();
    int getWrongStateColor();
    int getCorrectStateColor();
    int getPathWidth();
    int getDotNormalSize();
    int getDotSelectedSize() ;
    int getPatternSize();
    int getDotAnimationDuration();
    int getPathEndAnimationDuration();
    void setPattern(int patternViewMode,const std::vector<Dot*>& pattern);
    void setViewMode(int patternViewMode);
    void setDotCount(int dotCount);
    void setAspectRatioEnabled(bool aspectRatioEnabled);
    void setAspectRatio(int aspectRatio);
    void setNormalStateColor(int normalStateColor);
    void setWrongStateColor(int wrongStateColor) ;
    void setCorrectStateColor(int correctStateColor);
    void setPathWidth(int pathWidth);
    void setDotNormalSize(int dotNormalSize);
    void setDotSelectedSize(int dotSelectedSize);
    void setDotAnimationDuration(int dotAnimationDuration);
    void setPathEndAnimationDuration(int pathEndAnimationDuration);
    void setInStealthMode(bool inStealthMode);
    void setTactileFeedbackEnabled(bool tactileFeedbackEnabled);
    void setInputEnabled(bool inputEnabled);
    void setEnableHapticFeedback(bool enableHapticFeedback);
    void addPatternLockListener(const PatternLockViewListener& patternListener);
    void removePatternLockListener(const PatternLockViewListener& patternListener);
    void clearPattern();    
};

}//endof namespace
#endif/*__PATTERN_LOCK_VIEW_H__*/
