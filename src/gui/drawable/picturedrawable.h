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
#ifndef __PICTURE_DRAWABLE_H__
#define __PICTURE_DRAWABLE_H__
#include <drawable/drawable.h>

namespace cdroid{

typedef Cairo::RefPtr<Cairo::RecordingSurface>Picture;

class PictureDrawable:public Drawable{
private:
    Picture mPicture;
public:
    PictureDrawable(Picture picture);
    Picture getPicture();
    void setPicture(Picture picture);
    void draw(Canvas& canvas)override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    int getOpacity()override;
    void setColorFilter(ColorFilter* colorFilter)override;
    void setAlpha(int alpha)override;
};

}
#endif
