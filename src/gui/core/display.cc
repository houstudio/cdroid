#include <display.h>
#include <cdgraph.h>

namespace cdroid{

Display::Display(int id,DisplayInfo&displayInfo)
	:mDisplayId(id),mIsValid(true),mFlags(0){
   mDisplayInfo = displayInfo;
   mType = TYPE_BUILT_IN;
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
    GFXGetDisplaySize(mDisplayId,(UINT*)&outSize.x,(UINT*)&outSize.y);
    const int rotation = mDisplayInfo.rotation;
    if((rotation==ROTATION_90)||(mDisplayInfo.rotation==ROTATION_270)){
        const int temp = outSize.x;
        outSize.x = outSize.y;
        outSize.y = temp;
    }
}

void Display::getRealSize(Point&outSize){
    updateDisplayInfoLocked();
    GFXGetDisplaySize(mDisplayId,(UINT*)&outSize.x,(UINT*)&outSize.y);
}

int Display::getRotation(){
    updateDisplayInfoLocked();
    return mDisplayInfo.rotation;
}

void Display::getMetrics(DisplayMetrics&outMetrics){
    outMetrics.setToDefaults();
    GFXGetDisplaySize(mDisplayId,(UINT*)&outMetrics.widthPixels,(UINT*)&outMetrics.heightPixels);
}

void Display::getRealMetrics(DisplayMetrics&outMetrics){
    outMetrics.setToDefaults();
    GFXGetDisplaySize(mDisplayId,(UINT*)&outMetrics.widthPixels,(UINT*)&outMetrics.heightPixels);
}

}
