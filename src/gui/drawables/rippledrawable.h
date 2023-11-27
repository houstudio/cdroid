#ifndef __RIPPLE_DRAWABLE_H__
#define __RIPPLE_DRAWABLE_H__
#include <drawables/layerdrawable.h>
#include <drawables/rippleforeground.h>

namespace cdroid{

class RippleDrawable:public LayerDrawable{
public:
    static constexpr int RADIUS_AUTO = -1;
    static constexpr int MASK_UNKNOWN = -1;
    static constexpr int MASK_NONE = 0;
    static constexpr int MASK_CONTENT = 1;
    static constexpr int MASK_EXPLICIT = 2;
    static constexpr int MAX_RIPPLES = 10;
    static constexpr int MASK_LAYER_ID=0;
private:
    class RippleState:public LayerDrawable::LayerState{
    public:
        std::vector<int>mTouchThemeAttrs;
        int mMaxRadius;
        ColorStateList*mColor;
        RippleState(LayerState* orig, RippleDrawable* owner);
        ~RippleState();
        void onDensityChanged(int sourceDensity, int targetDensity)override;
        void applyDensityScaling(int sourceDensity, int targetDensity);
        Drawable*newDrawable()override;
        int getChangingConfigurations()const override;
    };

    Rect mHotspotBounds;
    Rect mDrawingBounds;
    Rect mDirtyBounds;
    std::shared_ptr<RippleState>mState;
    Drawable* mMask;
    RippleBackground* mBackground;
 
    bool mHasValidMask;
    bool mRippleActive;
    RippleForeground* mRipple;
    float mPendingX;
    float mPendingY;
    bool mHasPending;
    std::vector<RippleForeground*>mExitingRipples;
    int  mDensity;
    bool mOverrideBounds;
    bool mForceSoftware;
private:
    RippleDrawable(std::shared_ptr<RippleState> state);
    void cancelExitingRipples();
    void setRippleActive(bool active);
    void setBackgroundActive(bool hovered, bool focused, bool pressed);
    bool isBounded()const;
    void tryRippleEnter();
    void tryRippleExit();
    void clearHotspots();
    void onHotspotBoundsChanged();
    void updateLocalState();
    int  getMaskType();
    void drawContent(Canvas& canvas);
    void drawBackgroundAndRipples(Canvas& canvas);
    void drawMask(Canvas& canvas);
protected:
    bool onStateChange(const std::vector<int>&stateSet)override;
    void onBoundsChange(const Rect& bounds)override;
public:
    RippleDrawable(const ColorStateList* color,Drawable* content,Drawable* mask);
    void jumpToCurrentState()override;
    int  getOpacity()override;
    bool setVisible(bool visible, bool restart)override;
    bool isProjected();
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void setColor(const ColorStateList* color);
    void setRadius(int radius);
    int  getRadius()const;
    bool setDrawableByLayerId(int id, Drawable* drawable)override;
    void setPaddingMode(int mode);
    bool canApplyTheme()override;
    void getHotspotBounds(Rect&out)override;
    void setHotspot(float x,float y)override;
    void setHotspotBounds(int left,int top,int w,int h)override;
    void draw(Canvas& canvas);
    void invalidateSelf()override;
    void invalidateSelf(bool invalidateMask);
    void pruneRipples();
    Rect getDirtyBounds();
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}
#endif
