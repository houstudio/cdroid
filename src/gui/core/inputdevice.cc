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

InputDevice::InputDevice(int fdev):listener(nullptr){
    INPUTDEVICEINFO devInfos;
    InputDeviceIdentifier di;
    Point sz;

    mDeviceClasses= 0;
    mKeyboardType = KEYBOARD_TYPE_NONE;
    InputGetDeviceInfo(fdev,&devInfos);
    di.name = devInfos.name;
    di.product= devInfos.product;
    di.vendor = devInfos.vendor;
    mDeviceInfo.initialize(fdev,0,0,di,devInfos.name,0,0);

    Display display =  WindowManager::getInstance().getDefaultDisplay();
    display.getRealSize(sz);
    mScreenWidth  = sz.x;
    mScreenHeight = sz.y;//ScreenSize is screen size in no roration
    if(mPrefs.getSectionCount()==0){
        mPrefs.load(App::getInstance().getDataPath() + std::string("input-devices.xml"));
    }
    LOGI("screenSize(%dx%d) rotation=%d",sz.x,sz.y,display.getRotation());
    LOGD("%d:[%s] Props=%02x%02x%02x%02x",fdev,devInfos.name,devInfos.propBitMask[0],
          devInfos.propBitMask[1],devInfos.propBitMask[2],devInfos.propBitMask[3]);
    for(int j=0;(j<ABS_CNT) && (j<sizeof(devInfos.axis)/sizeof(INPUTAXISINFO));j++){
        const INPUTAXISINFO*axis = devInfos.axis+j;
        if(axis->maximum != axis->minimum)
            mDeviceInfo.addMotionRange(axis->axis,0/*source*/,axis->minimum,axis->maximum,axis->flat,axis->fuzz,axis->resolution);
        LOGV_IF(axis->maximum!=axis->minimum,"devfd=%d axis[%d] range=%d,%d",fdev,axis->axis,axis->minimum,axis->maximum);
    }

    // See if this is a keyboard.  Ignore everything in the button range except for
    // joystick and gamepad buttons which are handled like keyboards for the most part.
    const bool haveKeyboardKeys = containsNonZeroByte(devInfos.keyBitMask, 0, SIZEOF_BITS(BTN_MISC))
            || containsNonZeroByte(devInfos.keyBitMask, SIZEOF_BITS(KEY_OK), SIZEOF_BITS(KEY_MAX + 1));
    const bool haveGamepadButtons = containsNonZeroByte(devInfos.keyBitMask, SIZEOF_BITS(BTN_MISC), SIZEOF_BITS(BTN_MOUSE))
            || containsNonZeroByte(devInfos.keyBitMask, SIZEOF_BITS(BTN_JOYSTICK), SIZEOF_BITS(BTN_DIGI));
    if (haveKeyboardKeys || haveGamepadButtons) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_KEYBOARD;
    }

    if(TEST_BIT(BTN_MOUSE,devInfos.keyBitMask) &&TEST_BIT(REL_X,devInfos.relBitMask) &&TEST_BIT(REL_Y,devInfos.relBitMask))
        mDeviceClasses = INPUT_DEVICE_CLASS_CURSOR;
    if(TEST_BIT(ABS_MT_POSITION_X, devInfos.absBitMask) && TEST_BIT(ABS_MT_POSITION_Y, devInfos.absBitMask)) {
        // Some joysticks such as the PS3 controller report axes that conflict
        // with the ABS_MT range.  Try to confirm that the device really is a touch screen.
        if (TEST_BIT(BTN_TOUCH, devInfos.keyBitMask) || !haveGamepadButtons) {
            mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT;
        }
        // Is this an old style single-touch driver?
    } else if (TEST_BIT(BTN_TOUCH, devInfos.keyBitMask)
            && TEST_BIT(ABS_X, devInfos.absBitMask) && TEST_BIT(ABS_Y, devInfos.absBitMask)) {
        mDeviceClasses |= INPUT_DEVICE_CLASS_TOUCH;
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
        uint32_t assumedClasses = mDeviceClasses | INPUT_DEVICE_CLASS_JOYSTICK;
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
            break;
        }
    }

    // Check whether this device supports the vibrator.
    if (TEST_BIT(FF_RUMBLE, devInfos.ffBitMask)) {
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
    kmap = nullptr;
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

KeyDevice::KeyDevice(int fd)
   :InputDevice(fd){
   msckey=0;
   mLastDownKey=-1;
   mRepeatCount=0;
   const std::string fname=App::getInstance().getDataPath()+getName()+".kl";
   KeyLayoutMap::load(fname,kmap);
}

int KeyDevice::isValidEvent(int type,int code,int value){
    return (type==EV_KEY)||(type==EV_SYN);
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
        mLastDownKey = (value ? keycode : -1);//key down
        if(mLastDownKey==keycode)
            mRepeatCount+=(value==0);
        else
            mRepeatCount=0;

        mEvent.initialize(getId(),getSources(),(value?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP)/*action*/,flags,
              keycode,code/*scancode*/,0/*metaState*/,mRepeatCount, mDownTime,SystemClock::uptimeMicros()/*eventtime*/);
        LOGV("fd[%d] keycode:%08x->%04x[%s] action=%d flags=%d",getId(),code,keycode, mEvent.getLabel(),value,flags);
        if(listener)listener(mEvent); 
        break;
    case EV_SYN:
        LOGV("fd[%d].SYN value=%d code=%d",getId(),value,code);
        break;
    default:LOGD("event type %x source=%x",type,getSources());break;
    }
    return 0;
}

TouchDevice::TouchDevice(int fd):InputDevice(fd){
    mPointSlot = 0;
    #define ISRANGEVALID(range) (range&&(range->max-range->min))
    const InputDeviceInfo::MotionRange*rangeX = mDeviceInfo.getMotionRange(ABS_X,0);
    Display display =  WindowManager::getInstance().getDefaultDisplay();
    if(rangeX==nullptr) rangeX = mDeviceInfo.getMotionRange(ABS_MT_POSITION_X,0);
    mTPWidth  = ISRANGEVALID(rangeX)? (rangeX->max-rangeX->min) : mScreenWidth;
    mMinX = ISRANGEVALID(rangeX) ? rangeX->min : 0;
    mMaxX = ISRANGEVALID(rangeX) ? rangeX->max : 0;

    const InputDeviceInfo::MotionRange*rangeY = mDeviceInfo.getMotionRange(ABS_Y,0);
    if(rangeY==nullptr) rangeY = mDeviceInfo.getMotionRange(ABS_MT_POSITION_Y,0);
    mTPHeight = ISRANGEVALID(rangeY) ? (rangeY->max-rangeY->min) : mScreenHeight;
    mMinY = ISRANGEVALID(rangeY) ? rangeY->min : 0;
    mMaxY = ISRANGEVALID(rangeY) ? rangeY->max : 0;

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
    LOGI("screen(%d,%d) rotation=%d [%s] X(%d,%d) Y(%d,%d) invert=%d,%d switchXY=%d",mScreenWidth, mScreenHeight,
        display.getRotation(),section.c_str(),mMinX,mMaxX,mMinY,mMaxY,mInvertX,mInvertY,mSwitchXY);

    mMatrix = Cairo::identity_matrix();
    //display rotation is defined as anticlockwise,but cairo use clockwise
    switch(display.getRotation()){
    case Display::ROTATION_0:/*do nothing*/break;
    case Display::ROTATION_270:
	   mMatrix.rotate(-M_PI/2.f);
	   mMatrix.translate(-int(mScreenWidth),0);
	   break;
    case Display::ROTATION_180:mMatrix.translate(mScreenWidth,mScreenHeight);
	   mMatrix.scale(-1,-1);
	   break;
    case Display::ROTATION_90 :
	   mMatrix.translate(mScreenHeight,0);
	   mMatrix.rotate(M_PI/2.f);
	   break;
    default:/**do nothing*/break;
    }
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
    default:return  -1; 
    }
}

void TouchDevice::setAxisValue(int index,int axis,int value,bool isRelative){
    const int rotation = WindowManager::getInstance().getDefaultDisplay().getRotation();
    auto it = mPointMAP.find(index);
    axis = ABS2AXIS(axis);
    switch(axis){
    case MotionEvent::AXIS_X:
       switch(rotation){
       case Display::ROTATION_0  : value -= mMinX ;break;
       case Display::ROTATION_90 : axis = MotionEvent::AXIS_Y; value -= mMinX; break; /*value=value;*/
       case Display::ROTATION_180: value= mMaxX - value; break;
       case Display::ROTATION_270: axis = MotionEvent::AXIS_Y; value = mMaxX - value; break;//tested
       }

       if(mScreenWidth != mTPWidth)
	   value = (value * mScreenWidth)/mTPWidth;
       if(mInvertX)value = mScreenWidth - value;
       if(mSwitchXY)axis= MotionEvent::AXIS_Y;
       break;
    case MotionEvent::AXIS_Y:
       switch(rotation){
       case Display::ROTATION_0  : value -= mMinY; break;
       case Display::ROTATION_90 : axis = MotionEvent::AXIS_X; value = mMaxY - value; break;
       case Display::ROTATION_180: value= mMaxY - value; break;
       case Display::ROTATION_270: axis = MotionEvent::AXIS_X; value -= mMinY ;break; /*value=value;*/
       }

       if(mScreenHeight != mTPHeight)
	   value = (value * mScreenHeight)/mTPHeight;
       if(mInvertY)value = mScreenHeight - value;
       if(mSwitchXY)axis= MotionEvent::AXIS_X;
       break;
    case MotionEvent::AXIS_Z:break;
    default:return;
    }
    if(it == mPointMAP.end()){
        TouchPoint tp;
        tp.coord.clear();
        tp.prop.id= mPointMAP.size();
        auto it2 = mPointMAP.insert(std::pair<int,TouchPoint>(index,tp));
        it = it2.first;
    }
    it->second.coord.setAxisValue(axis,value);
}

int TouchDevice::isValidEvent(int type,int code,int value){
    return (type==EV_KEY)||(type==EV_ABS)||(type==EV_SYN)||true;
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
            mEvent.setAction(value ? MotionEvent::ACTION_DOWN : MotionEvent::ACTION_UP);
            if(value){
                mMoveTime = mDownTime = tv.tv_sec * 1000000 + tv.tv_usec;
                mEvent.setButtonState(MotionEvent::BUTTON_PRIMARY);
            }else{
                mMoveTime = tv.tv_sec * 1000000 + tv.tv_usec;;
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
            mMoveTime = tv.tv_sec * 1000000 + tv.tv_usec;
            setAxisValue(0,code,value,false) ; break;
        //case ABS_PRESSURE  : setAxisValue(0,code,value,false) ; break;
        case ABS_MT_SLOT    : mPointSlot = value ; break;
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
        case REL_Y://LOGD("EV_REL code %x,%x",code,value);
        case REL_Z:setAxisValue(0,code,value,true);break;
        }break;
    case EV_SYN:
        switch(code){
        case SYN_REPORT:
        case SYN_MT_REPORT:
            mMoveTime =(tv.tv_sec * 1000000 + tv.tv_usec);
            mEvent.initialize(getId(),getSources(),mEvent.getAction(),mEvent.getActionButton(),
                 0/*flags*/, 0/*edgeFlags*/, 0/*metaState*/, mEvent.getButtonState() ,
                 0/*xOffset*/,0/*yOffset*/ , 0/*xPrecision*/, 0/*yPrecision*/ ,
                 mDownTime , mMoveTime , 0 , nullptr , nullptr);
            for(auto p:mPointMAP){
                mEvent.addSample(mMoveTime,p.second.prop,p.second.coord);
            }
            LOGV_IF(mEvent.getAction()==MotionEvent::ACTION_UP,"%s pos=%.f,%.f",MotionEvent::actionToString(mEvent.getAction()).c_str(),
            mPointMAP.begin()->second.coord.getX(),mEvent.getX(),mEvent.getY());
            if(mEvent.getAction()==MotionEvent::ACTION_DOWN){
                mLastDownX = mEvent.getX();
                mLastDownY = mEvent.getY();
            }else if(mEvent.getAction()==MotionEvent::ACTION_MOVE){
                if((mMoveTime-mDownTime<50*1000) && (mLastDownX==mEvent.getX()) && (mLastDownY==mEvent.getY())){
                    //the same positioned moveing ,skip this event
                    break;
                }
                mLastDownX= mEvent.getX();
                mLastDownY= mEvent.getY();
                mDownTime = mMoveTime;
            }
            if(listener){
                //mEvent.transform(mMatrix);
                listener(mEvent);
            }
            //if(mEvent.getAction()==MotionEvent::ACTION_UP) mPointMAP.clear();
            mEvent.setAction(MotionEvent::ACTION_MOVE);
        }break;
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
