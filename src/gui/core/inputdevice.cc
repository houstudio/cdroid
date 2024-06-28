#include <cdinput.h>
#include <inputdevice.h>
#include <systemclock.h>
#include <cdlog.h>
#include <chrono>
#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <keylayoutmap.h>
#include <core/app.h>
#include <core/windowmanager.h>
#include <linux/input.h>

using namespace std;
namespace cdroid{

static bool containsNonZeroByte(const uint8_t* array, uint32_t startIndex, uint32_t endIndex) {
    const uint8_t* end = array + endIndex;
    array += startIndex;
    while (array != end) {
        if (*(array++) != 0) {
            return true;
        }
    }
    return false;
}

#define TEST_BIT(bit, array)    ((array)[(bit)/8] & (1<<((bit)%8)))
#define SIZEOF_BITS(bits)  (((bits) + 7) / 8)

Preferences InputDevice::mPrefs;

InputDevice::InputDevice(int fdev){
    INPUTDEVICEINFO devInfos;
    InputDeviceIdentifier di;
    Point displaySize;
    std::ostringstream oss;

    mSeqID = 0;
    mDeviceClasses= 0;
    mKeyboardType = KEYBOARD_TYPE_NONE;
    InputGetDeviceInfo(fdev,&devInfos);
    di.name = devInfos.name;
    di.product= devInfos.product;
    di.vendor = devInfos.vendor;
    mDeviceInfo.initialize(fdev,0,0,di,devInfos.name,0,0);

    Display display =  WindowManager::getInstance().getDefaultDisplay();
    display.getRealSize(displaySize);
    mScreenWidth  = displaySize.x;
    mScreenHeight = displaySize.y;//ScreenSize is screen size in no roration
    if(mPrefs.getSectionCount()==0){
        mPrefs.load(App::getInstance().getDataPath() + std::string("input-devices.xml"));
    }
    LOGI("screenSize(%dx%d) rotation=%d",displaySize.x,displaySize.y,display.getRotation());

    // See if this is a keyboard.  Ignore everything in the button range except for
    // joystick and gamepad buttons which are handled like keyboards for the most part.
    const bool haveKeyboardKeys = containsNonZeroByte(devInfos.keyBitMask, 0, SIZEOF_BITS(BTN_MISC))
            || containsNonZeroByte(devInfos.keyBitMask, SIZEOF_BITS(KEY_OK), SIZEOF_BITS(KEY_MAX + 1));
    const bool haveGamepadButtons = containsNonZeroByte(devInfos.keyBitMask, SIZEOF_BITS(BTN_MISC), SIZEOF_BITS(BTN_MOUSE))
            || containsNonZeroByte(devInfos.keyBitMask, SIZEOF_BITS(BTN_JOYSTICK), SIZEOF_BITS(BTN_DIGI));
    if (haveKeyboardKeys || haveGamepadButtons) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_KEYBOARD;
        oss<<"Keyboard";
    }

    if(TEST_BIT(BTN_MOUSE,devInfos.keyBitMask) &&TEST_BIT(REL_X,devInfos.relBitMask) &&TEST_BIT(REL_Y,devInfos.relBitMask))
        mDeviceClasses = INPUT_DEVICE_CLASS_CURSOR;
    if(TEST_BIT(ABS_DISTANCE, devInfos.absBitMask)){
        oss<<"Proximity";//Proximity sensor
    }
    if(TEST_BIT(ABS_X, devInfos.absBitMask) && TEST_BIT(ABS_Y, devInfos.absBitMask) && TEST_BIT(ABS_Z, devInfos.absBitMask)){
        oss<<"Accelerometer";//Accelerometer sensor
    }
    if(TEST_BIT(ABS_RX, devInfos.absBitMask) && TEST_BIT(ABS_RY, devInfos.absBitMask) && TEST_BIT(ABS_RZ, devInfos.absBitMask)){
        oss<<"Gyroscope";//Gyroscope sensor
    }
    if(TEST_BIT(ABS_HAT0X, devInfos.absBitMask) && TEST_BIT(ABS_HAT0Y, devInfos.absBitMask) 
             && TEST_BIT(ABS_HAT1X, devInfos.absBitMask) && TEST_BIT(ABS_HAT1Y, devInfos.absBitMask)){
        oss<<"Magnetometer";//Magnetometer sensor
    }
    if(TEST_BIT(ABS_MT_POSITION_X, devInfos.absBitMask) && TEST_BIT(ABS_MT_POSITION_Y, devInfos.absBitMask)) {
        // Some joysticks such as the PS3 controller report axes that conflict
        // with the ABS_MT range.  Try to confirm that the device really is a touch screen.
        if (TEST_BIT(BTN_TOUCH, devInfos.keyBitMask) || !haveGamepadButtons) {
            mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT;
            oss<<"MTouch";
        }
        // Is this an old style single-touch driver?
    } else if ( TEST_BIT(ABS_X, devInfos.absBitMask) && TEST_BIT(ABS_Y, devInfos.absBitMask) ) {
        if(TEST_BIT(BTN_TOUCH, devInfos.keyBitMask)) mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH;
        else mDeviceClasses |= INPUT_DEVICE_CLASS_ROTARY_ENCODER;
        oss<<(TEST_BIT(BTN_TOUCH, devInfos.keyBitMask)?"STouch":"Rotaty Encoder");
        // Is this a BT stylus?
    } else if ((TEST_BIT(ABS_PRESSURE, devInfos.absBitMask) || TEST_BIT(BTN_TOUCH, devInfos.keyBitMask))
            && !TEST_BIT(ABS_X, devInfos.absBitMask) && !TEST_BIT(ABS_Y, devInfos.absBitMask)) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_EXTERNAL_STYLUS;
        // Keyboard will try to claim some of the buttons but we really want to reserve those so we
        // can fuse it with the touch screen data, so just take them back. Note this means an
        // external stylus cannot also be a keyboard device.
        mDeviceClasses &= ~INPUT_DEVICE_CLASS_KEYBOARD;
    }
 
    // See if this device is a joystick.
    // Assumes that joysticks always have gamepad buttons in order to distinguish them
    // from other devices such as accelerometers that also have absolute axes.
    if (haveGamepadButtons) {
        const uint32_t assumedClasses = mDeviceClasses | INPUT_DEVICE_CLASS_JOYSTICK;
        for (int i = 0; i <= ABS_MAX; i++) {
            if (TEST_BIT(i, devInfos.absBitMask)
                    && (getAbsAxisUsage(i, assumedClasses) & INPUT_DEVICE_CLASS_JOYSTICK)) {
                mDeviceClasses = assumedClasses;
                break;
            }
        }
    }

    // Check whether this device has switches.
    for (int i = 0; i <= SW_MAX; i++) {
        if (TEST_BIT(i, devInfos.swBitMask)) {
            mDeviceClasses |= INPUT_DEVICE_CLASS_SWITCH;
            oss<<"Switch";
            break;
        }
    }

    // Check whether this device supports the vibrator.
    if (TEST_BIT(FF_RUMBLE, devInfos.ffBitMask)) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_VIBRATOR;
        oss<<"Vibrator";
    }

    // Configure virtual keys.
    if ((mDeviceClasses & INPUT_DEVICE_CLASS_TOUCH)) {
        // Load the virtual keys for the touch screen, if any.
        // We do this now so that we can make sure to load the keymap if necessary.
        uint32_t status = 0;//loadVirtualKeyMapLocked(device);
        if (!status) {
            mDeviceClasses |= INPUT_DEVICE_CLASS_KEYBOARD;
        }
    } 
    kmap = nullptr;
    LOGI("%d:[%s] Props=%02x%02x%02x%02x[%s]",fdev,devInfos.name,devInfos.propBitMask[0],
          devInfos.propBitMask[1],devInfos.propBitMask[2],devInfos.propBitMask[3],oss.str().c_str());
    for(int j=0;(j<ABS_CNT) && (j<sizeof(devInfos.axis)/sizeof(INPUTAXISINFO));j++){
        const INPUTAXISINFO*axis = devInfos.axis+j;
        if(axis->maximum != axis->minimum)
            mDeviceInfo.addMotionRange(axis->axis,0/*source*/,axis->minimum,axis->maximum,axis->flat,axis->fuzz,axis->resolution);
        LOGV_IF(axis->maximum!=axis->minimum,"devfd=%d axis[%d] range=%d,%d",fdev,axis->axis,axis->minimum,axis->maximum);
    }
}

void InputDevice::bindDisplay(int id){
    Point displaySize;
    Display display =  WindowManager::getInstance().getDefaultDisplay();
    display.getRealSize(displaySize);
    mScreenWidth  = displaySize.x;
    mScreenHeight = displaySize.y;//ScreenSize is screen size in no roration
}

uint32_t getAbsAxisUsage(int32_t axis, uint32_t mDeviceClasses) {
    // Touch devices get dibs on touch-related axes.
    if (mDeviceClasses & INPUT_DEVICE_CLASS_TOUCH) {
        switch (axis) {
        case ABS_X:
        case ABS_Y:
        case ABS_PRESSURE:
        case ABS_TOOL_WIDTH:
        case ABS_DISTANCE:
        case ABS_TILT_X:
        case ABS_TILT_Y:
        case ABS_MT_SLOT:
        case ABS_MT_TOUCH_MAJOR:
        case ABS_MT_TOUCH_MINOR:
        case ABS_MT_WIDTH_MAJOR:
        case ABS_MT_WIDTH_MINOR:
        case ABS_MT_ORIENTATION:
        case ABS_MT_POSITION_X:
        case ABS_MT_POSITION_Y:
        case ABS_MT_TOOL_TYPE:
        case ABS_MT_BLOB_ID:
        case ABS_MT_TRACKING_ID:
        case ABS_MT_PRESSURE:
        case ABS_MT_DISTANCE:
            return INPUT_DEVICE_CLASS_TOUCH;
        }
    }

    // External stylus gets the pressure axis
    if (mDeviceClasses & INPUT_DEVICE_CLASS_EXTERNAL_STYLUS) {
        if (axis == ABS_PRESSURE) {
            return INPUT_DEVICE_CLASS_EXTERNAL_STYLUS;
        }
    }

    // Joystick devices get the rest.
    return mDeviceClasses & INPUT_DEVICE_CLASS_JOYSTICK;
}

int InputDevice::isValidEvent(int type,int code,int value){
    return true;
}

int InputDevice::getId()const{
    return mDeviceInfo.getId();
}

int InputDevice::getSources()const{
    return mDeviceInfo.getSources();
}

int InputDevice::getVendorId()const{
    return mDeviceInfo.getIdentifier().vendor;
}

int InputDevice::getProductId()const{
    return mDeviceInfo.getIdentifier().product;
}

bool InputDevice::isVirtual()const{
    return getId()<0;
}

bool InputDevice::isFullKeyboard()const{
    return ((getSources() & SOURCE_KEYBOARD) == SOURCE_KEYBOARD)
	    && (mKeyboardType == KEYBOARD_TYPE_ALPHABETIC);
}

bool InputDevice::supportsSource(int source)const{
    return (getSources() & source) == source;
}

int InputDevice::getClasses()const{
    return mDeviceClasses;
}

const std::string&InputDevice::getName()const{
    return mDeviceInfo.getIdentifier().name;
}

int InputDevice::getEventCount()const{
    return mEvents.size();
}

void InputDevice::pushEvent(InputEvent*e){
    mEvents.push_back(e);
}

InputEvent*InputDevice::popEvent(){
    InputEvent*event = mEvents.front();
    mEvents.pop_front();
    return event;
}

KeyDevice::KeyDevice(int fd)
   :InputDevice(fd){
   msckey = 0;
   mLastDownKey = -1;
   mRepeatCount = 0;
   mDeviceInfo.addSource(SOURCE_KEYBOARD);
   const std::string fname=App::getInstance().getDataPath()+getName()+".kl";
   KeyLayoutMap::load(fname,kmap);
}

int KeyDevice::isValidEvent(int type,int code,int value){
    return (type==EV_KEY)||(type==EV_SYN);
}

int KeyDevice::putRawEvent(const struct timeval&tv,int type,int code,int value){
    int flags  = 0;
    int keycode= code;
    if(!isValidEvent(type,code,value)){
         LOGD("invalid event type %x source=%x",type,mDeviceInfo.getSources());
         return -1;
    }
    switch(type){
    case EV_KEY:
        if(kmap)kmap->mapKey(code/*scancode*/,0,&keycode/*keycode*/,(uint32_t*)&flags);
        mLastDownKey = (value ? keycode : -1);//key down
        if(mLastDownKey==keycode)
            mRepeatCount+=(value==0);
        else
            mRepeatCount=0;

        mEvent.initialize(getId(),getSources(),(value?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP)/*action*/,flags,
              keycode,code/*scancode*/,0/*metaState*/,mRepeatCount, mDownTime,SystemClock::uptimeMicros()/*eventtime*/);
        LOGV("fd[%d] keycode:%08x->%04x[%s] action=%d flags=%d",getId(),code,keycode, mEvent.getLabel(),value,flags);
        mEvents.push_back(KeyEvent::obtain(mEvent));
        break;
    case EV_SYN:
        LOGV("fd[%d].SYN value=%d code=%d",getId(),value,code);
        break;
    default:LOGD("event type %x source=%x",type,getSources());break;
    }
    return 0;
}

TouchDevice::TouchDevice(int fd):InputDevice(fd){
    mTypeB = false;
    mTrackID = mSlotID = -1;
    mAxisFlags = 0;
    mLastAction= -1;
    mCorrectedDeviceClasses = mDeviceClasses;
    #define ISRANGEVALID(range) (range&&(range->max-range->min))
    std::vector<InputDeviceInfo::MotionRange>&mr = mDeviceInfo.getMotionRanges();
    for(int i=0;i<mr.size();i++){
        InputDeviceInfo::MotionRange&range=mr.at(i);
        const int axis = ABS2AXIS(range.axis);
        if(axis>=0)range.axis = axis;
        range = mr.at(i);
    }
    const InputDeviceInfo::MotionRange*rangeX = mDeviceInfo.getMotionRange(MotionEvent::AXIS_X,0);
    Display display =  WindowManager::getInstance().getDefaultDisplay();
    mTPWidth  = ISRANGEVALID(rangeX) ? (rangeX->max-rangeX->min) : mScreenWidth;
    mMinX = ISRANGEVALID(rangeX) ? rangeX->min : 0;
    mMaxX = ISRANGEVALID(rangeX) ? rangeX->max : mScreenWidth;

    const InputDeviceInfo::MotionRange*rangeY = mDeviceInfo.getMotionRange(MotionEvent::AXIS_Y,0);
    mTPHeight = ISRANGEVALID(rangeY) ? (rangeY->max-rangeY->min) : mScreenHeight;
    mMinY = ISRANGEVALID(rangeY) ? rangeY->min : 0;
    mMaxY = ISRANGEVALID(rangeY) ? rangeY->max : mScreenHeight;

    const InputDeviceInfo::MotionRange*rangePressure =  mDeviceInfo.getMotionRange(MotionEvent::AXIS_PRESSURE,0);
    mPressureMin = ISRANGEVALID(rangePressure) ? rangePressure->min : 0;
    mPressureMax = ISRANGEVALID(rangePressure) ? rangePressure->max : 0;

    const std::string section = getName();
    mInvertX = mInvertY = mSwitchXY = false;
    if(mPrefs.hasSection(section)){
        mMinX = mPrefs.getInt(section,"minX",mMinX);
        mMaxX = mPrefs.getInt(section,"maxX",mMaxX);
        mMinY = mPrefs.getInt(section,"minY",mMinY);
        mMaxY = mPrefs.getInt(section,"maxY",mMaxY);
        mInvertX = mPrefs.getBool(section,"invertX",false);
        mInvertY = mPrefs.getBool(section,"invertY",false);
        mSwitchXY= mPrefs.getBool(section,"switchXY",false);
    }
    mTPWidth = (mMaxX!=mMinX)?std::abs(mMaxX - mMinX):mScreenWidth;
    mTPHeight= (mMaxY!=mMinY)?std::abs(mMaxY - mMinY):mScreenHeight;
    mLastBits.clear();
    mCurrBits.clear();
    mEvent = nullptr;
    mActionButton = 0;
    mButtonState  = 0;
    mPointerCoords.resize(16);
    mPointerProps.resize(16);
    mCoord.clear();
    mProp.clear();
    mDeviceInfo.addSource(SOURCE_CLASS_POINTER);
    LOGI("screen(%d,%d) rotation=%d [%s] X(%d,%d) Y(%d,%d) invert=%d,%d switchXY=%d",mScreenWidth, mScreenHeight,
        display.getRotation(),section.c_str(),mMinX,mMaxX,mMinY,mMaxY,mInvertX,mInvertY,mSwitchXY);
}

int TouchDevice::ABS2AXIS(int absaxis){
    switch(absaxis){
    case ABS_MT_POSITION_X:
    case ABS_X:/*REL_X*/ return MotionEvent::AXIS_X;

    case ABS_MT_POSITION_Y:
    case ABS_Y:/*REL_Y*/ return MotionEvent::AXIS_Y;

    case ABS_Z:/*REL_Z*/ return MotionEvent::AXIS_Z;

    case ABS_RX:/*REL_RX*/return MotionEvent::AXIS_RX;
    case ABS_RY:/*REL_RY*/return MotionEvent::AXIS_RY;
    case ABS_RZ:/*REL_RZ*/return MotionEvent::AXIS_RZ;

    case ABS_MT_PRESSURE:
    case ABS_PRESSURE: return MotionEvent::AXIS_PRESSURE;
    case ABS_TOOL_WIDTH:
    case ABS_MT_TOUCH_MAJOR: return MotionEvent::AXIS_SIZE;
    case ABS_WHEEL:/*REL_WHEEL*/ return MotionEvent::AXIS_WHEEL;
    default:return  -1; 
    }
}

/*Android use 0->PointerCount as PointerID
 *other PointerID will caused many crashes */

void TouchDevice::setAxisValue(int raw_axis,int value,bool isRelative){
    const int rotation = WindowManager::getInstance().getDefaultDisplay().getRotation();
    int slot, axis = ABS2AXIS(raw_axis);
    int tmp;
    switch(axis){
    case MotionEvent::AXIS_X:
        switch(rotation){
        case Display::ROTATION_0  : value -= mMinX ; break;
        case Display::ROTATION_90 : axis = MotionEvent::AXIS_Y; value -= mMinX; break; /*value=value;*/
        case Display::ROTATION_180: value= mMaxX - value; break;
        case Display::ROTATION_270: axis = MotionEvent::AXIS_Y; value = mMaxX - value; break;//tested
        }

        if(mInvertX)value = mMaxX - value + mMinX;
        if(mSwitchXY){
            value = (value * mScreenWidth)/mTPHeight;
            axis= MotionEvent::AXIS_Y;
        }else if(mScreenWidth != mTPWidth){
            value = (value * mScreenWidth)/mTPWidth;
        }
        mCoord.setAxisValue(axis,value);
        break;
    case MotionEvent::AXIS_Y:
        switch(rotation){
        case Display::ROTATION_0  : value -= mMinY; break;
        case Display::ROTATION_90 : axis = MotionEvent::AXIS_X; value = mMaxY - value; break;
        case Display::ROTATION_180: value= mMaxY - value; break;
        case Display::ROTATION_270: axis = MotionEvent::AXIS_X; value -= mMinY ; break;
        }

        if(mInvertY)value = mMaxY - value + mMinY;
        if(mSwitchXY){
            value = (value * mScreenHeight)/mTPWidth;
            axis= MotionEvent::AXIS_X;
        }else{
            value = (value * mScreenHeight)/mTPHeight;
        }mCoord.setAxisValue(axis,value);
        break;
    case MotionEvent::AXIS_PRESSURE:
        tmp = std::max(mPressureMax - mPressureMin,1);
        mCoord.setAxisValue(axis,float(value - mPressureMin)/tmp);
        break;
    default:/*MotionEvent::AXIS_Z:*/ break;
    }/*endof switch(axis)*/

    if( (raw_axis>=ABS_MT_SLOT) && (raw_axis<=ABS_CNT) )
        mAxisFlags |= 1 << (raw_axis - ABS_MT_SLOT);
    switch(raw_axis){
    case ABS_X ... ABS_Z :
        mSlotID = 0 ; mTrackID = 0;
        mProp.id= 0;
        mDeviceClasses &= ~INPUT_DEVICE_CLASS_TOUCH_MT;
        break;
    case ABS_MT_POSITION_X...ABS_MT_POSITION_Y:
        //mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH_MT;
	break;
    case ABS_MT_SLOT:
        mTypeB = true;
        mSlotID= value;
        slot = mTrack2Slot.indexOfValue(value);
        if(slot>=0){
#if defined(USE_TRACKINGID_AS_POINTERID)&&USE_TRACKINGID_AS_POINTERID
            mTrackID = mTrack2Slot.keyAt(slot);
            mProp.id = mTrackID;
#else
            mProp.id = slot;
#endif
        }
        break;
    case ABS_MT_TRACKING_ID:
        slot = mTrack2Slot.indexOfKey(mTrackID = value);
        if( (slot ==-1) && (value!=-1) ){
            const int index = mTrack2Slot.size();
            mCurrBits.markBit(index);
            mTrack2Slot.put(mTrackID,(mTypeB?mSlotID:index));
            if( mTypeB==false ) mSlotID = index;
            slot = index;
            LOGV("Slot=%d TRACKID=%d %08x,%08x",mSlotID,value,mLastBits.value,mCurrBits.value);
        }else if((value==-1)&&mTypeB){//for TypeB
            const uint32_t pointerIndex = mTrack2Slot.indexOfValue(mSlotID);
            LOGV("clearbits %d %08x,%08x",pointerIndex,mLastBits.value,mCurrBits.value);
            mCurrBits.clearBit(pointerIndex);
        }
#if defined(USE_TRACKINGID_AS_POINTERID)&&USE_TRACKINGID_AS_POINTERID
        mProp.id = mTrackID;
#else
        mProp.id = slot;
#endif
        break;
    default:break;
    }
}

int TouchDevice::isValidEvent(int type,int code,int value){
    return (type==EV_KEY)||(type==EV_ABS)||(type==EV_SYN)||true;
}

int TouchDevice::getActionByBits(int& pointIndex){
    const uint32_t diffBits = mLastBits.value^mCurrBits.value;
    pointIndex = diffBits?BitSet32::firstMarkedBit(diffBits):mTrack2Slot.indexOfValue(mSlotID);
    if(((mDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT)==0))
        pointIndex = 0;
    if(((mCorrectedDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT)==0)&&(mDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT)){
        if(mLastAction==MotionEvent::ACTION_UP)
            return MotionEvent::ACTION_DOWN;
    }
    if(mLastBits.count()==mCurrBits.count()){
        return MotionEvent::ACTION_MOVE;
    }else if(mLastBits.count()<mCurrBits.count()){
        return mCurrBits.count()>1?MotionEvent::ACTION_POINTER_DOWN:MotionEvent::ACTION_DOWN;
    }else if(mLastBits.count()>mCurrBits.count()){
        return mCurrBits.count()>0?MotionEvent::ACTION_POINTER_UP:MotionEvent::ACTION_UP;
    }else{
        if(mLastBits.count()&&(mCurrBits.count()==0))return MotionEvent::ACTION_UP;
    }
    return MotionEvent::ACTION_MOVE;
}

static std::string printEvent(MotionEvent*e){
    std::ostringstream oss;
    oss<<"MotionEvent::Acion="<<e->getActionMasked()<<" Index="<<e->getActionIndex()<<" eventTime:"<<e->getDownTime()<<"/"<<e->getEventTime();
    oss<<" ("<<int(e->getX())<<","<<int(e->getY())<<"}"<<" historySize="<<e->getHistorySize();
    for(int i=0;i<e->getPointerCount();i++){
       oss<<std::endl<<"   Pointer["<<i<<"].id="<<e->getPointerId(i)<<" ";
       oss<<"("<<int(e->getX(i))<<","<<int(e->getY(i))<<") {";
       for(int j=0;j<e->getHistorySize();j++){
          oss<<"("<<e->getHistoricalEventTime(j)<<":"<<int(e->getHistoricalX(i,j))<<","<<int(e->getHistoricalY(i,j))<<")";
       }
       oss<<"}";
    }
    return oss.str(); 
}

int TouchDevice::putRawEvent(const struct timeval&tv,int type,int code,int value){
    int slot,pointerCount,pointerIndex,action;
    MotionEvent*lastEvent;
    if(!isValidEvent(type,code,value))return -1;
    //LOGV("%lu:%04u %d,%d,%d",tv.tv_sec,tv.tv_usec,type,code,value);
    switch(type){
    case EV_KEY:
        switch(code){
        case BTN_TOUCH :
        case BTN_STYLUS:
            mActionButton = MotionEvent::BUTTON_PRIMARY;
            if(value)mCurrBits.markBit(0);else mCurrBits.clearBit(0);
            mAxisFlags|=0x80000000;
            if(value){
                mMoveTime = mDownTime = tv.tv_sec * 1000 + tv.tv_usec/1000;
                mButtonState = MotionEvent::BUTTON_PRIMARY;
            }else{
                mMoveTime = tv.tv_sec * 1000 + tv.tv_usec/1000;
                mButtonState &= ~MotionEvent::BUTTON_PRIMARY;
            }
            break;
        case BTN_0:
        case BTN_STYLUS2:
            mActionButton = MotionEvent::BUTTON_SECONDARY;
            if(value)
                mButtonState = MotionEvent::BUTTON_SECONDARY;
            else
                mButtonState &= ~MotionEvent::BUTTON_SECONDARY;
            break;
        case BTN_TOOL_FINGER:break;
        }break;
    case EV_ABS:
        switch(code){
        case ABS_X ... ABS_Z :
        case ABS_MT_SLOT ... ABS_MT_TOOL_Y://MT_TOUCH_MAJOR...MT_TOOL_Y
             setAxisValue(code,value,false);break;
        }break;
    case EV_REL:
        if((code>=REL_X)&&(code<=REL_Z)){
            setAxisValue(code,value,true);
        }break;
    case EV_SYN:
        if((code!=SYN_REPORT) && (code!=SYN_MT_REPORT))break;
#ifndef DISABLE_MTASST
    #define TRACKING_FLAG ((1<<(ABS_MT_TRACKING_ID-ABS_MT_SLOT))|(1<<(ABS_MT_SLOT-ABS_MT_SLOT)))
        if( ((mDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT) && ((mAxisFlags&TRACKING_FLAG)==0))
                ||((mCorrectedDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT)==0) ){
            mCorrectedDeviceClasses &= ~INPUT_DEVICE_CLASS_TOUCH_MT;
            if((mAxisFlags&0x80000000)==0) {mCurrBits.markBit(0); mLastBits.markBit(0);mTrack2Slot.clear();}
            if( mAxisFlags&TRACKING_FLAG ) mCurrBits.clear();
            mTrack2Slot.put(0,0); mProp.id = 0;
        }
        if(code==SYN_REPORT)mAxisFlags = 0;
#endif

#if defined(USE_TRACKINGID_AS_POINTERID)&&USE_TRACKINGID_AS_POINTERID
        slot = mTrack2Slot.indexOfKey(mProp.id);
#else
        slot = mProp.id;
        if( (mProp.id==-1) && ((mCorrectedDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT)==0) )
            mProp.id = 0;
#endif
        slot = slot>=0?slot:0;
        mPointerProps [slot] = mProp;
        mPointerCoords[slot] = mCoord;
        if( code == SYN_MT_REPORT )break;
        action = getActionByBits(pointerIndex);
        mMoveTime = (tv.tv_sec * 1000LL + tv.tv_usec/1000);
        lastEvent = (mEvents.size()>1) ? (MotionEvent*)mEvents.back() : nullptr;
        pointerCount = (mCorrectedDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT) ? std::max(mLastBits.count(),mCurrBits.count()) : 1;
        if(lastEvent&&(lastEvent->getActionMasked()==MotionEvent::ACTION_MOVE)&&(action==MotionEvent::ACTION_MOVE)&&(mMoveTime-lastEvent->getDownTime()<100)){
            auto lastTime = lastEvent->getDownTime();
            lastEvent->addSample(mMoveTime,mPointerCoords.data());
            LOGV("eventdur=%d %s",int(mMoveTime-lastTime),printEvent(lastEvent).c_str());
        }else {
            const bool useBackupProps = ((action==MotionEvent::ACTION_UP)||(action==MotionEvent::ACTION_POINTER_UP))&&(mDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT);
            const PointerCoords  *coords = useBackupProps ? mPointerCoordsBak.data(): mPointerCoords.data();
            const PointerProperties*props= useBackupProps ? mPointerPropsBak.data() : mPointerProps.data();
            mEvent = MotionEvent::obtain(mMoveTime , mMoveTime , action , pointerCount,props,coords, 0/*metaState*/,mButtonState,
                 0,0/*x/yPrecision*/,getId()/*deviceId*/, 0/*edgeFlags*/, getSources(), 0/*flags*/);
            LOGV_IF(action!=MotionEvent::ACTION_MOVE,"mask=%08x,%08x (%.f,%.f)\n%s",mLastBits.value,mCurrBits.value,
                 mCoord.getX(),mCoord.getY(),printEvent(mEvent).c_str());
            mEvent->setActionButton(mActionButton);
            mEvent->setAction(action|(pointerIndex<<MotionEvent::ACTION_POINTER_INDEX_SHIFT));

            MotionEvent*e = MotionEvent::obtain(*mEvent);
            mEvents.push_back(e);
            mEvent->recycle();
        }
        mLastAction = action;
        if( mLastBits.count() > mCurrBits.count() ){
            const uint32_t pointerIndex = BitSet32::firstMarkedBit(mLastBits.value^mCurrBits.value);
            LOGV("clearbits %d %08x,%08x trackslot.size=%d",pointerIndex,mLastBits.value,mCurrBits.value, mTrack2Slot.size());
            if(mDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT) mCurrBits.clearBit(pointerIndex);
            if( pointerIndex<mTrack2Slot.size())
                mTrack2Slot.removeAt(pointerIndex);
            mPointerProps.erase (mPointerProps.begin() + pointerIndex);
            mPointerCoords.erase(mPointerCoords.begin()+ pointerIndex);
            mPointerProps.resize(mPointerProps.size()+1);
            mPointerCoords.resize(mPointerCoords.size()+1);
        }else {
            mPointerCoordsBak.clear();
            mPointerCoordsBak.assign(mPointerCoords.begin(),mPointerCoords.begin() + pointerCount);
            mPointerPropsBak.clear();
            mPointerPropsBak.assign(mPointerProps.begin(),mPointerProps.begin() + pointerCount);
        }

        mLastBits.value = mCurrBits.value;
        mProp.clear();
        if( (mDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT)&&(mCorrectedDeviceClasses&INPUT_DEVICE_CLASS_TOUCH_MT) && (mTypeB==false) ){
            mCoord.clear();
            mCurrBits.clear(); //only typeA
            mTrack2Slot.clear();
            for(int i = 0;i < pointerCount;i++){mPointerCoords[i].clear();mPointerCoords[i].clear();};
        }
        break;/*caseof EV_SYN*/
    }
    return 0;
}

MouseDevice::MouseDevice(int fd):TouchDevice(fd){
    mX = mY = 0;
    memset(mButtonStates,0,sizeof(mButtonStates));
}

int MouseDevice::isValidEvent(int type,int code,int value){
    return (type==EV_KEY)||(type==EV_REL)||(type==EV_SYN)||true;
}

int MouseDevice::putRawEvent(const struct timeval&tv,int type,int code,int value){
    if(!isValidEvent(type,code,value))return -1;
    return TouchDevice::putRawEvent(tv,type,code,value);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

InputDeviceInfo::InputDeviceInfo() {
    initialize(-1, 0, -1, InputDeviceIdentifier(), std::string(), false, false);
}

InputDeviceInfo::InputDeviceInfo(const InputDeviceInfo& other) :
        mId(other.mId), mGeneration(other.mGeneration), mControllerNumber(other.mControllerNumber),
        mIdentifier(other.mIdentifier), mAlias(other.mAlias), mIsExternal(other.mIsExternal),
        mHasMic(other.mHasMic), mSources(other.mSources),
        mKeyboardType(other.mKeyboardType), mHasVibrator(other.mHasVibrator),
        mHasButtonUnderPad(other.mHasButtonUnderPad),
        mMotionRanges(other.mMotionRanges) {
}

InputDeviceInfo::~InputDeviceInfo() {
}

void InputDeviceInfo::initialize(int32_t id, int32_t generation, int32_t controllerNumber,
       const InputDeviceIdentifier& identifier, const std::string& alias, bool isExternal,bool hasMic) {
    mId = id;
    mGeneration = generation;
    mControllerNumber = controllerNumber;
    mIdentifier = identifier;
    mAlias = alias;
    mIsExternal = isExternal;
    mHasMic = hasMic;
    mSources = 0;
    //mKeyboardType = AINPUT_KEYBOARD_TYPE_NONE;
    mHasVibrator = false;
    mHasButtonUnderPad = false;
    mMotionRanges.clear();
}

const InputDeviceInfo::MotionRange* InputDeviceInfo::getMotionRange(int32_t axis, uint32_t source) const {
    const size_t numRanges = mMotionRanges.size();
    for (size_t i = 0; i < numRanges; i++) {
        const MotionRange* range = mMotionRanges.data()+i;
        if (range->axis == axis && range->source == source) {
            return range;
        }
    }
    return NULL;
}

void InputDeviceInfo::addSource(uint32_t source) {
    mSources |= source;
}

void InputDeviceInfo::addMotionRange(int32_t axis, uint32_t source, float vmin, float vmax,
       float flat, float fuzz, float resolution) {
    MotionRange range;
    range.axis = axis;
    range.source = source;
    range.min  = vmin;
    range.max  = vmax;
    range.flat = flat;
    range.fuzz = fuzz;
    range.resolution = resolution;
    mMotionRanges.push_back(range);
}

void InputDeviceInfo::addMotionRange(const MotionRange& range) {
    mMotionRanges.push_back(range);
}

void InputDeviceInfo::addSensorInfo(const InputDeviceSensorInfo& info) {
    auto it = mSensors.find(info.type);
    if (it != mSensors.end()) {
        //LOGW("Sensor type %s already exists, will be replaced by new sensor added.",
        //      ftl::enum_string(info.type).c_str());
        it->second = info;
    }else{
        mSensors.insert({info.type, info});
    }
}

void InputDeviceInfo::addBatteryInfo(const InputDeviceBatteryInfo& info) {
    auto it = mBatteries.find(info.id);
    if (it != mBatteries.end()) {
        LOGW("Battery id %d already exists, will be replaced by new battery added.", info.id);
        it->second = info; 
    }else{
        mBatteries.insert({info.id, info});
    }
}

void InputDeviceInfo::addLightInfo(const InputDeviceLightInfo& info) {
    auto it = mLights.find(info.id);
    if (it != mLights.end()) {
        LOGW("Light id %d already exists, will be replaced by new light added.", info.id);
        it->second = info;
    }else{
        mLights.insert({info.id, info});
    }
}

std::vector<InputDeviceSensorInfo> InputDeviceInfo::getSensors()const{
    std::vector<InputDeviceSensorInfo> infos;
    infos.reserve(mSensors.size());
    for (const auto&info : mSensors) {
        infos.push_back(info.second);
    }
    return infos;
}

std::vector<InputDeviceLightInfo> InputDeviceInfo::getLights() const{
    std::vector<InputDeviceLightInfo> infos;
    infos.reserve(mLights.size());
    for (const auto& info: mLights) {
        infos.push_back(info.second);
    }
    return infos;
}

static bool isValidNameChar(char ch) {
    return isascii(ch) && (isdigit(ch) || isalpha(ch) || ch == '-' || ch == '_');
}

static void appendInputDeviceConfigurationFileRelativePath(std::string& path,
        const std::string& name, const std::string&type) {
    path.append(type);
    for (size_t i = 0; i < name.length(); i++) {
        char ch = name[i];
        if (!isValidNameChar(ch)) {
            ch = '_';
        }
        path.append(&ch, 1);
    }
    path.append(type);
}

std::string getInputDeviceConfigurationFilePathByName(const std::string& name,const std::string&type) {
    std::string path;// Search system repository.
    struct stat st;
    // Treblized input device config files will be located /odm/usr or /vendor/usr.
    const char *rootsForPartition[] {"/odm", "/vendor", getenv("ANDROID_ROOT")};
    for (size_t i = 0; i < sizeof(rootsForPartition)/sizeof(rootsForPartition[0]); i++) {
        path=rootsForPartition[i];
        path.append("/usr/");
        appendInputDeviceConfigurationFileRelativePath(path, name, type);
        if (!stat(path.c_str(),&st)) {
            return path;
        }
    }

    // Search user repository.
    // TODO Should only look here if not in safe mode.
    path = getenv("ANDROID_DATA");
    path.append("/system/devices/");
    appendInputDeviceConfigurationFileRelativePath(path, name, type);
    if (!stat(path.c_str(),&st)){
        return path;
    }
    // Not found.
    return std::string();
}

std::string getInputDeviceConfigurationFilePathByDeviceIdentifier(
      const InputDeviceIdentifier& deviceIdentifier,const std::string& type) {
    if (deviceIdentifier.vendor !=0 && deviceIdentifier.product != 0) {
        if (deviceIdentifier.version != 0) {// Try vendor product version.
            std::ostringstream name;
            name<<"Vendor_"<<hex<<setfill('0')<<setw(4)<<deviceIdentifier.vendor
                 <<"_Product_"<<deviceIdentifier.product<<"_Version_"<<deviceIdentifier.version;
            std::string versionPath(getInputDeviceConfigurationFilePathByName(name.str(),type));
            if (!versionPath.empty()) {
                return versionPath;
            }
        }
        // Try vendor product.
        std::ostringstream name;
        name<<"Vendor_"<<std::ios::hex<<setfill('0')<<setw(4)<<deviceIdentifier.vendor<<"_Product_"<<deviceIdentifier.product;
        std::string productPath(getInputDeviceConfigurationFilePathByName(name.str(),type));
        if (!productPath.empty()) {
            return productPath;
        }
    }
    // Try device name.
    return getInputDeviceConfigurationFilePathByName(deviceIdentifier.name, type);
}
}
