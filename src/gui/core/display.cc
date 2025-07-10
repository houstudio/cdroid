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
#include <display.h>
#include <cdgraph.h>

namespace cdroid{

Display::Display(int id,DisplayInfo&displayInfo)
	:mDisplayId(id),mIsValid(true),mType(TYPE_BUILT_IN)
    ,mFlags(0),mDisplayInfo(displayInfo){
   mDisplayInfo.type= mType;
   displayInfo.type = mType;
}

void Display::updateDisplayInfoLocked(){
}

int Display::getDisplayId(){
    updateDisplayInfoLocked();
    return mDisplayId;
}

int Display::getType(){
    return mType;
}

bool Display::isValid(){
    return mIsValid;
}

bool Display::getDisplayInfo(DisplayInfo&info){
    updateDisplayInfoLocked();
    info = mDisplayInfo;
    return true;
}

void Display::getSize(Point&outSize){
    updateDisplayInfoLocked();
    GFXGetDisplaySize(mDisplayId,(uint32_t*)&outSize.x,(uint32_t*)&outSize.y);
    const int rotation = mDisplayInfo.rotation;
    if((rotation==ROTATION_90)||(mDisplayInfo.rotation==ROTATION_270)){
        const int temp = outSize.x;
        outSize.x = outSize.y;
        outSize.y = temp;
    }
}

void Display::getRealSize(Point&outSize){
    updateDisplayInfoLocked();
    GFXGetDisplaySize(mDisplayId,(uint32_t*)&outSize.x,(uint32_t*)&outSize.y);
}

int Display::getState(){
    updateDisplayInfoLocked();
    return mDisplayInfo.state;
}

int Display::getRotation(){
    updateDisplayInfoLocked();
    return mDisplayInfo.rotation;
}

void Display::getMetrics(DisplayMetrics&outMetrics){
    outMetrics.setToDefaults();
    GFXGetDisplaySize(mDisplayId,(uint32_t*)&outMetrics.widthPixels,(uint32_t*)&outMetrics.heightPixels);
}

void Display::getRealMetrics(DisplayMetrics&outMetrics){
    outMetrics.setToDefaults();
    GFXGetDisplaySize(mDisplayId,(uint32_t*)&outMetrics.widthPixels,(uint32_t*)&outMetrics.heightPixels);
}

}
