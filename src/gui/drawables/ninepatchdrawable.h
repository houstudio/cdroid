#ifndef __NINEPATCH_DRAWABLE_H__
#define __NINEPATCH_DRAWABLE_H__
#include <drawables/drawable.h>
namespace cdroid{

class NinePatchDrawable:public Drawable{
private:
    struct DIV{
        int pos;
        int len;
        bool stretchable;
    };
    class NinePatchState:public std::enable_shared_from_this<NinePatchState>,public ConstantState{
    private:
        int get_ninepatch(std::vector<DIV>&divHorz,std::vector<DIV>&divVert,bool padding);
    public:
        float mBaseAlpha;// = 1.0f;
        bool mDither;// = DEFAULT_DITHER;
        bool mAutoMirrored;// = false;
        Rect mPadding;
        Insets mOpticalInsets;
        int mTintMode;
		int mChangingConfigurations;
        std::vector<DIV> mHorz;
        std::vector<DIV> mVert;
        ColorStateList*mTint;
        RefPtr<ImageSurface>mNinePatch;
        NinePatchState();
        NinePatchState(const NinePatchState&state);
        NinePatchState(RefPtr<ImageSurface>bitmap,const Rect*padding=nullptr);
        Drawable*newDrawable()override;
        void draw(Canvas&canvas,const Rect&rect);
        int getChangingConfigurations()const override;
    };
    int mAlpha;
    int mBitmapWidth;
    int mBitmapHeight;
    int mTargetDensity;
    Insets mOpticalInsets;
    Rect mPadding;
    bool mMutated;
    bool needsMirroring();
    void computeBitmapSize();
    std::shared_ptr<NinePatchState>mNinePatchState;
    PorterDuffColorFilter*mTintFilter;
    NinePatchDrawable(std::shared_ptr<NinePatchState>state);
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
public:
    NinePatchDrawable(Context*,const std::string&resid);
    NinePatchDrawable(RefPtr<ImageSurface>bmp);
    ~NinePatchDrawable();
    void setTargetDensity(int density);
    Insets getOpticalInsets()override;
    void setAlpha(int alpha)override;
    bool getPadding(Rect& padding) override;
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
