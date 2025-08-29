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
#ifndef __INPUT_DEVICE_H__
#define __INPUT_DEVICE_H__
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/bitset.h>
#include <core/rect.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <time.h>
#include <core/preferences.h>
#include <core/sparsearray.h>
//#include <utils/flags.h>
namespace cdroid{
struct InputDeviceIdentifier {
    InputDeviceIdentifier() :
        bus(0), vendor(0), product(0), version(0), nonce(0){
    }
    // Information provided by the kernel.
    std::string name;
    std::string location;
    std::string uniqueId;
    uint16_t bus;
    uint16_t vendor;
    uint16_t product;
    uint16_t version;

    // A composite input device descriptor string that uniquely identifies the device
    // even across reboots or reconnections.  The value of this field is used by
    // upper layers of the input system to associate settings with individual devices.
    // It is hashed from whatever kernel provided information is available.
    // Ideally, the way this value is computed should not change between Android releases
    // because that would invalidate persistent settings that rely on it.
    std::string descriptor;

    // A value added to uniquely identify a device in the absence of a unique id. This
    // is intended to be a minimum way to distinguish from other active devices and may
    // reuse values that are not associated with an input anymore.
    uint16_t nonce;
};
/* Types of input device sensors. Keep sync with core/java/android/hardware/Sensor.java */
enum class InputDeviceSensorType : int32_t {
    ACCELEROMETER = 1,//ASENSOR_TYPE_ACCELEROMETER,
    MAGNETIC_FIELD = 2,//ASENSOR_TYPE_MAGNETIC_FIELD,
    ORIENTATION = 3,
    GYROSCOPE = 4,//ASENSOR_TYPE_GYROSCOPE,
    LIGHT = 5,//ASENSOR_TYPE_LIGHT,
    PRESSURE = 6,//ASENSOR_TYPE_PRESSURE,
    TEMPERATURE = 7,
    PROXIMITY = 8,//ASENSOR_TYPE_PROXIMITY,
    GRAVITY = 9,//ASENSOR_TYPE_GRAVITY,
    LINEAR_ACCELERATION = 10,//ASENSOR_TYPE_LINEAR_ACCELERATION,
    ROTATION_VECTOR = 11,//ASENSOR_TYPE_ROTATION_VECTOR,
    RELATIVE_HUMIDITY = 12,//ASENSOR_TYPE_RELATIVE_HUMIDITY,
    AMBIENT_TEMPERATURE = 13,//ASENSOR_TYPE_AMBIENT_TEMPERATURE,
    MAGNETIC_FIELD_UNCALIBRATED = 14,//ASENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
    GAME_ROTATION_VECTOR = 15,//ASENSOR_TYPE_GAME_ROTATION_VECTOR,
    GYROSCOPE_UNCALIBRATED = 16,//ASENSOR_TYPE_GYROSCOPE_UNCALIBRATED,
    SIGNIFICANT_MOTION = 17,//ASENSOR_TYPE_SIGNIFICANT_MOTION,

    ftl_first = ACCELEROMETER,
    ftl_last = SIGNIFICANT_MOTION
};

enum class InputDeviceSensorAccuracy : int32_t {
    ACCURACY_NONE = 0,
    ACCURACY_LOW = 1,
    ACCURACY_MEDIUM = 2,
    ACCURACY_HIGH = 3,
};

enum class InputDeviceSensorReportingMode : int32_t {
    CONTINUOUS = 0,
    ON_CHANGE = 1,
    ONE_SHOT = 2,
    SPECIAL_TRIGGER = 3,
};

enum class InputDeviceLightType : int32_t {
    INPUT = 0,
    PLAYER_ID = 1,
    KEYBOARD_BACKLIGHT = 2,

    ftl_last = KEYBOARD_BACKLIGHT
};

enum class InputDeviceLightCapability : uint32_t {
    /** Capability to change brightness of the light */
    BRIGHTNESS = 0x00000001,
    /** Capability to change color of the light */
    RGB = 0x00000002,
};

struct InputDeviceSensorInfo {
    explicit InputDeviceSensorInfo(const std::string& name,const std::string& vendor, int32_t version,
        InputDeviceSensorType type, InputDeviceSensorAccuracy accuracy, float maxRange, float resolution,
        float power, int32_t minDelay, int32_t fifoReservedEventCount, int32_t fifoMaxEventCount,
        const std::string& stringType, int32_t maxDelay, int32_t flags,int32_t id);
    // Name string of the sensor.
    std::string name;
    // Vendor string of this sensor.
    std::string vendor;
    // Version of the sensor's module.
    int32_t version;
    // Generic type of this sensor.
    InputDeviceSensorType type;
    // The current accuracy of sensor event.
    InputDeviceSensorAccuracy accuracy;
    // Maximum range of the sensor in the sensor's unit.
    float maxRange;
    // Resolution of the sensor in the sensor's unit.
    float resolution;
    // The power in mA used by this sensor while in use.
    float power;
    // The minimum delay allowed between two events in microsecond or zero if this sensor only
    // returns a value when the data it's measuring changes.
    int32_t minDelay;
    // Number of events reserved for this sensor in the batch mode FIFO.
    int32_t fifoReservedEventCount;
    // Maximum number of events of this sensor that could be batched.
    int32_t fifoMaxEventCount;
    // The type of this sensor as a string.
    std::string stringType;
    // The delay between two sensor events corresponding to the lowest frequency that this sensor
    // supports.
    int32_t maxDelay;
    // Sensor flags
    int32_t flags;
    // Sensor id, same as the input device ID it belongs to.
    int32_t id;
};

struct InputDeviceLightInfo {
    explicit InputDeviceLightInfo(const std::string& name, int32_t id, InputDeviceLightType type,
                                  /*ftl::Flags<InputDeviceLightCapability>*/uint32_t capabilityFlags,
                                  int32_t ordinal)
          : name(name), id(id), type(type), capabilityFlags(capabilityFlags), ordinal(ordinal) {}
    // Name string of the light.
    std::string name;
    // Light id
    int32_t id;
    // Type of the light.
    InputDeviceLightType type;
    // Light capabilities.
    /*ftl::Flags<InputDeviceLightCapability>*/uint32_t capabilityFlags;
    // Ordinal of the light
    int32_t ordinal;
};

struct InputDeviceBatteryInfo {
    explicit InputDeviceBatteryInfo(const std::string& name, int32_t id) : name(name), id(id) {}
    // Name string of the battery.
    std::string name;
    // Battery id
    int32_t id;
};

struct KeyboardLayoutInfo {
    explicit KeyboardLayoutInfo(const std::string& languageTag,const std::string& layoutType)
          : languageTag(languageTag), layoutType(layoutType) {}

    // A BCP 47 conformant language tag such as "en-US".
    std::string languageTag;
    // The layout type such as QWERTY or AZERTY.
    std::string layoutType;

    inline bool operator==(const KeyboardLayoutInfo& other) const {
        return languageTag == other.languageTag && layoutType == other.layoutType;
    }
    inline bool operator!=(const KeyboardLayoutInfo& other) const { return !(*this == other); }
};

class InputDeviceInfo {
public:
    InputDeviceInfo();
    InputDeviceInfo(const InputDeviceInfo& other);
    ~InputDeviceInfo();

    struct MotionRange {
        int32_t axis;
        uint32_t source;
        float min;
        float max;
        float flat;
        float fuzz;
        float resolution;
    };

    void initialize(int32_t id, int32_t generation, int32_t controllerNumber, 
        const InputDeviceIdentifier& identifier,const std::string& alias, bool isExternal,bool hasMic);

    inline int32_t getId() const { return mId; }
    inline int32_t getControllerNumber() const { return mControllerNumber; }
    inline int32_t getGeneration() const { return mGeneration; }
    inline const InputDeviceIdentifier& getIdentifier() const { return mIdentifier; }
    inline const std::string& getAlias() const { return mAlias; }
    inline const std::string& getDisplayName() const {
        return mAlias.empty() ? mIdentifier.name : mAlias;
    }
    inline bool isExternal() const { return mIsExternal; }
    inline bool hasMic() const { return mHasMic; }
    inline uint32_t getSources() const { return mSources; }

    MotionRange* getMotionRange(int32_t axis, uint32_t source);

    void addSource(uint32_t source);
    void addMotionRange(int32_t axis,uint32_t source,float min, float max,float flat,float fuzz,float resolution);
    void addMotionRange(const MotionRange& range);

    void addSensorInfo(const InputDeviceSensorInfo& info);
    void addBatteryInfo(const InputDeviceBatteryInfo& info);
    void addLightInfo(const InputDeviceLightInfo& info);

    inline void setKeyboardType(int32_t keyboardType) { mKeyboardType = keyboardType; }
    inline int32_t getKeyboardType() const { return mKeyboardType; }

    inline void setVibrator(bool hasVibrator) { mHasVibrator = hasVibrator; }
    inline bool hasVibrator() const { return mHasVibrator; }

    inline void setButtonUnderPad(bool hasButton) { mHasButtonUnderPad = hasButton; }
    inline bool hasButtonUnderPad() const { return mHasButtonUnderPad; }

    inline void setHasSensor(bool hasSensor) { mHasSensor = hasSensor; }
    inline bool hasSensor() const { return mHasSensor; }

    inline std::vector<MotionRange>& getMotionRanges() {
        return mMotionRanges;
    }
    std::vector<InputDeviceSensorInfo> getSensors()const;
    std::vector<InputDeviceLightInfo> getLights()const;
private:
    int32_t mId;
    int32_t mGeneration;
    int32_t mControllerNumber;
    InputDeviceIdentifier mIdentifier;
    std::string mAlias;
    bool mIsExternal;
    bool mHasMic;
    uint32_t mSources;
    int32_t mKeyboardType;
    bool mHasVibrator;
    bool mHasButtonUnderPad;
    bool mHasSensor;
    std::vector<MotionRange> mMotionRanges;
    std::unordered_map<int32_t/*InputDeviceSensorType*/, InputDeviceSensorInfo> mSensors;
    /* Map from light ID to light info */
    std::unordered_map<int32_t, InputDeviceLightInfo> mLights;
    /* Map from battery ID to battery info */
    std::unordered_map<int32_t, InputDeviceBatteryInfo> mBatteries;
};

/* Input device classes. */
enum InputDeviceClass{
    /* The input device is a keyboard or has buttons. */
    INPUT_DEVICE_CLASS_KEYBOARD      = 0x00000001,

    /* The input device is an alpha-numeric keyboard (not just a dial pad). */
    INPUT_DEVICE_CLASS_ALPHAKEY      = 0x00000002,

    /* The input device is a touchscreen or a touchpad (either single-touch or multi-touch). */
    INPUT_DEVICE_CLASS_TOUCH         = 0x00000004,

    /* The input device is a cursor device such as a trackball or mouse. */
    INPUT_DEVICE_CLASS_CURSOR        = 0x00000008,

    /* The input device is a multi-touch touchscreen. */
    INPUT_DEVICE_CLASS_TOUCH_MT      = 0x00000010,

    /* The input device is a directional pad (implies keyboard, has DPAD keys). */
    INPUT_DEVICE_CLASS_DPAD          = 0x00000020,

    /* The input device is a gamepad (implies keyboard, has BUTTON keys). */
    INPUT_DEVICE_CLASS_GAMEPAD       = 0x00000040,

    /* The input device has switches. */
    INPUT_DEVICE_CLASS_SWITCH        = 0x00000080,

    /* The input device is a joystick (implies gamepad, has joystick absolute axes). */
    INPUT_DEVICE_CLASS_JOYSTICK      = 0x00000100,

    /* The input device has a vibrator (supports FF_RUMBLE). */
    INPUT_DEVICE_CLASS_VIBRATOR      = 0x00000200,

    /* The input device has a microphone. */
    INPUT_DEVICE_CLASS_MIC           = 0x00000400,

    /* The input device is an external stylus (has data we want to fuse with touch data). */
    INPUT_DEVICE_CLASS_EXTERNAL_STYLUS = 0x00000800,

    /* The input device has a rotary encoder */
    INPUT_DEVICE_CLASS_ROTARY_ENCODER = 0x00001000,

    /* The input device is virtual (not a real device, not part of UI configuration). */
    INPUT_DEVICE_CLASS_VIRTUAL       = 0x40000000,

    /* The input device is external (not built-in). */
    INPUT_DEVICE_CLASS_EXTERNAL      = 0x80000000,
};
extern uint32_t getAbsAxisUsage(int32_t axis, uint32_t deviceClasses);

class InputDevice{
public:
    static constexpr int SOURCE_CLASS_MASK     = 0x000000ff;
    static constexpr int SOURCE_CLASS_NONE     = 0x00000000;
    static constexpr int SOURCE_CLASS_BUTTON   = 0x00000001;
    static constexpr int SOURCE_CLASS_POINTER  = 0x00000002;
    static constexpr int SOURCE_CLASS_TRACKBALL= 0x00000004;
    static constexpr int SOURCE_CLASS_POSITION = 0x00000008;
    static constexpr int SOURCE_CLASS_JOYSTICK = 0x00000010;
    static constexpr int SOURCE_UNKNOWN  = 0x00000000;
    static constexpr int SOURCE_KEYBOARD = 0x00000100 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_DPAD = 0x00000200 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_GAMEPAD = 0x00000400 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_TOUCHSCREEN = 0x00001000 | SOURCE_CLASS_POINTER;
    static constexpr int SOURCE_MOUSE = 0x00002000 | SOURCE_CLASS_POINTER;
    static constexpr int SOURCE_STYLUS = 0x00004000 | SOURCE_CLASS_POINTER;
    static constexpr int SOURCE_BLUETOOTH_STYLUS =   0x00008000 | SOURCE_STYLUS;
    static constexpr int SOURCE_TRACKBALL = 0x00010000 | SOURCE_CLASS_TRACKBALL;
    static constexpr int SOURCE_MOUSE_RELATIVE = 0x00020000 | SOURCE_CLASS_TRACKBALL;
    static constexpr int SOURCE_TOUCHPAD = 0x00100000 | SOURCE_CLASS_POSITION;
    static constexpr int SOURCE_TOUCH_NAVIGATION = 0x00200000 | SOURCE_CLASS_NONE;
    static constexpr int SOURCE_ROTARY_ENCODER = 0x00400000 | SOURCE_CLASS_NONE;
    static constexpr int SOURCE_JOYSTICK = 0x01000000 | SOURCE_CLASS_JOYSTICK;
    static constexpr int SOURCE_HDMI = 0x02000000 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_ANY = 0xffffff00;

    static constexpr int KEYBOARD_TYPE_NONE = 0;
    static constexpr int KEYBOARD_TYPE_NON_ALPHABETIC = 1;
    static constexpr int KEYBOARD_TYPE_ALPHABETIC = 2;
protected:
    int mDeviceClasses;
    int mKeyboardType;
    int32_t mScreenWidth;
    int32_t mScreenHeight;
    uint32_t mAxisFlags;
    uint32_t mCorrectedDeviceClasses;
    int32_t mLastAction;
    int32_t mScreenRotation;
    int32_t mSeqID;
    nsecs_t mLastEventTime;
    InputDeviceInfo mDeviceInfo;
    class KeyLayoutMap*kmap;
    static Preferences mPrefs;
    std::deque<InputEvent*>mEvents;
    virtual int isValidEvent(int type,int code,int value);
public:
    InputDevice(int fdev);
    virtual int putEvent(long sec,long usec,int type,int code,int value){return 0;}//PENDING need more rawevent OK,wecan getevent now
    int getId()const;
    int getProductId()const;
    int getVendorId()const;
    bool isVirtual()const;
    bool isFullKeyboard()const;
    bool supportsSource(int source)const;
    int getSources()const;
    int getClasses()const;
    int getEventCount()const;
    void pushEvent(InputEvent*);
    InputEvent*popEvent();
    const std::string&getName()const;
    const InputDeviceIdentifier&getIdentifier()const;
    void getLastEvent(int&action,nsecs_t&etime)const;
    void bindDisplay(int);
};

class KeyDevice:public InputDevice{
private:
    int mLastDownKey;
    int mRepeatCount;
protected:
    int msckey;
    KeyEvent mEvent;
    nsecs_t mDownTime;
    int isValidEvent(int type,int code,int value)override;
public:
    KeyDevice(int fd);
    int putEvent(long sec,long usec,int type,int code,int value)override;
};

class TouchDevice:public InputDevice{
private:
    int parseVirtualKeys(const std::string&);
protected:
    MotionEvent* mEvent;
    nsecs_t mDownTime;
    nsecs_t mMoveTime;
    int32_t mSlotID;
    int32_t mTrackID;
    int32_t mTPWidth;
    int32_t mTPHeight;
    int32_t mMinX,mMaxX;
    int32_t mMinY,mMaxY;
    int32_t mPressureMin;
    int32_t mPressureMax;
    int32_t mActionButton;
    int32_t mButtonState;
    bool mSwitchXY;
    bool mInvertX;
    bool mInvertY;
    bool mTypeB;
    BitSet32 mLastBits,mCurrBits;
    SparseArray<int>mTrack2Slot;
    PointerCoords mCoord;
    PointerProperties mProp;
    std::vector<PointerCoords>mPointerCoords;
    std::vector<PointerCoords>mPointerCoordsBak;
    std::vector<PointerProperties>mPointerProps;
    std::vector<PointerProperties>mPointerPropsBak;
    std::vector<std::pair<Rect,int>>mVirtualKeyMap;
    int getActionByBits(int&pointIndex);
    void setAxisValue(int axis,int value,bool isRelative);
    int isValidEvent(int type,int code,int value)override;
    int ABS2AXIS(int absaxis);
public:
    TouchDevice(int fd);
    int putEvent(long sec,long usec,int type,int code,int value)override;
};

class MouseDevice:public TouchDevice{
protected:
    uint8_t mButtonStates[16];
    int32_t mX,mY,mZ;
    int isValidEvent(int type,int code,int value)override;
public:
    MouseDevice(int fd);
    int putEvent(long sec,long usec,int type,int code,int value)override;
};
}//namespace
#endif 
