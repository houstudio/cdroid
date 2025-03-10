#ifndef __VIEW_OUTLINE_PROVIDER_H__
#define __VIEW_OUTLINE_PROVIDER_H__
#include <functional>
namespace cdroid{
    class View;
    class Outline;
    using ViewOutlineProvider = std::function<void(View& view, Outline& outline)>;
    namespace OutlineProvider{
    extern const ViewOutlineProvider BACKGROUND;

    /**
     * Maintains the outline of the View to match its rectangular bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    extern const ViewOutlineProvider BOUNDS;
    /**
     * Maintains the outline of the View to match its rectangular padded bounds,
     * at <code>1.0f</code> alpha.
     *
     * This can be used to enable Views that are opaque but lacking a background cast a shadow.
     */
    extern const ViewOutlineProvider PADDED_BOUNDS;
    }
}/*endof namespace*/
#endif
