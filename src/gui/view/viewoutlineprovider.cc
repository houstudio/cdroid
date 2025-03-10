#include <view/view.h>
#include <core/outline.h>
#include <view/viewoutlineprovider.h>
namespace cdroid{
namespace OutlineProvider{
    const ViewOutlineProvider BACKGROUND =[](View& view, Outline& outline) {
        Drawable* background = view.getBackground();
        if (background != nullptr) {
            background->getOutline(outline);
        } else {
            outline.setRect(0, 0, view.getWidth(), view.getHeight());
            outline.setAlpha(0.0f);
        }
    };

    /**
     * Maintains the outline of the View to match its rectangular bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    const ViewOutlineProvider BOUNDS = [](View& view, Outline& outline) {
         outline.setRect(0, 0, view.getWidth(), view.getHeight());
    };

    /**
     * Maintains the outline of the View to match its rectangular padded bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    const ViewOutlineProvider PADDED_BOUNDS = [](View& view, Outline& outline) {
          outline.setRect(view.getPaddingLeft(),  view.getPaddingTop(),
                   view.getWidth() - view.getPaddingRight(),
                   view.getHeight() - view.getPaddingBottom());
    };
}
}/*endof namespace*/
