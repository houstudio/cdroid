#ifndef __GRAVITY_H__
#define __GRAVITY_H__
#include <rect.h>
#include <string>

namespace cdroid{

class Gravity{
public:
    enum{
        /** Constant indicating that no gravity has been set **/
        NO_GRAVITY = 0x0000,
    
        /** Raw bit indicating the gravity for an axis has been specified. */
        AXIS_SPECIFIED = 0x0001,

        /** Raw bit controlling how the left/top edge is placed. */
        AXIS_PULL_BEFORE = 0x0002,
        /** Raw bit controlling how the right/bottom edge is placed. */
        AXIS_PULL_AFTER = 0x0004,
        /** Raw bit controlling whether the right/bottom edge is clipped to its
        * container, based on the gravity direction being applied. */
        AXIS_CLIP = 0x0008,

        /** Bits defining the horizontal axis. */
        AXIS_X_SHIFT = 0,
        /** Bits defining the vertical axis. */
        AXIS_Y_SHIFT = 4,

        /** Push object to the top of its container, not changing its size. */
        TOP = (AXIS_PULL_BEFORE|AXIS_SPECIFIED)<<AXIS_Y_SHIFT,
        /** Push object to the bottom of its container, not changing its size. */
        BOTTOM = (AXIS_PULL_AFTER|AXIS_SPECIFIED)<<AXIS_Y_SHIFT,
        /** Push object to the left of its container, not changing its size. */
        LEFT = (AXIS_PULL_BEFORE|AXIS_SPECIFIED)<<AXIS_X_SHIFT,
        /** Push object to the right of its container, not changing its size. */
        RIGHT = (AXIS_PULL_AFTER|AXIS_SPECIFIED)<<AXIS_X_SHIFT,

        /** Place object in the vertical center of its container, not changing its
        *  size. */
        CENTER_VERTICAL = AXIS_SPECIFIED<<AXIS_Y_SHIFT,
        /** Grow the vertical size of the object if needed so it completely fills
        *  its container. */
        FILL_VERTICAL = TOP|BOTTOM,

        /** Place object in the horizontal center of its container, not changing its
        *  size. */
        CENTER_HORIZONTAL = AXIS_SPECIFIED<<AXIS_X_SHIFT,
        /** Grow the horizontal size of the object if needed so it completely fills
        *  its container. */
        FILL_HORIZONTAL = LEFT|RIGHT,

        /** Place the object in the center of its container in both the vertical
        *  and horizontal axis, not changing its size. */
        CENTER = CENTER_VERTICAL|CENTER_HORIZONTAL,

        /** Grow the horizontal and vertical size of the object if needed so it
        *  completely fills its container. */
        FILL = FILL_VERTICAL|FILL_HORIZONTAL,

        /** Flag to clip the edges of the object to its container along the
        *  vertical axis. */
        CLIP_VERTICAL = AXIS_CLIP<<AXIS_Y_SHIFT,
    
        /** Flag to clip the edges of the object to its container along the
        *  horizontal axis. */
        CLIP_HORIZONTAL = AXIS_CLIP<<AXIS_X_SHIFT,

        /** Raw bit controlling whether the layout direction is relative or not (START/END instead of
        * absolute LEFT/RIGHT).
        */
        RELATIVE_LAYOUT_DIRECTION = 0x00800000,

        /**
        * Binary mask to get the absolute horizontal gravity of a gravity.
        */
        HORIZONTAL_GRAVITY_MASK = (AXIS_SPECIFIED |AXIS_PULL_BEFORE | AXIS_PULL_AFTER) << AXIS_X_SHIFT,
        /**
        * Binary mask to get the vertical gravity of a gravity.
        */
        VERTICAL_GRAVITY_MASK = (AXIS_SPECIFIED |AXIS_PULL_BEFORE | AXIS_PULL_AFTER) << AXIS_Y_SHIFT,

        /** Special constant to enable clipping to an overall display along the
        *  vertical dimension.  This is not applied by default by
        *  {@link #apply(int, int, int, Rect, int, int, Rect)}; you must do so
        *  yourself by calling {@link #applyDisplay}.
        */
        DISPLAY_CLIP_VERTICAL = 0x10000000,
    
        /** Special constant to enable clipping to an overall display along the
        *  horizontal dimension.  This is not applied by default by
        *  {@link #apply(int, int, int, Rect, int, int, Rect)}; you must do so
        *  yourself by calling {@link #applyDisplay}.
        */
        DISPLAY_CLIP_HORIZONTAL = 0x01000000,
    
        /** Push object to x-axis position at the start of its container, not changing its size. */
        START = RELATIVE_LAYOUT_DIRECTION | LEFT,

        /** Push object to x-axis position at the end of its container, not changing its size. */
        END = RELATIVE_LAYOUT_DIRECTION | RIGHT,

        /**
        * Binary mask for the horizontal gravity and script specific direction bit.
        */
        RELATIVE_HORIZONTAL_GRAVITY_MASK = START | END
    };
public:
    static void apply(int gravity, int w, int h,const RECT& container,RECT& outRect);
    static void apply(int gravity, int w, int h,const RECT& container,RECT& outRect, int layoutDirection);
    static void apply(int gravity, int w, int h,const RECT& container,int xAdj, int yAdj, RECT& outRect);
    static void apply(int gravity, int w, int h,const RECT& container,int xAdj, int yAdj, RECT& outRect, int layoutDirection);
    static void applyDisplay(int gravity,const RECT& display,RECT& inoutObj);
    static void applyDisplay(int gravity,const RECT& display,RECT& inoutObj, int layoutDirection);
    static bool isVertical(int gravity);
    static bool isHorizontal(int gravity);
    static int getAbsoluteGravity(int gravity, int layoutDirection);
    static const std::string toString(int gravity);
};

class LayoutDirection{
public:
    enum{
        UNDEFINED=-1,
        LTR      =0,
        RTL      =1,
        INHERIT  =2,
        LOCAL    =3
    };
};
}
#endif
