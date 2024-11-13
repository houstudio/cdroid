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
