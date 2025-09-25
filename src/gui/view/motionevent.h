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
#ifndef __MOTION_EVENT_H__
#define __MOTION_EVENT_H__
#include <view/inputevent.h>
#include <cairomm/matrix.h>
#include <core/bitset.h>
#include <limits>
#include <cmath>

namespace cdroid{

#if defined(_MSC_VER)  // MSVC
#define ALIGN_ATTRIBUTE(n) //__declspec(align(n))
#elif defined(__GNUC__) || defined(__clang__)  // GCC或Clang
#define ALIGN_ATTRIBUTE(n) __attribute__((aligned(n)))
#else
#error "Unsupported compiler"
#endif

struct PointerCoords {
    enum { MAX_AXES = 30 }; // 30 so that sizeof(PointerCoords) == 128

    // Bitfield of axes that are present in this structure.
    uint64_t bits ALIGN_ATTRIBUTE(8);
    bool isResampled;
    // Values of axes that are stored in this structure packed in order by axis id
    // for each axis that is present in the structure according to 'bits'.
    float values[MAX_AXES];

    void clear();
    bool isEmpty() const;

    float getAxisValue(int32_t axis) const;
    int setAxisValue(int32_t axis, float value);

    void scale(float scale);
    void applyOffset(float xOffset, float yOffset);

    float getX() const;
    float getY() const;

    bool operator==(const PointerCoords& other) const;
    bool operator!=(const PointerCoords& other) const;
    void copyFrom(const PointerCoords& other);

private:
    void tooManyAxes(int axis);
};

/* Pointer property data.*/
struct PointerProperties {
    // The id of the pointer.
    int32_t id;

    // The pointer tool type.
    int32_t toolType;

    void clear();
    bool operator==(const PointerProperties& other) const;
    bool operator!=(const PointerProperties& other) const;
    PointerProperties();
    void copyFrom(const PointerProperties& other);
};

class MotionEvent:public InputEvent{
public:
    enum{
        ACTION_MASK = 0xff,
        ACTION_POINTER_INDEX_MASK = 0xff00,
        ACTION_POINTER_INDEX_SHIFT=8,
        ACTION_DOWN = 0,
        ACTION_UP = 1,
        ACTION_MOVE = 2,
        ACTION_CANCEL = 3,
        ACTION_OUTSIDE = 4,
        ACTION_POINTER_DOWN = 5,
        ACTION_POINTER_UP = 6,
        ACTION_HOVER_MOVE = 7,
        ACTION_SCROLL = 8,
        ACTION_HOVER_ENTER = 9,
        ACTION_HOVER_EXIT = 10,
        ACTION_BUTTON_PRESS = 11,
        ACTION_BUTTON_RELEASE = 12
    };
    /*Flag indicating the motion event intersected the top edge of the screen.*/
    enum{
        EDGE_TOP   = 0x0001,
        EDGE_BOTTOM= 0x0002,
        EDGE_LEFT  = 0x0004,
        EDGE_RIGHT = 0x0008
    };
    enum{
        INVALID_POINTER_ID = -1,
        AXIS_X = 0,
        AXIS_Y = 1,
        AXIS_PRESSURE = 2,
        AXIS_SIZE = 3,
        AXIS_TOUCH_MAJOR = 4,
        AXIS_TOUCH_MINOR = 5,
        AXIS_TOOL_MAJOR  = 6,
        AXIS_TOOL_MINOR  = 7,
        AXIS_ORIENTATION = 8,
        AXIS_VSCROLL = 9,
        AXIS_HSCROLL = 10,
        AXIS_Z  = 11,
        AXIS_RX = 12,
        AXIS_RY = 13,
        AXIS_RZ = 14,
        AXIS_HAT_X = 15,
        AXIS_HAT_Y = 16,
        AXIS_LTRIGGER = 17,
        AXIS_RTRIGGER = 18,
        AXIS_THROTTLE = 19,
        AXIS_RUDDER = 20,
        AXIS_WHEEL  = 21,
        AXIS_GAS = 22,
        AXIS_BRAKE = 23,
        AXIS_DISTANCE = 24,
        AXIS_TILT = 25,
        AXIS_SCROLL = 26,
        AXIS_RELATIVE_X = 27,
        AXIS_RELATIVE_Y = 28,
        AXIS_GENERIC_1 = 32,
        AXIS_GENERIC_2 = 33,
        AXIS_GENERIC_3 = 34,
        AXIS_GENERIC_4 = 35,
        AXIS_GENERIC_5 = 36,
        AXIS_GENERIC_6 = 37,
        AXIS_GENERIC_7 = 38,
        AXIS_GENERIC_8 = 39,
        AXIS_GENERIC_9 = 40,
        AXIS_GENERIC_10 = 41,
        AXIS_GENERIC_11 = 42,
        AXIS_GENERIC_12 = 43,
        AXIS_GENERIC_13 = 44,
        AXIS_GENERIC_14 = 45,
        AXIS_GENERIC_15 = 46,
        AXIS_GENERIC_16 = 47
    };
    enum{
        BUTTON_PRIMARY = 1 << 0,
        BUTTON_SECONDARY = 1 << 1,
        BUTTON_TERTIARY = 1 << 2,
        BUTTON_BACK = 1 << 3,
        BUTTON_FORWARD = 1 << 4,
        BUTTON_STYLUS_PRIMARY = 1 << 5,
        BUTTON_STYLUS_SECONDARY = 1 << 6
    };
    enum{
        FLAG_WINDOW_IS_OBSCURED =0x01,
        FLAG_WINDOW_IS_PARTIALLY_OBSCURED = 0x2,
        FLAG_HOVER_EXIT_PENDING = 0x4,
        FLAG_IS_GENERATED_GESTURE = 0x8,
        FLAG_TAINTED = 0x80000000,
        FLAG_TARGET_ACCESSIBILITY_FOCUS = 0x40000000,
        CLASSIFICATION_NONE = 0,
        CLASSIFICATION_AMBIGUOUS_GESTURE = 1,
        CLASSIFICATION_DEEP_PRESS = 2,
        CLASSIFICATION_TWO_FINGER_SWIPE = 3,
        CLASSIFICATION_MULTI_FINGER_SWIPE = 4,
        CLASSIFICATION_PINCH = 5,
    };
    enum{
        TOOL_TYPE_UNKNOWN = 0,
        TOOL_TYPE_FINGER = 1,
        TOOL_TYPE_STYLUS = 2,
        TOOL_TYPE_MOUSE = 3,
        TOOL_TYPE_ERASER = 4
    };
private:
    static constexpr float INVALID_CURSOR_POSITION = NAN;//std::numeric_limits<float>::quiet_NaN();
    static constexpr int HISTORY_CURRENT = 0x80000000;
    static MotionEvent*obtain();
    static void ensureSharedTempPointerCapacity(size_t desiredCapacity);
protected:
    int32_t mActionButton;
    int32_t mFlags;
    int32_t mEdgeFlags;
    int32_t mMetaState;
    int32_t mButtonState;
    int32_t mClassification;
    float mXOffset;
    float mYOffset;
    float mXPrecision;
    float mYPrecision;
    float mCursorXPosition;
    float mCursorYPosition;
    nsecs_t mDownTime;
    std::vector<PointerProperties> mPointerProperties;
    std::vector< nsecs_t > mSampleEventTimes;
    std::vector<PointerCoords> mSamplePointerCoords;
public:
    MotionEvent();
    MotionEvent(const MotionEvent&m);
    MotionEvent*copy()const override{return obtain(*this);}
    void initialize(int deviceId,int source,int displayId,int action,int actionButton,
        int flags, int edgeFlags,int metaState, int buttonState, float xOffset, float yOffset,
	    float xPrecision, float yPrecision,nsecs_t downTime, nsecs_t eventTime, size_t pointerCount,
        const PointerProperties* pointerProperties,const PointerCoords* pointerCoords);

    static MotionEvent*obtain(nsecs_t downTime, nsecs_t eventTime, int action,
        int pointerCount, const PointerProperties* pointerProperties,const PointerCoords* pointerCoords,
        int metaState, int buttonState, float xPrecision, float yPrecision, int deviceId,
        int edgeFlags, int source, int flags);

    static MotionEvent* obtain(nsecs_t downTime, nsecs_t eventTime, int action,
        float x, float y, float pressure, float size, int metaState,
        float xPrecision, float yPrecision, int deviceId, int edgeFlags);

    static MotionEvent* obtain(nsecs_t downTime, nsecs_t eventTime, int action, float x, float y, int metaState);
    static MotionEvent* obtain(const MotionEvent& other);
    static MotionEvent* obtainNoHistory(const MotionEvent& other);
    static bool isTouchEvent(int32_t source, int32_t action);
    void copyFrom(const MotionEvent& other, bool keepHistory);
    MotionEvent*split(int idBits);
    void setSource(int)override;
    int getType()const override{return INPUT_EVENT_TYPE_MOTION;}
    inline void setAction(int32_t action) { mAction = action; }
    inline int32_t getActionMasked() const { return mAction &ACTION_MASK; }
    int getActionIndex()const{
        return (mAction & ACTION_POINTER_INDEX_MASK) >>  ACTION_POINTER_INDEX_SHIFT;
    }
    bool isTouchEvent()const;
    inline nsecs_t getDownTime()const{return mDownTime;}
    inline void setDownTime(nsecs_t downTime){mDownTime = downTime;}
    inline size_t getPointerCount() const { return mPointerProperties.size(); }
    inline int32_t getFlags() const { return mFlags; }
    inline void setFlags(int32_t flags) { mFlags = flags; }
    inline bool isTainted()const override{return (mFlags&FLAG_TAINTED)!=0;}
    inline void setTainted(bool tainted)override{
        if(tainted)mFlags|=FLAG_TAINTED;
        else mFlags&=~FLAG_TAINTED;
    }
    inline int32_t getEdgeFlags() const { return mEdgeFlags; }
    inline void setEdgeFlags(int32_t edgeFlags) { mEdgeFlags = edgeFlags; }
    inline bool isHoverExitPending()const{return (getFlags()&FLAG_HOVER_EXIT_PENDING)!=0;}
    inline void setHoverExitPending(bool hoverExitPending){
        if(hoverExitPending)mFlags|=FLAG_HOVER_EXIT_PENDING;
        else mFlags&=~FLAG_HOVER_EXIT_PENDING;
    }
    inline int32_t getMetaState() const { return mMetaState; }
    inline void setMetaState(int32_t metaState) { mMetaState = metaState; }
    inline int32_t getButtonState() const { return mButtonState; }
    inline void setButtonState(int32_t buttonState) { mButtonState = buttonState; }
    bool isButtonPressed(int button)const;
    int32_t  getClassification()const{return mClassification;}
    inline int32_t getActionButton() const { return mActionButton; }
    inline void setActionButton(int32_t button) { mActionButton = button; }
    inline float getXOffset() const { return mXOffset; }
    inline float getYOffset() const { return mYOffset; }
    inline float getXPrecision() const { return mXPrecision; }
    inline float getYPrecision() const { return mYPrecision; }
    inline size_t getHistorySize() const { return mSampleEventTimes.size() - 1; }

    nsecs_t getHistoricalEventTime(size_t historicalIndex) const;
    nsecs_t getHistoricalEventTimeNanos(size_t historicalIndex) const;
    void cancel();
    float getXDispatchLocation(int pointerIndex);
    float getYDispatchLocation(int pointerIndex);
    float getXCursorPosition()const;
    float getYCursorPosition()const;
private:
    void updateCursorPosition();
    ////////////////////////////////// Raw AXIS Properties ///////////////////////////////////
    const PointerCoords& getRawPointerCoords(size_t pointerIndex) const;
    int getHistoricalRawPointerCoords(size_t pointerIndex, size_t historicalIndex,PointerCoords&) const;
    float getHistoricalRawAxisValue(int32_t axis, size_t pointerIndex,size_t historicalIndex) const;
    float getHistoricalRawX(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalRawY(size_t pointerIndex, size_t historicalIndex) const;
    float getRawAxisValue(int32_t axis, size_t pointerIndex) const;
    inline float getRawX(size_t pointerIndex) const { return getRawAxisValue(AXIS_X, pointerIndex); }
    inline float getRawY(size_t pointerIndex) const { return getRawAxisValue(AXIS_Y, pointerIndex); }
public:
    void setCursorPosition(float x, float y);
    /////////////////////// AXIS Properties has been transformed //////////////////////////////
    int getPointerCoords(int pointerIndex,PointerCoords&)const;
    int getHistoricalPointerCoords(size_t pointerIndex, size_t historicalIndex,PointerCoords&) const;
    float getHistoricalAxisValue(int axis, size_t pointerIndex,size_t historicalIndex) const;
    float getHistoricalX(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalY(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalPressure(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalSize(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalTouchMajor(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalTouchMinor(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalToolMajor(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalToolMinor(size_t pointerIndex, size_t historicalIndex) const;
    float getHistoricalOrientation(size_t pointerIndex, size_t historicalIndex) const;
    float getAxisValue(int32_t axis)const;
    float getAxisValue(int32_t axis, size_t pointerIndex)const;
    float getX(size_t pointerIndex=0) const { return getAxisValue(AXIS_X,pointerIndex); }
    float getY(size_t pointerIndex=0) const { return getAxisValue(AXIS_Y,pointerIndex); }

    //////////////////////////////////////////////////////////////////////////////////////////    
    int getPointerProperties(size_t pointerIndex,PointerProperties&pp) const;
    void addSample(nsecs_t eventTime,const PointerCoords*);
    void offsetLocation(float xOffset, float yOffset);
    void setLocation(float x,float y);
    void scale(float scaleFactor);

    // Apply 3x3 perspective matrix transformation.
    // Matrix is in row-major form and compatible with SkMatrix.
    void transform(const float matrix[9]);
    void transform(const Cairo::Matrix & matrix);
    void addBatch(nsecs_t eventTime, float x, float y, float pressure, float size, int metaState);
    void addBatch(nsecs_t eventTime,const std::vector<PointerCoords>& pointerCoords, int metaState);
    bool addBatch(const MotionEvent& event);
    inline int32_t getPointerId(size_t pointerIndex) const {
        return mPointerProperties[pointerIndex].id;
    }
    MotionEvent*clampNoHistory(float left, float top, float right, float bottom);
    int getPointerIdBits()const;
    inline int32_t getToolType(size_t pointerIndex) const {
        return mPointerProperties[pointerIndex].toolType;
    }
    inline nsecs_t getEventTime() const override{ return mSampleEventTimes[getHistorySize()]; }
    inline float getPressure(size_t pointerIndex) const {
        return getAxisValue(AXIS_PRESSURE, pointerIndex);
    }
    inline float getSize(size_t pointerIndex) const {
        return getAxisValue(AXIS_SIZE, pointerIndex);
    }
    inline float getTouchMajor(size_t pointerIndex) const {
        return getAxisValue(AXIS_TOUCH_MAJOR, pointerIndex);
    }
    inline float getTouchMinor(size_t pointerIndex) const {
        return getAxisValue(AXIS_TOUCH_MINOR, pointerIndex);
    }
    inline float getToolMajor(size_t pointerIndex) const {
        return getAxisValue(AXIS_TOOL_MAJOR, pointerIndex);
    }
    inline float getToolMinor(size_t pointerIndex) const {
        return getAxisValue(AXIS_TOOL_MINOR, pointerIndex);
    }
    inline float getOrientation(size_t pointerIndex) const {
        return getAxisValue(AXIS_ORIENTATION, pointerIndex);
    }
    bool isResampled(size_t pointerIndex, size_t historicalIndex) const;
    int32_t findPointerIndex(int32_t pointerId) const;
    inline bool isTargetAccessibilityFocus()const{
        return (mFlags & FLAG_TARGET_ACCESSIBILITY_FOCUS) != 0;
    }
    inline void setTargetAccessibilityFocus(bool targetsFocus){
        if(targetsFocus)
            mFlags|=FLAG_TARGET_ACCESSIBILITY_FOCUS;
        else
            mFlags&=~FLAG_TARGET_ACCESSIBILITY_FOCUS;
    }
    static std::string actionToString(int action);
    static std::string axisToString(int axis);
    static int axisFromString(const std::string&symbolicName);
    static std::string buttonStateToString(int buttonState);
    static std::string toolTypeToString(int toolType);
    void toStream(std::ostream& os)const override;
};
}/*endof namespace*/
#endif/*__MOTION_EVENT_H__*/
