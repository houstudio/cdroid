#ifndef __VIEW_OVERLAY_H__
#define __VIEW_OVERLAY_H__
#include <view/view.h>
#include <view/viewgroup.h>

namespace cdroid{
class ViewOverlay{
public:
    class OverlayViewGroup:public ViewGroup{
    protected:
        View* mHostView;
        std::vector<Drawable*>mDrawables;
    protected:
        bool verifyDrawable(Drawable* who)const override;
        void dispatchDraw(Canvas& canvas);
        void onLayout(bool changed, int l, int t, int w, int h)override;
        void invalidateViewProperty(bool invalidateParent, bool forceRedraw)override;
        void invalidateParentCaches()override;
        void invalidateParentIfNeeded()override;
    public:
        OverlayViewGroup(Context*context,View* hostView);
        void add(Drawable* drawable);
        void remove(Drawable* drawable);
        void add(View* child);
        void remove(View* view);
        void clear();
        bool isEmpty()const;
        void invalidateDrawable(Drawable& drawable)override;
        void invalidate(const Rect& dirty)override;
        void invalidate(int l, int t, int w, int h)override;
        void invalidate(bool invalidateCache)override;
        void onDescendantInvalidated(View* child,View* target)override;
        ViewGroup* invalidateChildInParent(int* location, Rect& dirty)override;
    };
protected:
    OverlayViewGroup* mOverlayViewGroup;
public:
    ViewOverlay(Context* context, View* hostView);
    ~ViewOverlay();
    ViewGroup* getOverlayView()const;
    void add(Drawable* drawable);
    void remove(Drawable* drawable);
    void clear();
    bool isEmpty()const;
};

class ViewGroupOverlay:public ViewOverlay{
public:
    ViewGroupOverlay(Context* context, View* hostView);
    void add(View*);
    void remove(View*);
};

}
#endif
