/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.view.GhostView (@hide).
 *
 * Draws another View in an overlay without changing the parent. android uses RenderNode
 * (display list) for onDraw; CDROID's cairo draws the ghosted view directly onto the canvas
 * with the stored matrix applied. The overlay z-ordering binary-search is simplified to append.
 *********************************************************************************/
#ifndef __CDROID_VIEW_GHOSTVIEW_H__
#define __CDROID_VIEW_GHOSTVIEW_H__

#include <cairomm/matrix.h>
#include <view/view.h>

namespace cdroid{

class ViewGroup;
class Canvas;

/**
 * A View that draws another View in an overlay. @hide. Ported from android-36 android.view.GhostView.
 */
class GhostView: public View{
public:
    /** Adds a GhostView for {@code view} to {@code viewGroup}'s overlay, using the given matrix. */
    static GhostView* addGhost(View* view, ViewGroup* viewGroup, const Cairo::Matrix* matrix);
    /** Adds a GhostView with a matrix calculated from the view's position. */
    static GhostView* addGhost(View* view, ViewGroup* viewGroup);
    /** Removes one reference; when references reach 0 the GhostView is removed from the overlay. */
    static void removeGhost(View* view);
    /** Returns the GhostView currently shadowing {@code view}, or null. */
    static GhostView* getGhost(View* view);
    /** Calculates the matrix to transform from view's parent coords to host's local coords. */
    static void calculateMatrix(View* view, ViewGroup* host, Cairo::Matrix& matrix);

    void setMatrix(const Cairo::Matrix* matrix);
    void setVisibility(int visibility) override;

protected:
    void onDraw(Canvas& canvas) override;
    void onDetachedFromWindow() override;

private:
    explicit GhostView(View* view);

    View* mView;
    int mReferences = 0;
    bool mBeingMoved = false;
    Cairo::Matrix mMatrix; // identity by default
};

} // namespace cdroid
#endif // __CDROID_VIEW_GHOSTVIEW_H__
