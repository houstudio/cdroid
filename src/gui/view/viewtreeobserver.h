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
#ifndef __VIEW_TREE_OBSERVER_H__
#define __VIEW_TREE_OBSERVER_H__
#include <cairomm/region.h>
#include <core/callbackbase.h>
#include <core/rect.h>
#include <vector>
namespace cdroid{
class View;
class Context;
class ViewTreeObserver{
public:
    class OnWindowAttachListener:public EventSet{
    public:
       CallbackBase <void> onWindowAttached;
       CallbackBase <void> onWindowDetached;
    };
    typedef CallbackBase <void,bool> OnWindowFocusChangeListener;
    typedef CallbackBase <void,View*, View*> OnGlobalFocusChangeListener;
    typedef CallbackBase <void> OnGlobalLayoutListener;
    typedef CallbackBase <bool> OnPreDrawListener;
    typedef CallbackBase <void> OnDrawListener;
    typedef CallbackBase <void,bool> OnTouchModeChangeListener;
    typedef CallbackBase <void> OnScrollChangedListener;
    typedef CallbackBase <void> OnWindowShownListener;

    class InternalInsetsInfo{
    public:
        static constexpr int TOUCHABLE_INSETS_FRAME = 0;
        static constexpr int TOUCHABLE_INSETS_CONTENT = 1;
        static constexpr int TOUCHABLE_INSETS_VISIBLE = 2;
        static constexpr int TOUCHABLE_INSETS_REGION = 3;
    public:
        Rect contentInsets;
        Rect visibleInsets;
        Cairo::RefPtr<Cairo::Region> touchableRegion;
        int mTouchableInsets;
    public:
        void setTouchableInsets(int val);
        void reset();
        bool isEmpty()const;
        int hashCode()const;
        bool equals(Object*)const;
        void set(const InternalInsetsInfo& other);
    };
    typedef CallbackBase <void,InternalInsetsInfo&> OnComputeInternalInsetsListener;
    typedef CallbackBase <void> OnEnterAnimationCompleteListener;
private:
    std::vector <OnWindowFocusChangeListener> mOnWindowFocusListeners;
    std::vector <OnWindowAttachListener> mOnWindowAttachListeners;
    std::vector <OnGlobalFocusChangeListener> mOnGlobalFocusListeners;
    std::vector <OnTouchModeChangeListener> mOnTouchModeChangeListeners;
    std::vector <OnEnterAnimationCompleteListener> mOnEnterAnimationCompleteListeners;

    std::vector <OnGlobalLayoutListener> mOnGlobalLayoutListeners;
    std::vector <OnComputeInternalInsetsListener> mOnComputeInternalInsetsListeners;
    std::vector <OnScrollChangedListener> mOnScrollChangedListeners;
    std::vector <OnPreDrawListener> mOnPreDrawListeners;
    std::vector <OnWindowShownListener> mOnWindowShownListeners;
    std::vector <OnDrawListener> mOnDrawListeners;

    bool mInDispatchOnDraw;
    static bool sIllegalOnDrawModificationIsFatal;
    bool mWindowShown;
    bool mAlive;
public:
    ViewTreeObserver(cdroid::Context* context);
    void merge(ViewTreeObserver& observer);
    void addOnWindowAttachListener(const OnWindowAttachListener& victim);
    void removeOnWindowAttachListener(const OnWindowAttachListener& victim);
    void addOnWindowFocusChangeListener(const OnWindowFocusChangeListener& listener);
    void removeOnWindowFocusChangeListener(const OnWindowFocusChangeListener& victim);
    void addOnGlobalFocusChangeListener(const OnGlobalFocusChangeListener& listener);
    void removeOnGlobalFocusChangeListener(const OnGlobalFocusChangeListener& victim);
    void addOnGlobalLayoutListener(const OnGlobalLayoutListener& listener);
    void removeGlobalOnLayoutListener(const OnGlobalLayoutListener& victim);
    void removeOnGlobalLayoutListener(const OnGlobalLayoutListener& victim);
    void addOnPreDrawListener(const OnPreDrawListener& listener);
    void removeOnPreDrawListener(const OnPreDrawListener& victim);
    void addOnWindowShownListener(const OnWindowShownListener& listener);
    void removeOnWindowShownListener(const OnWindowShownListener& victim);
    void addOnDrawListener(const OnDrawListener& listener);
    void removeOnDrawListener(const OnDrawListener& victim);
    void addOnScrollChangedListener(const OnScrollChangedListener& listener);
    void removeOnScrollChangedListener(const OnScrollChangedListener& victim);
    void addOnTouchModeChangeListener(const OnTouchModeChangeListener& listener);
    void removeOnTouchModeChangeListener(const OnTouchModeChangeListener& victim);
    void addOnComputeInternalInsetsListener(const OnComputeInternalInsetsListener& listener);
    void removeOnComputeInternalInsetsListener(const OnComputeInternalInsetsListener& victim);
    void addOnEnterAnimationCompleteListener(const OnEnterAnimationCompleteListener& listener);
    void removeOnEnterAnimationCompleteListener(const OnEnterAnimationCompleteListener& listener);
    void checkIsAlive()const;
    bool isAlive()const;
    void kill();
    void dispatchOnWindowAttachedChange(bool attached);
    void dispatchOnWindowFocusChange(bool hasFocus);
    void dispatchOnGlobalFocusChange(View* oldFocus, View* newFocus);
    void dispatchOnGlobalLayout();
    bool hasOnPreDrawListeners();
    bool dispatchOnPreDraw();
    void dispatchOnWindowShown();
    void dispatchOnDraw();
    void dispatchOnTouchModeChanged(bool inTouchMode);
    void dispatchOnScrollChanged();
    bool hasComputeInternalInsetsListeners();
    void dispatchOnComputeInternalInsets(InternalInsetsInfo& inoutInfo);
    void dispatchOnEnterAnimationComplete();
};
}//endof namespace
#endif //__VIEW_TREE_OBSERVER_H__
