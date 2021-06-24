#ifndef __NINEPATCH_DRAWABLE_H__
#define __NINEPATCH_DRAWABLE_H__
#include <drawables/drawable.h>
namespace cdroid{

class NinePatchDrawable:public Drawable{
private:
    class NinePatchState:public std::enable_shared_from_this<NinePatchState>,public ConstantState{
    public:
        float mBaseAlpha;// = 1.0f;
        bool mDither;// = DEFAULT_DITHER;
        bool mAutoMirrored;// = false;
        RECT mPadding;
        RECT mOpticalInsets;
        int mTintMode;
		int mChangingConfigurations;
        std::vector<NinePatchBlock> mHorz;
        std::vector<NinePatchBlock> mVert;
        ColorStateList*mTint;
        RefPtr<ImageSurface>mNinePatch;
        NinePatchState();
        NinePatchState(const NinePatchState&state);
        NinePatchState(RefPtr<ImageSurface>bitmap,const RECT*padding=nullptr);
        Drawable*newDrawable()override;
		int getChangingConfigurations()const override;
    };
    int mAlpha;
    int mBitmapWidth;
    int mBitmapHeight;
    int mTargetDensity;
    RECT mOpticalInsets;
    RECT mPadding;
    bool mMutated;
    bool needsMirroring();
    void computeBitmapSize();
    std::shared_ptr<NinePatchState>mNinePatchState;
    PorterDuffColorFilter*mTintFilter;
    NinePatchDrawable(std::shared_ptr<NinePatchState>state);
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
public:
    NinePatchDrawable();
    NinePatchDrawable(RefPtr<ImageSurface>bmp);
    ~NinePatchDrawable();
    void setTargetDensity(int density);
    void setAlpha(int alpha)override;
    bool getPadding(RECT& padding) override;
    int getAlpha()const override;
    void setTintList(ColorStateList* tint)override;
    void setTintMode(int mode)override;
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()override;
    int getIntrinsicWidth() const override;
    int getIntrinsicHeight()const override;
    Drawable*mutate()override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};
}
#endif
