#ifndef __BITMAP_DRAWABLE_H__
#define __BITMAP_DRAWABLE_H__
#include <drawables/drawable.h>
#include <cairomm/surface.h>
#include <cairomm/refptr.h>
namespace cdroid{

enum TileMode{
    DISABLED=-1,
    CLAMP =0,
    REPEAT=1,
    MIRROR=2
};
class BitmapDrawable:public Drawable{
private:
    class BitmapState:public std::enable_shared_from_this<BitmapState>,public ConstantState{
    public:
        float mBaseAlpha;
        int mAlpha;
        int mGravity;
        int mTransparency;
        bool mAutoMirrored;
        bool mAntiAlias;
        int mChangingConfigurations;
        std::vector<int>mThemeAttrs;
        const ColorStateList* mTint;
        int mTintMode;
        int mTileModeX;
        int mTileModeY;
        int mSrcDensityOverride;
        int mTargetDensity;
        Cairo::RefPtr<Cairo::ImageSurface>mBitmap;
        BitmapState();
        BitmapState(Cairo::RefPtr<Cairo::ImageSurface>bitmap);
        BitmapState(const BitmapState&bitmapState);
        ~BitmapState()override;
        BitmapDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    int mBitmapWidth;
    int mBitmapHeight;
    Insets mOpticalInsets;
    std::shared_ptr<BitmapState>mBitmapState;
    PorterDuffColorFilter*mTintFilter;
    bool needMirroring();
    void computeBitmapSize();
    void updateDstRectAndInsetsIfDirty();
    BitmapDrawable(std::shared_ptr<BitmapState>state);
protected:
    bool mMutated;
    Rect mDstRect;
    bool mDstRectAndInsetsDirty;
    void onBoundsChange(const Rect&r)override;
    bool onStateChange(const std::vector<int>&)override;
public:
    BitmapDrawable(Cairo::RefPtr<Cairo::ImageSurface>img);
    BitmapDrawable(Context*ctx,const std::string&resname);
    ~BitmapDrawable();
    static int computeTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp);
    Cairo::RefPtr<Cairo::ImageSurface> getBitmap()const;
    void setBitmap(Cairo::RefPtr<Cairo::ImageSurface>bmp);
    void setAlpha(int a)override;
    int getAlpha()const override;
    int getGravity()const;
    void setGravity(int gravity);
    void setAntiAlias(bool aa);
    bool hasAntiAlias()const;
    void setDither(bool)override;
    int getIntrinsicWidth()const override;
    int getIntrinsicHeight()const override;
    int getOpacity()override;
    int getTileModeX()const;
    int getTileModeY()const;
    void setTileModeX(int);
    void setTileModeY(int);
    void setTileModeXY(int,int);
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()override;
    void setTintList(const ColorStateList*lst)override;
    void setTintMode(int mode)override;
    int getTintMode()const;
    BitmapDrawable*mutate()override;
    void clearMutated()override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
    Insets getOpticalInsets()override;
    static Drawable*inflate(Context*,const AttributeSet&atts);
};

}
#endif
