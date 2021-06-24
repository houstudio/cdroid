#ifndef __ABS_SEEK_BAR_H__
#define __ABS_SEEK_BAR_H__
#include <widget/progressbar.h>

namespace cdroid{

class AbsSeekBar:public ProgressBar{
private:
    bool mSplitTrack;
    bool mHasThumbTint;
    bool mHasThumbTintMode;
    float mDisabledAlpha;
    float mTouchDownX;
    int mScaledTouchSlop;
    bool mIsDragging;
    void initSeekBar();
    float getScale()const;
    void startDrag(MotionEvent& event);
    void applyThumbTint();
    void updateThumbAndTrackPos(int w, int h);
    void setThumbPos(int w, Drawable* thumb, float scale, int offset);
protected:
    int mKeyProgressIncrement;
    int mThumbOffset;
    bool mIsUserSeekable;
    float mTouchProgressOffset;
    Drawable*mThumb;
    Drawable*mTickMark;
    void trackTouchEvent(MotionEvent& event);
    void onStartTrackingTouch();
    void onStopTrackingTouch();
    void onSizeChanged(int w,int h,int oldw,int oldh)override;
    bool verifyDrawable(Drawable* who)const override;
    virtual void drawThumb(Canvas&canvas);
    virtual void drawTrack(Canvas&canvas);
    virtual void drawTickMarks(Canvas&canvas);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void onProgressRefresh(float scale, bool fromUser, int progress);
    void onVisualProgressChanged(int id, float scale)override;
    void drawableStateChanged()override;
public:
    AbsSeekBar(Context*ctx,const AttributeSet&attrs);
    AbsSeekBar(int w,int h);
    void setKeyProgressIncrement(int increment);
    int getKeyProgressIncrement()const;
    void setThumbOffset(int thumbOffset);
    int getThumbOffset()const;
    Drawable*getThumb()const;
    void setThumb(Drawable*thumb);
    void setTickMark(Drawable* tickMark);
    Drawable* getTickMark()const;
    void setMin(int min);
    void setMax(int max);
    void onDraw(Canvas&canvas);
    bool onKeyDown(int keycode,KeyEvent&event)override;
    bool onTouchEvent(MotionEvent& event)override;
};

}//namespace


#endif
