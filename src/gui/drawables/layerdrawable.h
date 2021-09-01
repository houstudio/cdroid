#ifndef __LAYER_DRAWABLE_H__
#define __LAYER_DRAWABLE_H__
#include <drawables/drawable.h>
namespace cdroid{


class LayerDrawable:public Drawable,public Drawable::Callback {
protected:
    class ChildDrawable{
    public:
        Drawable*mDrawable;
        int mInsetL, mInsetT, mInsetR, mInsetB,mInsetE,mInsetS;
        int mWidth,mHeight;
        int mGravity;
        int mDensity;
        int mId;
        int mPaddingMode;
        std::vector<int>mThemeAttrs;
        ChildDrawable(int density);
        ChildDrawable(ChildDrawable* orig,LayerDrawable*owner);
        ~ChildDrawable();
        void setDensity(int targetDensity);
        void applyDensityScaling(int sourceDensity, int targetDensity);
    };
    class LayerState:public std::enable_shared_from_this<LayerState>,public ConstantState{ 
    public:
        int mDensity;
        int mPaddingTop,mPaddingBottom;
        int mPaddingLeft,mPaddingRight;
        int mPaddingStart,mPaddingEnd;
        int mOpacityOverride;
        int mChangingConfigurations,mChildrenChangingConfigurations;
        bool mCheckedOpacity;
        int  mOpacity;
        std::vector<int>mThemeAttrs;

        bool mCheckedStateful;
        bool mIsStateful;
        bool mAutoMirrored;
        int mPaddingMode;
        std::vector< ChildDrawable*>mChildren;
        LayerState();
        LayerState(const LayerState*state,LayerDrawable*owner);
        ~LayerState();
        Drawable*newDrawable()override;
        int getChangingConfigurations()const override;
        bool isStateful()const;
        bool hasFocusStateSpecified()const;
        bool canConstantState();
        void invalidateCache();
        void setDensity(int targetDensity);
        virtual void onDensityChanged(int sourceDensity, int targetDensity);
        void applyDensityScaling(int sourceDensity, int targetDensity);
    };
    bool mSuspendChildInvalidation;
    bool mChildRequestedInvalidation;
    void updateLayerBounds(const Rect& bounds);
    void updateLayerBoundsInternal(const Rect& bounds);
    void suspendChildInvalidation();
    void resumeChildInvalidation();
    static int resolveGravity(int gravity, int width, int height,
            int intrinsicWidth, int intrinsicHeight);
    std::shared_ptr<LayerState> mLayerState;
private:
    bool mMutated;
    std::vector<int>mPaddingL;
    std::vector<int>mPaddingT;
    std::vector<int>mPaddingR;
    std::vector<int>mPaddingB;
    bool refreshChildPadding(int i, ChildDrawable* r);
    void setLayerInsetInternal(int index, int l, int t, int r, int b, int s, int e);
    void computeNestedPadding(Rect& padding);
    void computeStackedPadding(Rect& padding);
    ChildDrawable* createLayer(Drawable* dr);
protected:
    virtual LayerState* createConstantState(LayerState* state);
    void onBoundsChange(const Rect& bounds)override;
    bool onLevelChange(int level)override;
    bool onStateChange(const std::vector<int>& state)override;

    void ensurePadding();
    void refreshPadding();

    int addLayer(ChildDrawable* layer);
    LayerDrawable(std::shared_ptr<LayerState>state);
    ChildDrawable* addLayer(Drawable* dr,const std::vector<int>&themeAttrs,int id,int left,int top,int right,int bottom);
public:
    enum PaddingMode{
        PADDING_MODE_NEST=0,
        PADDING_MODE_STACK=1
    };
    LayerDrawable();
    LayerDrawable(const std::vector<Drawable*>&drawables);

    void setLayerSize(int index, int w, int h);
    int getLayerWidth(int index)const;
    void setLayerWidth(int index, int w);
    int getLayerHeight(int index)const;
    void setLayerHeight(int idx,int h);
    int getLayerGravity(int index)const;
    void setLayerGravity(int index, int gravity);
    void setLayerInset(int index, int l, int t, int r, int b);
    void setLayerInsetRelative(int index, int s, int t, int e, int b);
    int getLayerInsetLeft(int index)const;
    void setLayerInsetLeft(int index, int l);
    int getLayerInsetRight(int index)const;
    void setLayerInsetRight(int index, int r);
    int getLayerInsetTop(int index)const;
    void setLayerInsetTop(int index, int t);
    int getLayerInsetBottom(int index)const;
    void setLayerInsetBottom(int index, int b);
    int getLayerInsetStart(int index)const;
    void setLayerInsetStart(int index, int s);
    int getLayerInsetEnd(int index)const;
    void setLayerInsetEnd(int index, int e);
    int getId(int index);
    void setId(int index, int id);
    int getNumberOfLayers()const;

    int getIntrinsicWidth() const override;
    int getIntrinsicHeight()const override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;

    int addLayer(Drawable* dr); 
    Drawable*findDrawableByLayerId(int id);
    virtual bool setDrawableByLayerId(int id, Drawable* drawable);
    int findIndexByLayerId(int id)const;
    Drawable* getDrawable(int index);
    void setDrawable(int index, Drawable* drawable);
    bool getPadding(Rect& padding)override;
    void setPadding(int left, int top, int right, int bottom);
    virtual void setPaddingMode(int mode);
    int getPaddingMode()const;

    Drawable*mutate()override;
    void clearMutated()override;
    bool onLayoutDirectionChanged(int layoutDirection)override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void invalidateDrawable(Drawable& who);
    void scheduleDrawable(Drawable& who,Runnable what, long when);
    void unscheduleDrawable(Drawable& who,Runnable what);
    void draw(Canvas&canvas)override;
    static Drawable*inflate(Context*,const AttributeSet&atts);
};
}
#endif
