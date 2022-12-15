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
InputDevice::InputDevice(int fdev):listener(nullptr){
    INPUTDEVICEINFO info;
    InputDeviceIdentifier di;

    mDeviceClasses=0;
    InputGetDeviceInfo(fdev,&info);
    di.name=info.name;
    di.product=info.product;
    di.vendor=info.vendor;
    mDeviceInfo.initialize(fdev,0,0,di,std::string(),0,0);

    // See if this is a keyboard.  Ignore everything in the button range except for
    // joystick and gamepad buttons which are handled like keyboards for the most part.
    bool haveKeyboardKeys = containsNonZeroByte(info.keyBitMask, 0, SIZEOF_BITS(BTN_MISC))
            || containsNonZeroByte(info.keyBitMask, SIZEOF_BITS(KEY_OK), SIZEOF_BITS(KEY_MAX + 1));
    bool haveGamepadButtons = containsNonZeroByte(info.keyBitMask, SIZEOF_BITS(BTN_MISC), SIZEOF_BITS(BTN_MOUSE))
            || containsNonZeroByte(info.keyBitMask, SIZEOF_BITS(BTN_JOYSTICK), SIZEOF_BITS(BTN_DIGI));
    if (haveKeyboardKeys || haveGamepadButtons) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_KEYBOARD;
    }

    if(TEST_BIT(BTN_MOUSE,info.keyBitMask) &&TEST_BIT(REL_X,info.relBitMask) &&TEST_BIT(REL_Y,info.relBitMask))
        mDeviceClasses=INPUT_DEVICE_CLASS_CURSOR;
    if(TEST_BIT(ABS_MT_POSITION_X, info.absBitMask) && TEST_BIT(ABS_MT_POSITION_Y, info.absBitMask)) {
        // Some joysticks such as the PS3 controller report axes that conflict
        // with the ABS_MT range.  Try to confirm that the device really is a touch screen.
        if (TEST_BIT(BTN_TOUCH, info.keyBitMask) || !haveGamepadButtons) {
            mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT;
        }
        // Is this an old style single-touch driver?
    } else if (TEST_BIT(BTN_TOUCH, info.keyBitMask)
            && TEST_BIT(ABS_X, info.absBitMask) && TEST_BIT(ABS_Y, info.absBitMask)) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH;
        // Is this a BT stylus?
    } else if ((TEST_BIT(ABS_PRESSURE, info.absBitMask) || TEST_BIT(BTN_TOUCH, info.keyBitMask))
            && !TEST_BIT(ABS_X, info.absBitMask) && !TEST_BIT(ABS_Y, info.absBitMask)) {
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
        uint32_t assumedClasses = mDeviceClasses | INPUT_DEVICE_CLASS_JOYSTICK;
        for (int i = 0; i <= ABS_MAX; i++) {
            if (TEST_BIT(i, info.absBitMask)
                    && (getAbsAxisUsage(i, assumedClasses) & INPUT_DEVICE_CLASS_JOYSTICK)) {
                mDeviceClasses = assumedClasses;
                break;
            }
        }
    }

    // Check whether this device has switches.
    for (int i = 0; i <= SW_MAX; i++) {
        if (TEST_BIT(i, info.swBitMask)) {
            mDeviceClasses |= INPUT_DEVICE_CLASS_SWITCH;
            break;
        }
    }

    // Check whether this device supports the vibrator.
    if (TEST_BIT(FF_RUMBLE, info.ffBitMask)) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_VIBRATOR;
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
    kmap=nullptr;
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
int InputDevice::getSource()const{
    return mDeviceInfo.getSources();
}
int InputDevice::getVendor()const{
    return mDeviceInfo.getIdentifier().vendor;
}
int InputDevice::getProduct()const{
    return mDeviceInfo.getIdentifier().product;
}

int InputDevice::getClasses()const{
    return mDeviceClasses;
}

const std::string&InputDevice::getName()const{
    return mDeviceInfo.getIdentifier().name;
}

KeyDevice::KeyDevice(int fd)
   :InputDevice(fd){
   msckey=0;
   mLastDownKey=-1;
   mRepeatCount=0;
   const std::string fname=App::getInstance().getDataPath()+getName()+".kl";
   KeyLayoutMap::load(fname,kmap);
}

int KeyDevice::putRawEvent(const struct timeval&tv,int type,int code,int value){
    int flags  =0;
    int keycode=code;
    if(!isValidEvent(type,code,value)){
         LOGD("invalid event type %x source=%x",type,mDeviceInfo.getSources());
         return -1;
    }
    switch(type){
    case EV_KEY:
        if(kmap)kmap->mapKey(code/*scancode*/,0,&keycode/*keycode*/,(uint32_t*)&flags);
        mLastDownKey=(value?keycode:-1);//key down
        if(mLastDownKey==keycode)
            mRepeatCount+=(value==0);
        else
            mRepeatCount=0;

        mEvent.initialize(getId(),getSource(),(value?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP)/*action*/,flags,
              keycode,code/*scancode*/,0/*metaState*/,mRepeatCount, mDownTime,SystemClock::uptimeNanos()/*eventtime*/);
        LOGV("fd[%d] keycode:%08x->%04x[%s] action=%d flags=%d",getId(),code,keycode, mEvent.getLabel(),value,flags);
        if(listener)listener(mEvent); 
        break;
    case EV_SYN:
        LOGV("fd[%d].SYN value=%d code=%d",getId(),value,code);
        break;
    default:LOGD("event type %x source=%x",type,getSource());break;
    }
    return 0;
}

TouchDevice::TouchDevice(int fd):InputDevice(fd){
    mPointSlot = 0;
}

static int ABS2AXIS(int absaxis){
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
    case ABS_PRESSURE:return MotionEvent::AXIS_PRESSURE;
    
    case ABS_WHEEL:/*REL_WHEEL*/ return MotionEvent::AXIS_PRESSURE; 
    }
}

void TouchDevice::setAxisValue(int index,int axis,int value,bool isRelative){
    auto it=mPointMAP.find(index);
    if(it==mPointMAP.end()){
        TouchPoint tp;
        tp.coord.clear();
        tp.prop.id=mPointMAP.size();
        auto it2=mPointMAP.insert(std::pair<int,TouchPoint>(index,tp));
        it=it2.first;
    }
    axis=ABS2AXIS(axis);
    if(isRelative){
        value=it->second.coord.getAxisValue(axis)+value;
    }
    it->second.coord.setAxisValue(axis,value);
}

int TouchDevice::putRawEvent(const struct timeval&tv,int type,int code,int value){
    if(!isValidEvent(type,code,value))return -1;
    LOGV("%lu:%04u %d,%d,%d",tv.tv_sec,tv.tv_usec,type,code,value);
    switch(type){
    case EV_KEY:
        switch(code){
        case BTN_TOUCH :
        case BTN_STYLUS:
            mEvent.setActionButton(MotionEvent::BUTTON_PRIMARY);
            mEvent.setAction(value?MotionEvent::ACTION_DOWN:MotionEvent::ACTION_UP);
            if(value){
                mMoveTime = mDownTime =tv.tv_sec*1000+tv.tv_usec/1000;
                mEvent.setButtonState(MotionEvent::BUTTON_PRIMARY);
            }else{
                mMoveTime =tv.tv_sec*1000+tv.tv_usec/1000;
                mEvent.setButtonState(mEvent.getButtonState()&(~MotionEvent::BUTTON_PRIMARY));
            }
            break;
        case BTN_0:
        case BTN_STYLUS2:
            mEvent.setActionButton(MotionEvent::BUTTON_SECONDARY);
            if(value)
                mEvent.setButtonState(MotionEvent::BUTTON_SECONDARY);
            else
                mEvent.setButtonState(mEvent.getButtonState()&(~MotionEvent::BUTTON_SECONDARY));
            break;
        case BTN_TOOL_FINGER:break;
        }break;
    case EV_ABS:
        switch(code){
        case ABS_X ... ABS_Z : 
            mMoveTime =tv.tv_sec*1000+tv.tv_usec/1000;
            setAxisValue(0,code,value,false) ; break;
        //case ABS_PRESSURE  : setAxisValue(0,code,value,false) ; break;
        case ABS_MT_SLOT    : mPointSlot=value ; break;
        case ABS_MT_TRACKING_ID:
        case ABS_MT_TOUCH_MAJOR:
        case ABS_MT_POSITION_X :
        case ABS_MT_POSITION_Y :
             setAxisValue(mPointSlot,code,value,false);break;
        case ABS_MT_WIDTH_MINOR:
        case ABS_MT_PRESSURE :
        case ABS_MT_DISTANCE :
        case ABS_MT_TOOL_TYPE:
        case ABS_MT_ORIENTATION:break; 
        }break;
    case EV_REL:
        switch(code){
        case REL_X:
        case REL_Y:
        case REL_Z:setAxisValue(0,code,value,true);break;
        }break;
    case EV_SYN:
        switch(code){
        case SYN_REPORT:
        case SYN_MT_REPORT:
            LOGV_IF(mPointMAP.size(),"%s time:%lld pos=%.f,%.f",MotionEvent::actionToString(mEvent.getAction()).c_str(),
               mMoveTime,mPointMAP.begin()->second.coord.getX(),mPointMAP.begin()->second.coord.getY() ); 
            mEvent.initialize(getId(),getSource(),mEvent.getAction(),mEvent.getActionButton(),
               0/*flags*/, 0/*edgeFlags*/, 0/*metaState*/, mEvent.getButtonState() , 0/*xOffset*/,0/*yOffset*/,
               0/*xPrecision*/, 0/*yPrecision*/ , mDownTime , mMoveTime , 0 , nullptr , nullptr);
            for(auto p:mPointMAP){
                mEvent.addSample(mMoveTime,p.second.prop,p.second.coord);
            }
            if(listener)listener(mEvent);
            if(mEvent.getAction()==MotionEvent::ACTION_UP)
                mPointMAP.clear();
            mEvent.setAction(MotionEvent::ACTION_MOVE);
        }break;
    }
    return 0;
}

MouseDevice::MouseDevice(int fd):TouchDevice(fd){
    memset(buttonstats,0,sizeof(buttonstats));
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
        const InputDeviceIdentifier& identifier, const std::string& alias, bool isExternal,
        bool hasMic) {
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
    size_t numRanges = mMotionRanges.size();
    for (size_t i = 0; i < numRanges; i++) {
        const MotionRange& range = mMotionRanges.at(i);
        if (range.axis == axis && range.source == source) {
            return &range;
        }
    }
    return NULL;
}

void InputDeviceInfo::addSource(uint32_t source) {
    mSources |= source;
}

void InputDeviceInfo::addMotionRange(int32_t axis, uint32_t source, float min, float max,
        float flat, float fuzz, float resolution) {
    MotionRange range = { axis, source, min, max, flat, fuzz, resolution };
    mMotionRanges.push_back(range);
}

void InputDeviceInfo::addMotionRange(const MotionRange& range) {
    mMotionRanges.push_back(range);
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
    // Search system repository.
    std::string path;
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
    path=getenv("ANDROID_DATA");
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
        if (deviceIdentifier.version != 0) {
            // Try vendor product version.
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
