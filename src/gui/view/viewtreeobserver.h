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
    void addOnWindowFocusChangeListener(OnWindowFocusChangeListener listener);
    void removeOnWindowFocusChangeListener(OnWindowFocusChangeListener victim);
    void addOnGlobalFocusChangeListener(OnGlobalFocusChangeListener listener);
    void removeOnGlobalFocusChangeListener(OnGlobalFocusChangeListener victim);
    void addOnGlobalLayoutListener(OnGlobalLayoutListener listener);
    void removeGlobalOnLayoutListener(OnGlobalLayoutListener victim);
    void removeOnGlobalLayoutListener(OnGlobalLayoutListener victim);
    void addOnPreDrawListener(OnPreDrawListener listener);
    void removeOnPreDrawListener(OnPreDrawListener victim);
    void addOnWindowShownListener(OnWindowShownListener listener);
    void removeOnWindowShownListener(OnWindowShownListener victim);
    void addOnDrawListener(OnDrawListener listener);
    void removeOnDrawListener(OnDrawListener victim);
    void addOnScrollChangedListener(OnScrollChangedListener listener);
    void removeOnScrollChangedListener(OnScrollChangedListener victim);
    void addOnTouchModeChangeListener(OnTouchModeChangeListener listener);
    void removeOnTouchModeChangeListener(OnTouchModeChangeListener victim);
    void addOnComputeInternalInsetsListener(OnComputeInternalInsetsListener listener);
    void removeOnComputeInternalInsetsListener(OnComputeInternalInsetsListener victim);
    void addOnEnterAnimationCompleteListener(OnEnterAnimationCompleteListener listener);
    void removeOnEnterAnimationCompleteListener(OnEnterAnimationCompleteListener listener);
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
