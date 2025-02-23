#include <drawables/picturedrawable.h>

namespace cdroid{

PictureDrawable::PictureDrawable(Picture picture){
    mPicture=picture;
}

Picture PictureDrawable::getPicture(){
    return mPicture;
}

void PictureDrawable::setPicture(Picture picture){
    mPicture=picture;
}

void PictureDrawable::draw(Canvas& canvas){
    if (mPicture) {
        Rect bounds = getBounds();
        canvas.save();
        canvas.set_source(mPicture,bounds.left, bounds.top);
        canvas.rectangle(bounds.left, bounds.top,bounds.width, bounds.height);
        canvas.clip();
        canvas.paint();
        canvas.restore();
    }
}

int PictureDrawable::getIntrinsicWidth() {
    return mPicture ? mPicture->ink_extents().width : -1;
}

int PictureDrawable::getIntrinsicHeight() {
    return mPicture ? mPicture->ink_extents().height: -1;
}

int PictureDrawable::getOpacity(){
        // not sure, so be safe
    return TRANSLUCENT;
}

void PictureDrawable::setColorFilter(ColorFilter* colorFilter) {
}

void PictureDrawable::setAlpha(int alpha){
}

}
