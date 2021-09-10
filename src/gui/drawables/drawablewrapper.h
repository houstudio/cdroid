#ifndef __DRAWABLE_WRAPPER_H__
#define __DRAWABLE_WRAPPER_H__
#include <drawables/drawable.h>
namespace cdroid{

class DrawableWrapper:public Drawable,Drawable::Callback{
protected:
    class DrawableWrapperState:public std::enable_shared_from_this<DrawableWrapperState>,public ConstantState{
    public:
        int mDensity;
        int mSrcDensityOverride;
        int mChangingConfigurations;
        std::vector<int>mThemeAttrs;
        std::shared_ptr<ConstantState> mDrawableState;

        DrawableWrapperState();
        DrawableWrapperState(const DrawableWrapperState& orig);
        void setDensity(int targetDensity);
        virtual void onDensityChanged(int sourceDensity, int targetDensity);
        Drawable* newDrawable()override;
        int getChangingConfigurations()const override;
        virtual bool canConstantState()const;
    };
private:
    bool mMutated;
    Drawable*mDrawable;
    std::shared_ptr<DrawableWrapperState>mState;
    void updateLocalState();
protected:
    virtual std::shared_ptr<DrawableWrapperState> mutateConstantState();
    DrawableWrapper(std::shared_ptr<DrawableWrapperState>state);
    bool onStateChange(const std::vector<int>& state)override;
    bool onLevelChange(int level)override;
    void onBoundsChange(const Rect& bounds)override;
public:
    DrawableWrapper(Drawable*d=nullptr);
    ~DrawableWrapper();
    int getIntrinsicWidth ()const override;
    int getIntrinsicHeight()const override;
    int getChangingConfigurations()const override;
    void setDrawable(Drawable* dr);
    Drawable* getDrawable();
    Drawable*mutate()override;
    void clearMutated()override;
    std::shared_ptr<ConstantState>getConstantState()override;
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable&who,Runnable what, long when)override;
    void unscheduleDrawable(Drawable& who,Runnable what)override;
    void draw(Canvas&canvas)override;
    bool getPadding(Rect& padding)override;
    Insets getOpticalInsets()override;
    bool setVisible(bool visible, bool restart)override;
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
};

}
#endif
