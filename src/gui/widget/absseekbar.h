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

    ColorStateList* mTickMarkTintList;
    int mTickMarkTintMode;
    bool mHasTickMarkTint = false;
    bool mHasTickMarkTintMode = false;

    void initSeekBar();
    float getScale()const;
    void startDrag(MotionEvent& event);
    void attemptClaimDrag();
    void applyThumbTint();
    void updateThumbAndTrackPos(int w, int h);
    void setThumbPos(int w, Drawable* thumb, float scale, int offset);
    void applyTickMarkTint();
protected:
    int mKeyProgressIncrement;
    int mThumbOffset;
    bool mIsUserSeekable;
    float mTouchProgressOffset;
    Drawable*mThumb;
    Drawable*mTickMark;
    void trackTouchEvent(MotionEvent& event);
    virtual void onStartTrackingTouch();
    virtual void onStopTrackingTouch();
    virtual void onKeyChange();
    void onSizeChanged(int w,int h,int oldw,int oldh)override;
    bool verifyDrawable(Drawable* who)const override;
    virtual void drawThumb(Canvas&canvas);
    virtual void drawTrack(Canvas&canvas);
    virtual void drawTickMarks(Canvas&canvas);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void onProgressRefresh(float scale, bool fromUser, int progress);
    void onVisualProgressChanged(int id, float scale)override;
    void drawableStateChanged()override;

    void onDraw(Canvas&canvas);
    bool onKeyDown(int keycode,KeyEvent&event)override;
    bool onTouchEvent(MotionEvent& event)override;
public:
    AbsSeekBar(int w,int h);
    AbsSeekBar(Context*ctx,const AttributeSet&attrs);
    void setKeyProgressIncrement(int increment);
    int getKeyProgressIncrement()const;
    void setThumbOffset(int thumbOffset);
    int getThumbOffset()const;
    Drawable*getThumb()const;
    void setThumb(Drawable*thumb);
    void setTickMark(Drawable* tickMark);
    Drawable* getTickMark()const;
    virtual void setMin(int min);
    virtual void setMax(int max);
    virtual bool canUserSetProgress()const;
    void onRtlPropertiesChanged(int layoutDirection)override;
    void drawableHotspotChanged(float x,float y)override;
};

}//namespace


#endif
