#ifndef __ORIENTED_BOUDING_BOX_H__
#define __ORIENTED_BOUDING_BOX_H__
#include <core/path.h>
namespace cdroid{
class OrientedBoundingBox {
public:
    float squareness;
    float width;
    float height;
    float orientation;
    float centerX;
    float centerY;
public:
    OrientedBoundingBox(float angle, float cx, float cy, float w, float h);

    /**
     * Currently used for debugging purpose only.
     *
     * @hide
     */
    cdroid::Path toPath();
};
}/*endof namespace*/
#endif/*__ORIENTED_BOUDING_BOX_H__*/
