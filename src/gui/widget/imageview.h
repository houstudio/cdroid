#ifndef __UI_IMAGE_VIEW_H__
#define __UI_IMAGE_VIEW_H__

#include <view/view.h>

namespace cdroid{

enum ScaleType{
    MATRIX       =0,
    FIT_XY       =1,
    FIT_START    =2,
    FIT_CENTER   =3,
    FIT_END      =4,
    CENTER       =5,
    CENTER_CROP  =6,
    CENTER_INSIDE=7
};
class ImageView : public View {
private:
    bool mColorMod;
    bool mHasColorFilter;
    bool mHasDrawableTint;
    bool mHasDrawableTintMode;
    bool mBaselineAlignBottom;
    int mBaseline;
    int mAlpha;
    int mViewAlphaScale;
    void initImageView();
    void resolveUri();
    int resolveAdjustedSize(int desiredSize, int maxSize,int measureSpec);
    void applyImageTint();
    void applyColorMod();
    bool isFilledByImage()const;
protected:
    std::string mResource;
    int mScaleType;
    int mLevel;
    int mMaxWidth;
    int mMaxHeight;
    int mDrawableWidth;
    int mDrawableHeight;
    int mRadii[4];
    bool mAdjustViewBounds;
    bool mMergeState;
    bool mHaveFrame;
    bool mCropToPadding;
    std::vector<int>mState;
    Drawable*mDrawable;
    ColorFilter*mColorFilter;
    const ColorStateList*mDrawableTintList;
    int mDrawableTintMode;
    BitmapDrawable*mRecycleableBitmapDrawable;
    Matrix mMatrix;
    Matrix mDrawMatrix;
    void updateDrawable(Drawable* d);
    void resizeFromDrawable();
    void configureBounds();
    bool setFrame(int l, int t, int w, int h)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void drawableStateChanged()override;
    virtual void onDraw(Canvas& canvas) override;
public:
    explicit ImageView(int w, int h);
    ImageView(Context*ctx,const AttributeSet&attrs);
    virtual ~ImageView();
    bool verifyDrawable(Drawable* dr)const override;
    void jumpDrawablesToCurrentState();
    void invalidateDrawable(Drawable& dr)override;
    std::vector<int> onCreateDrawableState()override;
    int getScaleType()const;
    void setScaleType(int st);
    void setImageMatrix(const Cairo::Matrix& matrix);
    Cairo::Matrix getImageMatrix()const;

    bool getCropToPadding()const;
    void setCropToPadding(bool cropToPadding);
    void setMaxWidth(int);
    void setMaxHeight(int);
    int getMaxWidth()const;
    int getMaxHeight()const;
    Drawable*getDrawable();
    void setBaseline(int baseline);
    int getBaseline()override;
    void setBaselineAlignBottom(bool aligned);
    bool getBaselineAlignBottom()const;
    bool getAdjustViewBounds()const;
    void setAdjustViewBounds(bool adjustViewBounds);
    /*resid can be assets's resource or local filepath*/
    void setImageResource(const std::string&resid);
    void setImageDrawable(Drawable* drawable);
    void setImageBitmap(Cairo::RefPtr<Cairo::ImageSurface>bitmap);
    void setImageTintList(const ColorStateList*tint);
    const ColorStateList* getImageTintList();
    void setImageTintMode(int mode);
    int getImageTintMode()const;
    void setColorFilter(int color,int mode);
    void setColorFilter(int color);
    void setColorFilter(ColorFilter* cf);
    void clearColorFilter();
    ColorFilter* getColorFilter();
    void setImageAlpha(int alpha);
    int getImageAlpha()const;
    bool isOpaque()const override;
    void setImageLevel(int level);
    void setSelected(bool selected)override;
    void setCornerRadii(int radius);
    void setCornerRadii(int topLeftRadius,int topRightRadius,int bottomRightRadius,int bottomLeftRadius);
    void setImageState(const std::vector<int>&state, bool merge);
    void drawableHotspotChanged(float x, float y)override;
};

}
#endif
