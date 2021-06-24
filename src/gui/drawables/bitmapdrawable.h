#ifndef __BITMAP_DRAWABLE_H__
#define __BITMAP_DRAWABLE_H__
#include <drawables/drawable.h>
#include <cairomm/surface.h>
#include <cairomm/refptr.h>
using namespace Cairo;
namespace cdroid{

class BitmapDrawable:public Drawable{
private:
    class BitmapState:public std::enable_shared_from_this<BitmapState>,public ConstantState{
    public:
        float mBaseAlpha;
        int mAlpha;
        int mGravity;
        bool mAutoMirrored;
		int mChangingConfigurations;
		std::vector<int>mThemeAttrs;
        ColorStateList* mTint;
        int mTintMode;
		int mSrcDensityOverride;
		int mTargetDensity;
        RefPtr<ImageSurface>mBitmap;
        BitmapState();
        BitmapState(RefPtr<ImageSurface>bitmap);
        BitmapState(const BitmapState&bitmapState);
        Drawable* newDrawable()override;
		int getChangingConfigurations()const override;
    };
	int mBitmapWidth;
	int mBitmapHeight;
    std::shared_ptr<BitmapState>mBitmapState;
    PorterDuffColorFilter*mTintFilter;
	void computeBitmapSize();
	void updateDstRectAndInsetsIfDirty();
    BitmapDrawable(std::shared_ptr<BitmapState>state);
protected:
	bool mMutated;
	RECT mDstRect;
	RECT mOpticalInsets;
	bool mDstRectAndInsetsDirty;
	void onBoundsChange(const RECT&r)override;
    bool onStateChange(const std::vector<int>&)override;
public:
    BitmapDrawable(RefPtr<ImageSurface>img);
    BitmapDrawable(std::istream&is);
    BitmapDrawable(const std::string&resname);
    RefPtr<ImageSurface> getBitmap()const;
    void setBitmap(RefPtr<ImageSurface>bmp);
    void setAlpha(int a)override;
    int getAlpha()const override;
    int getGravity()const;
    void setGravity(int gravity);
    int getIntrinsicWidth()const override;
    int getIntrinsicHeight()const override;
    int getOpacity()override;
    void setTintList(ColorStateList*lst)override;
    void setTintMode(int mode)override;
    Drawable*mutate()override;
    void clearMutated()override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void draw(Canvas&canvas)override;
	static Drawable*inflate(Context*,const AttributeSet&atts);
};

}
#endif
