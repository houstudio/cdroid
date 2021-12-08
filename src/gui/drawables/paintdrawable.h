#ifndef __PAINT_DRAWABLE_H__
#define __PAINT_DRAWABLE_H__
#include <drawables/shapedrawable.h>

namespace cdroid{

class PaintDrawable:public ShapeDrawable{
public:
    PaintDrawable();
    PaintDrawable(int color);
    void setCornerRadius(float radius);
    void setCornerRadii(const std::vector<int>& radius);
};

}

#endif
