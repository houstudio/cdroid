#pragma once
#include <drawables/drawable.h>

namespace cdroid{

typedef Cairo::RefPtr<Cairo::RecordingSurface>Picture;

class PictureDrawable:public Drawable{
private:
    Picture mPicture;
public:
    PictureDrawable(Picture picture);
    Picture getPicture();
    void setPicture(Picture picture);
    void draw(Canvas& canvas);
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    int getOpacity()override;
    void setColorFilter(ColorFilter* colorFilter);
    void setAlpha(int alpha)override;
};

}
