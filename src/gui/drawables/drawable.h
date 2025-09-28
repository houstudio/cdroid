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
#ifndef __DRAWABLE_H__
#define __DRAWABLE_H__
#include <core/canvas.h>
#include <drawables/stateset.h>
#include <drawables/colorstatelist.h>
#include <drawables/colorfilters.h>
#include <core/xmlpullparser.h>
#include <core/attributeset.h>
#include <core/porterduff.h>
#include <core/context.h>
#include <view/gravity.h>
#include <core/insets.h>
#include <core/outline.h>
#include <vector>

namespace cdroid{
class ColorStateList;
class Context;

class Animatable {
public:
    /** Starts the drawable's animation.  */
    virtual void start()=0;
    /** Stops the drawable's animation.  */
    virtual void stop()=0;
    /* Indicates whether the animation is running.
     * @return True if the animation is running, false otherwise.  */
    virtual bool isRunning()=0;
};

class Animatable2:public Animatable{
public:
    class AnimationCallback:public EventSet{
    public:
        CallbackBase<void,Drawable&> onAnimationStart;
        CallbackBase<void,Drawable&> onAnimationEnd;
    };
    virtual void registerAnimationCallback(const AnimationCallback& callback)=0;
    virtual bool unregisterAnimationCallback(const AnimationCallback& callback)=0;
};

enum PixelFormat{
    UNKNOWN=0,
    TRANSLUCENT=1,
    TRANSPARENT=2,
    OPAQUE=3
};

class Drawable{
public:
    class Callback{
    public:
        virtual ~Callback()=default;
        virtual void invalidateDrawable(Drawable& who)=0;
        virtual void scheduleDrawable(Drawable& who,Runnable& what, int64_t when)=0;
        virtual void unscheduleDrawable(Drawable& who,Runnable& what)=0;
    };
    class ConstantState{
    public:
        std::string mResource;
    public:
        virtual Drawable* newDrawable()=0;
        virtual int getChangingConfigurations()const=0;
        virtual ~ConstantState();
    };
    enum{
        DEFAULT_TINT_MODE=PorterDuff::Mode::SRC_IN
    };
protected:
    bool mVisible;
    int mLevel;
    int mLayoutDirection;
    int mChangingConfigurations;
    int mSrcDensityOverride;
    Rect mBounds;
    ColorFilter*mColorFilter;
    Callback*mCallback;
    std::vector<int>mStateSet;
    PorterDuffColorFilter *updateTintFilter(PorterDuffColorFilter* tintFilter,const ColorStateList* tint,int tintMode);
    virtual bool onStateChange(const std::vector<int>&) { return false;}
    virtual bool onLevelChange(int level) { return false; }
    virtual bool onLayoutDirectionChanged(int layoutDirection){return false;}
    virtual void onBoundsChange(const Rect& bounds){}
public:
    Drawable();
    virtual ~Drawable();
    void setBounds(int x,int y,int w,int h);
    void setBounds(const Rect&r);
    const Rect&getBounds()const;
    void copyBounds(Rect&)const;
    virtual Rect getDirtyBounds()const;
    virtual Drawable*mutate();
    virtual void clearMutated();
    virtual void inflate(XmlPullParser&parser,const AttributeSet&);
    void inflateWithAttributes(XmlPullParser&parser,const AttributeSet&);
    static Drawable*createFromXmlInner(XmlPullParser&parser,const AttributeSet&);
    static Drawable*createFromXmlInnerForDensity(XmlPullParser&parser,const AttributeSet&,int);
    virtual void setColorFilter(ColorFilter*);
    virtual ColorFilter*getColorFilter();
    void setColorFilter(int color,PorterDuff::Mode mode);
    void clearColorFilter();
    void setTint(int color);
    void setSrcDensityOverride(int density);
    /*
     * To make memory manager simple,
     * The Drawable must deep copy from the tint,and own the new instance 
     * */
    virtual void setTintList(const ColorStateList* tint);
    virtual void setTintMode(int);
    bool setState(const std::vector<int>&state);
    const std::vector<int>& getState()const;
    bool setLevel(int level);
    int getLevel()const{return mLevel;}
    virtual int getOpacity();
    virtual void setHotspot(float x,float y);
    virtual void setHotspotBounds(int left,int top,int width,int height);
    virtual void getHotspotBounds(Rect&outRect)const;
    virtual bool isProjected()const;
    virtual bool getPadding(Rect&padding);
    virtual Insets getOpticalInsets();
    virtual void getOutline(Outline&);
    virtual bool isStateful()const;
    virtual bool hasFocusStateSpecified()const;
    virtual Drawable*getCurrent();
    virtual void setAlpha(int alpha){};
    virtual int getAlpha()const{return 0xFF;}
    virtual std::shared_ptr<ConstantState>getConstantState();
    virtual void setAutoMirrored(bool mirrored);
    virtual bool isAutoMirrored()const;
    virtual bool canApplyTheme(){return false;}
    virtual void jumpToCurrentState();

    int getLayoutDirection()const;
    bool setLayoutDirection(int);
    virtual void setDither(bool);
    virtual void setFilterBitmap(bool filter);
    virtual bool isFilterBitmap()const;

    virtual int getIntrinsicWidth();
    virtual int getIntrinsicHeight();
    virtual int getMinimumWidth();
    virtual int getMinimumHeight();

    virtual bool setVisible(bool visible, bool restart);
    virtual bool isVisible()const;
    virtual int getChangingConfigurations()const;
    virtual void setChangingConfigurations(int);
    void setCallback(Callback*cbk);
    Callback* getCallback()const;
    virtual Cairo::RefPtr<Cairo::Region>getTransparentRegion();

    void scheduleSelf(Runnable& what, int64_t when);
    virtual void unscheduleSelf(Runnable& what);
    virtual void invalidateSelf();

    virtual void draw(Canvas&ctx)=0;
    static int resolveOpacity(int op1,int op2);
    static int resolveDensity(int parentDensity);
    static PorterDuff::Mode parseTintMode(int value, PorterDuff::Mode defaultMode);
    static float scaleFromDensity(float pixels, int sourceDensity, int targetDensity);
    static int scaleFromDensity(int pixels, int sourceDensity, int targetDensity, bool isSize);
};

}
#endif
