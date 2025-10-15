/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <drawable/picturedrawable.h>

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
