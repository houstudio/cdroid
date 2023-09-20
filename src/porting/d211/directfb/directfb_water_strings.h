#ifndef __DIRECTFB_WATER_STRINGS_H__
#define __DIRECTFB_WATER_STRINGS_H__
#include <directfb.h>
#include <directfb_water.h>


struct DFBWaterElementTypeName {
     WaterElementType type;
     const char *name;
};

#define DirectFBWaterElementTypeNames(Identifier) struct DFBWaterElementTypeName Identifier[] = { \
     { WET_POINT, "POINT" }, \
     { WET_SPAN, "SPAN" }, \
     { WET_LINE, "LINE" }, \
     { WET_LINE_STRIP, "LINE_STRIP" }, \
     { WET_LINE_LOOP, "LINE_LOOP" }, \
     { WET_TRIANGLE, "TRIANGLE" }, \
     { WET_TRIANGLE_FAN, "TRIANGLE_FAN" }, \
     { WET_TRIANGLE_STRIP, "TRIANGLE_STRIP" }, \
     { WET_RECTANGLE, "RECTANGLE" }, \
     { WET_RECTANGLE_STRIP, "RECTANGLE_STRIP" }, \
     { WET_TRAPEZOID, "TRAPEZOID" }, \
     { WET_TRAPEZOID_STRIP, "TRAPEZOID_STRIP" }, \
     { WET_QUADRANGLE, "QUADRANGLE" }, \
     { WET_QUADRANGLE_STRIP, "QUADRANGLE_STRIP" }, \
     { WET_POLYGON, "POLYGON" }, \
     { WET_CIRCLE, "CIRCLE" }, \
     { WET_ELLIPSE, "ELLIPSE" }, \
     { WET_ARC_CIRCLE, "ARC_CIRCLE" }, \
     { WET_ARC_ELLIPSE, "ARC_ELLIPSE" }, \
     { WET_QUAD_CURVE, "QUAD_CURVE" }, \
     { WET_QUAD_CURVE_STRIP, "QUAD_CURVE_STRIP" }, \
     { WET_CUBIC_CURVE, "CUBIC_CURVE" }, \
     { WET_CUBIC_CURVE_STRIP, "CUBIC_CURVE_STRIP" }, \
     { (WaterElementType) WET_UNKNOWN, "UNKNOWN" } \
};

#endif
