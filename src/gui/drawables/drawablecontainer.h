#ifndef __DRAWABLE_CONTAINER_H__
#define __DRAWABLE_CONTAINER_H__
#include <drawables/drawable.h>
namespace cdroid{

class DrawableContainer:public Drawable,Drawable::Callback{
protected:
    class DrawableContainerState:public std::enable_shared_from_this<DrawableContainerState>,public ConstantState{
    private:
        Drawable*prepareDrawable(Drawable* child);
    public:
        DrawableContainer*mOwner;
        int mDensity;
        int mChangingConfigurations;
        int mChildrenChangingConfigurations;
        bool mVariablePadding;
        bool mCheckedPadding;
        Rect mConstantPadding;
        bool mConstantSize;
        bool mCheckedConstantSize;
        int mConstantWidth,mConstantHeight;
        int mConstantMinimumWidth,mConstantMinimumHeight;

        bool mCheckedOpacity;
        int mOpacity;

        bool mCheckedConstantState;
        bool mCanConstantState;
        bool mCheckedStateful;
        bool mStateful;

        bool mDither;
        bool mMutated;
        int mLayoutDirection;
        int mEnterFadeDuration,mExitFadeDuration;
        bool mAutoMirrored;
        int mTintMode;
        ColorFilter*mColorFilter;
        const ColorStateList*mTintList;
        std::vector<Drawable* >mDrawables;
        std::map<int,std::shared_ptr<ConstantState> >mDrawableFutures;
        DrawableContainerState(const DrawableContainerState*orig,DrawableContainer*own);
        ~DrawableContainerState();
        DrawableContainer*newDrawable()override{return nullptr;}//must be overrided by inherited
        int addChild(Drawable* dr);
        int getChildCount()const;
        Drawable*getChild(int index);
        void invalidateCache();
        virtual void mutate();
        virtual void clearMutated();
        void createAllFutures();
        virtual bool isStateful();
        bool canConstantState();
        int getChangingConfigurations()const override;
        bool setLayoutDirection(int layoutDirection, int currentIndex);
        bool getConstantPadding(Rect&rect);
        void setConstantSize(bool constant){mConstantSize=constant;}
        bool isConstantSize()const {return mConstantSize;}
        void setVariablePadding(bool variable){mVariablePadding=variable;}
        int getConstantWidth();
        int getConstantHeight();
        int getConstantMinimumWidth();
        int getConstantMinimumHeight();
        void computeConstantSize();
        int getEnterFadeDuration()const {return mEnterFadeDuration; }
        void setEnterFadeDuration(int duration) {mEnterFadeDuration = duration; }
        int getExitFadeDuration()const {return mExitFadeDuration; }
        void setExitFadeDuration(int duration) {mExitFadeDuration = duration;}
        int getOpacity();
    };
    Rect mHotspotBounds;
    class BlockInvalidateCallback*mBlockInvalidateCallback;
    void initializeDrawableForDisplay(Drawable*d);
    std::shared_ptr<DrawableContainerState>mDrawableContainerState;
protected:
    int mAlpha = 0xFF;
    bool mHasAlpha;
    int mCurIndex;
    int mLastIndex;
    bool mMutated;
    Runnable mAnimationRunnable;
    long mEnterAnimationEnd;
    long mExitAnimationEnd;

    Drawable* mCurrDrawable;
    Drawable* mLastDrawable;
    DrawableContainer(Context*ctx,const AttributeSet&atts);
    bool needsMirroring();
    void animate(bool schedule);
    virtual std::shared_ptr<DrawableContainerState> cloneConstantState();
    virtual void setConstantState(std::shared_ptr<DrawableContainerState>state);
    void onBoundsChange(const Rect&bounds)override;
    bool onStateChange(const std::vector<int>&state)override;
    bool onLevelChange(int level)override;
    bool onLayoutDirectionChanged(int layoutDirection);
public:
    DrawableContainer();
    ~DrawableContainer()override;
    int getCurrentIndex()const;
    virtual void setCurrentIndex(int index);
    virtual bool selectDrawable(int index);
    Drawable*getCurrent()override;
    int addChild(Drawable*);
    int getChildCount()const;
    Drawable*getChild(int index);
    bool getPadding(Rect&padding)override;
    Insets getOpticalInsets()override;
    int getChangingConfigurations()const override;
    void setAlpha(int)override;
    int getAlpha()const override;
    int getOpacity()override;
    int getIntrinsicWidth() const override;
    int getIntrinsicHeight()const override;
    int getMinimumWidth() const override;
    int getMinimumHeight()const override;
    void getHotspotBounds(Rect& outRect) override;
    void setHotspotBounds(int left, int top, int width, int height)override;
    void setColorFilter(ColorFilter*colorFilter)override;
    void setTintList(const ColorStateList*tint)override;
    void setTintMode(int)override;
    void setEnterFadeDuration(int ms);
    void setExitFadeDuration(int ms);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()override;
    void jumpToCurrentState()override;

    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable&who,Runnable what, long when)override;
    void unscheduleDrawable(Drawable& who,Runnable what)override;
    bool setVisible(bool visible, bool restart)override;
    std::shared_ptr<ConstantState>getConstantState()override;
    DrawableContainer*mutate()override;
    void clearMutated()override;
    void draw(Canvas&canvas)override;
};
}
#endif
