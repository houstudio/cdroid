//#include <input/Input.h>
#include <map>
#include <bitset>
#include <chrono>
namespace cdroid {

constexpr size_t MAX_POINTERS = 16;
constexpr int32_t kEvdevHighResScrollUnitsPerDetent = 120;
enum class DeviceType {
    KEYBOARD,
    MOUSE,
    TOUCHSCREEN,
    DPAD,
    STYLUS,
    ROTARY_ENCODER,
};

int32_t openUinput(const char* readableName, int32_t vendorId, int32_t productId,
        const char* phys, DeviceType deviceType, int32_t screenHeight,int32_t screenWidth);

enum class UinputAction {
    RELEASE = 0,
    PRESS = 1,
    MOVE = 2,
    CANCEL = 3,
    ftl_last = CANCEL,
};

class VirtualInputDevice {
public:
    VirtualInputDevice(int32_t fd);
    virtual ~VirtualInputDevice();

protected:
    int32_t mFd;
    bool writeInputEvent(uint16_t type, uint16_t code, int32_t value,
                         std::chrono::nanoseconds eventTime);
    bool writeEvKeyEvent(int32_t androidCode, int32_t androidAction,
                         const std::map<int, int>& evKeyCodeMapping,
                         const std::map<int, UinputAction>& actionMapping,
                         std::chrono::nanoseconds eventTime);
};

class VirtualKeyboard : public VirtualInputDevice {
public:
    static const std::map<int, int> KEY_CODE_MAPPING;
    // Expose to share with VirtualDpad.
    static const std::map<int, UinputAction> KEY_ACTION_MAPPING;
    VirtualKeyboard(int32_t fd);
    virtual ~VirtualKeyboard() override;
    bool writeKeyEvent(int32_t androidKeyCode, int32_t androidAction,
                       std::chrono::nanoseconds eventTime);
};

class VirtualDpad : public VirtualInputDevice {
public:
    static const std::map<int, int> DPAD_KEY_CODE_MAPPING;
    VirtualDpad(int32_t fd);
    virtual ~VirtualDpad() override;
    bool writeDpadKeyEvent(int32_t androidKeyCode, int32_t androidAction,
                           std::chrono::nanoseconds eventTime);
};

class VirtualMouse : public VirtualInputDevice {
public:
    // Expose to share with VirtualStylus.
    static const std::map<int, UinputAction> BUTTON_ACTION_MAPPING;
    VirtualMouse(int32_t fd);
    virtual ~VirtualMouse() override;
    bool writeButtonEvent(int32_t androidButtonCode, int32_t androidAction,
                          std::chrono::nanoseconds eventTime);
    // TODO(b/259554911): changing float parameters to int32_t.
    bool writeRelativeEvent(float relativeX, float relativeY, std::chrono::nanoseconds eventTime);
    bool writeScrollEvent(float xAxisMovement, float yAxisMovement,
                          std::chrono::nanoseconds eventTime);

private:
    static const std::map<int, int> BUTTON_CODE_MAPPING;
    int32_t mAccumulatedHighResScrollX;
    int32_t mAccumulatedHighResScrollY;
};

class VirtualTouchscreen : public VirtualInputDevice {
public:
    // Expose to share with VirtualStylus.
    static const std::map<int, UinputAction> TOUCH_ACTION_MAPPING;
    VirtualTouchscreen(int32_t fd);
    virtual ~VirtualTouchscreen() override;
    // TODO(b/259554911): changing float parameters to int32_t.
    bool writeTouchEvent(int32_t pointerId, int32_t toolType, int32_t action, float locationX,
                         float locationY, float pressure, float majorAxisSize,
                         std::chrono::nanoseconds eventTime);

private:
    static const std::map<int, int> TOOL_TYPE_MAPPING;
    /* The set of active touch pointers on this device.
     * We only allow pointer id to go up to MAX_POINTERS because the maximum slots of virtual
     * touchscreen is set up with MAX_POINTERS. Note that in other cases Android allows pointer id
     * to go up to MAX_POINTERS_ID.
     */
    std::bitset<MAX_POINTERS> mActivePointers{};
    bool isValidPointerId(int32_t pointerId, UinputAction uinputAction);
    bool handleTouchDown(int32_t pointerId, std::chrono::nanoseconds eventTime);
    bool handleTouchUp(int32_t pointerId, std::chrono::nanoseconds eventTime);
};

class VirtualStylus : public VirtualInputDevice {
public:
    VirtualStylus(int32_t fd);
    ~VirtualStylus() override;
    bool writeMotionEvent(int32_t toolType, int32_t action, int32_t locationX, int32_t locationY,
                          int32_t pressure, int32_t tiltX, int32_t tiltY,
                          std::chrono::nanoseconds eventTime);
    bool writeButtonEvent(int32_t androidButtonCode, int32_t androidAction,
                          std::chrono::nanoseconds eventTime);

private:
    static const std::map<int, int> TOOL_TYPE_MAPPING;
    static const std::map<int, int> BUTTON_CODE_MAPPING;
    // True if the stylus is touching or hovering on the screen.
    bool mIsStylusDown;
    bool handleStylusDown(uint16_t tool, std::chrono::nanoseconds eventTime);
    bool handleStylusUp(uint16_t tool, std::chrono::nanoseconds eventTime);
};

class VirtualRotaryEncoder : public VirtualInputDevice {
public:
    VirtualRotaryEncoder(int32_t fd);
    virtual ~VirtualRotaryEncoder() override;
    bool writeScrollEvent(float scrollAmount, std::chrono::nanoseconds eventTime);

private:
    int32_t mAccumulatedHighResScrollAmount;
};

} // namespace cdroid
