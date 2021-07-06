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
#include <app.h>

using namespace std;
namespace cdroid{


InputDevice::InputDevice(int fdev):listener(nullptr){
   INPUTDEVICEINFO info;
   InputDeviceIdentifier di;
   InputGetDeviceInfo(fdev,&info);
   di.name=info.name;
   di.product=info.product;
   di.vendor=info.vendor;
   devinfo.initialize(fdev,0,0,di,std::string(),0,0);
   devinfo.addSource(info.source);
   kmap=nullptr;
   LOGD("device %d source=%x vid/pid=%x/%x name=%s",fdev,info.source,info.vendor,info.product,info.name);
}

int InputDevice::isvalid_event(const INPUTEVENT*e){
    return ((1<<e->type)&getSource())==(1<<e->type);
}

int InputDevice::getId()const{return devinfo.getId();}
int InputDevice::getSource()const{return devinfo.getSources();}
int InputDevice::getVendor()const{return devinfo.getIdentifier().vendor;}
int InputDevice::getProduct()const{return devinfo.getIdentifier().product;}

const std::string&InputDevice::getName()const{return devinfo.getIdentifier().name;}

KeyDevice::KeyDevice(int fd)
   :InputDevice(fd){
   msckey=0;
   lastDownKey=-1;
   repeatCount=0;
   const std::string fname=App::getInstance().getDataPath()+getName()+".kl";
   KeyLayoutMap::load(fname,kmap);
}

int KeyDevice::putrawdata(const INPUTEVENT*e){
    int flags;
    int keycode=e->code;
    if(!isvalid_event(e)){
         LOGD("invalid event type %x source=%x",e->type,devinfo.getSources());
         return -1;
    }
    switch(e->type){
    case EV_KEY:
        if(kmap)kmap->mapKey(e->code/*scancode*/,0,&keycode/*keycode*/,(uint32_t*)&flags);
        lastDownKey=(e->value?keycode:-1);//key down
        if(lastDownKey==keycode)
            repeatCount+=(e->value==0);
        else
            repeatCount=0;

        key.initialize(getId(),getSource(),(e->value?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP)/*action*/,flags,
                       keycode,e->code/*scancode*/,0/*metaState*/,repeatCount, downtime,SystemClock::uptimeNanos()/*eventtime*/);
        LOGV("fd[%d] keycode:%08x->%04x[%s] action=%d flags=%d",getId(),e->code,keycode, key.getLabel(),e->value,flags);
        if(listener)listener(key); 
        break;
    case EV_SYN:
        LOGV("fd[%d].SYN value=%d code=%d",getId(),e->value,e->code);
        break;
    default:LOGD("event type %x source=%x",e->type,getSource());break;
    }
    return 0;
}

TouchDevice::TouchDevice(int fd):InputDevice(fd){
    memset(coords,0,sizeof(coords));
    memset(ptprops,0,sizeof(ptprops));
    memset(buttonstats,0,sizeof(buttonstats));
}

int TouchDevice::putrawdata(const INPUTEVENT*e){
    if(!isvalid_event(e))return -1;
    return 0;
}

int MouseDevice::putrawdata(const INPUTEVENT*e){
    BYTE btnmap[]={ MotionEvent::BUTTON_PRIMARY  ,/*BTN_LEFT*/
                    MotionEvent::BUTTON_SECONDARY,/*BTN_RIGHT*/
                    MotionEvent::BUTTON_TERTIARY/*BTN_MIDDLE*/ ,0,0};
    int act_btn=e->value-BTN_MOUSE;
    if(!isvalid_event(e))return -1;
    switch(e->type){
    case EV_KEY:
        downtime=SystemClock::uptimeNanos();
        buttonstats[act_btn]=e->code;
        LOGV("Key %x /%d btn=%d %lld",e->value,e->code,btnmap[act_btn],downtime);
        mt.setAction(e->code?MotionEvent::ACTION_DOWN:MotionEvent::ACTION_UP);
        mt.setActionButton(btnmap[act_btn]);
        if(listener)listener(mt);
        if(e->code==0)mt.setActionButton(0);
        break;
    case EV_ABS:
        coords->setAxisValue(e->code,e->value);
        mt.setAction(MotionEvent::ACTION_MOVE);
        break;
    case EV_SYN:
        mt.initialize(getId(),getSource(),mt.getAction()/*action*/,mt.getActionButton()/*actionbutton*/,
               0/*flags*/,  0/*edgeFlags*/,0/*metaState*/,0/*buttonState*/,
               0/*xOffset*/,0/*yOffset*/,0/*xPrecision*/,0/*yPrecision*/,
               downtime,SystemClock::uptimeNanos(),1/*pointerCount*/,ptprops,coords);
        if(listener)listener(mt);
        break;
    }
    return 0;
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
