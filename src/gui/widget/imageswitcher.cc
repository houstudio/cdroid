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
#include <widget/imageswitcher.h>
#include <widget/imageview.h>

namespace cdroid{

DECLARE_WIDGET(ImageSwitcher)

ImageSwitcher::ImageSwitcher(int w,int h)
    :ViewSwitcher(w,h){
}

ImageSwitcher::ImageSwitcher(Context*ctx,const AttributeSet&atts)
  :ViewSwitcher(ctx,atts){
}

void ImageSwitcher::setImageResource(const std::string&resid){
    ImageView* image = (ImageView*)getNextView();
    image->setImageResource(resid);
    showNext();
}

void ImageSwitcher::setImageDrawable(Drawable* drawable){
    ImageView* image = (ImageView*)getNextView();
    image->setImageDrawable(drawable);
    showNext();
}

std::string ImageSwitcher::getAccessibilityClassName()const{
    return "ImageSwitcher";
}

}
