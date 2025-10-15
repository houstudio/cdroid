/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __DRAWABLE_CONTAINER_H__
#define __DRAWABLE_CONTAINER_H__
#include <drawable/drawable.h>
#include <core/sparsearray.h>
namespace cdroid{

class DrawableContainer:public Drawable,Drawable::Callback{
protected:
    class DrawableContainerState:public std::enable_shared_from_this<DrawableContainerState>,public ConstantState{
    private:
        Drawable*prepareDrawable(Drawable* child);
    public:
        DrawableContainer*mOwner;
        Rect mConstantPadding;
        int mDensity;
        int mChangingConfigurations;
        int mChildrenChangingConfigurations;
        bool mVariablePadding;
        bool mCheckedPadding;
        bool mConstantSize;
        bool mCheckedConstantSize;
        bool mCheckedOpacity;
        int mConstantWidth,mConstantHeight;
        int mConstantMinimumWidth,mConstantMinimumHeight;

        int mOpacity;

        bool mCheckedConstantState;
        bool mCanConstantState;
        bool mCheckedStateful;
        bool mStateful;

        bool mDither;
        bool mMutated;
        bool mAutoMirrored;
        int mLayoutDirection;
        int mEnterFadeDuration,mExitFadeDuration;
        int mTintMode;
        ColorFilter*mColorFilter;
        const ColorStateList*mTintList;
        std::vector<Drawable*>mDrawables;
        SparseArray<std::shared_ptr<ConstantState>>mDrawableFutures;
    public:
        DrawableContainerState(const DrawableContainerState*orig,DrawableContainer*own);
        ~DrawableContainerState()override;
        DrawableContainer*newDrawable()override{return nullptr;}//must be overrided by inherited
        int addChild(Drawable* dr);
        int getChildCount()const;
        Drawable*getChild(int index);
        std::vector<Drawable*>getChildren();
        void invalidateCache();
        virtual void mutate();
        virtual void clearMutated();
        void setVariablePadding(bool variable);
        void createAllFutures();
        virtual bool isStateful();
        bool canConstantState();
        int getChangingConfigurations()const override;
        bool setLayoutDirection(int layoutDirection, int currentIndex);
        bool getConstantPadding(Rect&rect);
        void setConstantSize(bool constant);
        bool isConstantSize()const;
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
    int mAlpha;
    int mCurIndex;
    int mLastIndex;
    bool mHasAlpha;
    bool mMutated;
    Runnable mAnimationRunnable;
    int64_t mEnterAnimationEnd;
    int64_t mExitAnimationEnd;

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
    bool onLayoutDirectionChanged(int layoutDirection)override;
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
    void getOutline(Outline&)override;
    int getChangingConfigurations()const override;
    void setAlpha(int)override;
    int getAlpha()const override;
    void setDither(bool dither)override;
    int getOpacity()override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    int getMinimumWidth() override;
    int getMinimumHeight() override;
    void getHotspotBounds(Rect& outRect)const override;
    void setHotspotBounds(int left, int top, int width, int height)override;
    void setColorFilter(ColorFilter*colorFilter)override;
    void setTintList(const ColorStateList*tint)override;
    void setTintMode(int)override;
    void setEnterFadeDuration(int ms);
    void setExitFadeDuration(int ms);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void setAutoMirrored(bool mirrored)override;
    bool isAutoMirrored()const override;
    void jumpToCurrentState()override;

    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable&who,const Runnable& what, int64_t when)override;
    void unscheduleDrawable(Drawable& who,const Runnable& what)override;
    bool setVisible(bool visible, bool restart)override;
    std::shared_ptr<ConstantState>getConstantState()override;
    DrawableContainer*mutate()override;
    void clearMutated()override;
    void draw(Canvas&canvas)override;
};
}
#endif
