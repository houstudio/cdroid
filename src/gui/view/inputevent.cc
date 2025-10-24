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
#include <view/inputevent.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/inputdevice.h>
#include <utils/atexit.h>
#include <utils/neverdestroyed.h>
#include <porting/cdlog.h>
#include <fcntl.h>
#include <unistd.h>
// --- InputEvent ---
namespace cdroid{

int InputEvent::mNextSeq=0;

InputEvent::InputEvent(){
   mSource = InputDevice::SOURCE_UNKNOWN;
   mDisplayId = 0;
   mSeq = mNextSeq++;
}

InputEvent::~InputEvent(){
}

void InputEvent::prepareForReuse(){
   mSeq = mNextSeq++;
}

int InputEvent::getDisplayId()const{
    return mDisplayId;
}

void InputEvent::setDisplayId(int id){
    mDisplayId = id;
}

int32_t InputEvent::nextId(){
    static IdGenerator idGen(IdGenerator::Source::OTHER);
    return idGen.nextId();
}

void InputEvent::initialize(int32_t id,int32_t deviceId, uint32_t source,int32_t displayId) {
    mId = id;
    mDeviceId = deviceId;
    mSource = source;
    mDisplayId = displayId;
}

void InputEvent::setSource(uint32_t source){
    mSource = source;
}

bool InputEvent::isFromSource(uint32_t source)const{
    return (getSource() & source) == source;
}

std::string InputEvent::sourceToString(uint32_t source) {
    if (source == InputDevice::SOURCE_UNKNOWN) {
        return "UNKNOWN";
    }
    if (source == static_cast<int32_t>(InputDevice::SOURCE_ANY)) {
        return "ANY";
    }
    static const std::map<int32_t, const char*> SOURCES{
            {InputDevice::SOURCE_KEYBOARD, "KEYBOARD"},
            {InputDevice::SOURCE_DPAD, "DPAD"},
            {InputDevice::SOURCE_GAMEPAD, "GAMEPAD"},
            {InputDevice::SOURCE_TOUCHSCREEN, "TOUCHSCREEN"},
            {InputDevice::SOURCE_MOUSE, "MOUSE"},
            {InputDevice::SOURCE_STYLUS, "STYLUS"},
            {InputDevice::SOURCE_BLUETOOTH_STYLUS, "BLUETOOTH_STYLUS"},
            {InputDevice::SOURCE_TRACKBALL, "TRACKBALL"},
            {InputDevice::SOURCE_MOUSE_RELATIVE, "MOUSE_RELATIVE"},
            {InputDevice::SOURCE_TOUCHPAD, "TOUCHPAD"},
            {InputDevice::SOURCE_TOUCH_NAVIGATION, "TOUCH_NAVIGATION"},
            {InputDevice::SOURCE_JOYSTICK, "JOYSTICK"},
            {InputDevice::SOURCE_HDMI, "HDMI"},
            //{InputDevice::SOURCE_SENSOR, "SENSOR"},
            {InputDevice::SOURCE_ROTARY_ENCODER, "ROTARY_ENCODER"},
    };
    std::string result;
    for (const auto& it/*[source_entry, str]*/ : SOURCES) {
        if ((source & it.first) == it.first) {
            if (!result.empty()) {
                result += " | ";
            }
            result += it.second;
        }
    }
    if (result.empty()) {
        //result = StringPrintf("0x%08x", source);
    }
    return result;
}

void InputEvent::initialize(const InputEvent& from) {
    mDeviceId = from.mDeviceId;
    mSource = from.mSource;
    mId = from.mId;
    mDisplayId = from.mDisplayId;
}

void InputEvent::recycle(){
    PooledInputEventFactory::getInstance().recycle(this);
}

std::ostream& operator<<(std::ostream& out, const InputEvent& event) {
    switch (event.getType()) {
    case InputEvent::INPUT_EVENT_TYPE_KEY:
        out << static_cast<const KeyEvent&>(event);
        break;
    case InputEvent::INPUT_EVENT_TYPE_MOTION:
        out << static_cast<const MotionEvent&>(event);
        break;
    case InputEvent::INPUT_EVENT_TYPE_FOCUS:
        out << "FocusEvent";
        break;
    case InputEvent::INPUT_EVENT_TYPE_CAPTURE:
        out << "CaptureEvent";
        break;
    case InputEvent::INPUT_EVENT_TYPE_DRAG:
        out << "DragEvent";
        break;
    case InputEvent::INPUT_EVENT_TYPE_TOUCH_MODE:
        out << "TouchModeEvent";
        break;
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// --- PooledInputEventFactory ---
static NeverDestroyed<PooledInputEventFactory>mInst(20);
PooledInputEventFactory& PooledInputEventFactory::getInstance(){
    return *mInst.get();
}

PooledInputEventFactory::PooledInputEventFactory(size_t maxPoolSize)
    :mMaxPoolSize(maxPoolSize) {
}

PooledInputEventFactory::~PooledInputEventFactory() {
    while(!mKeyEventPool.empty()) {
        delete mKeyEventPool.front();
        mKeyEventPool.pop();
    }
    while(!mMotionEventPool.empty()) {
        delete mMotionEventPool.front();
        mMotionEventPool.pop();
    }
}

KeyEvent* PooledInputEventFactory::createKeyEvent() {
    if (mKeyEventPool.size()) {
        KeyEvent* event = mKeyEventPool.front();
        mKeyEventPool.pop();
        return event;
    }
    return new KeyEvent();
}

MotionEvent* PooledInputEventFactory::createMotionEvent() {
    if (mMotionEventPool.size()) {
        MotionEvent* event = mMotionEventPool.front();
        mMotionEventPool.pop();
        return event;
    }
    return new MotionEvent();
}

void PooledInputEventFactory::recycle(InputEvent* event) {
    switch (event->getType()) {
    case InputEvent::INPUT_EVENT_TYPE_KEY:
        if (mKeyEventPool.size() < mMaxPoolSize) {
            mKeyEventPool.push(static_cast<KeyEvent*>(event));
            return;
        }
        LOGD("outOf KeyPool %d/%d",mKeyEventPool.size(),mMaxPoolSize);
        break;
    case InputEvent::INPUT_EVENT_TYPE_MOTION:
        if (mMotionEventPool.size() < mMaxPoolSize) {
            mMotionEventPool.push(static_cast<MotionEvent*>(event));
            return;
        }
        LOGD("outOf MotionPool %d/%d",mMotionEventPool.size(),mMaxPoolSize);
        break;
    }
    delete event;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// --- IdGenerator ---
static bool ReadFully(int fd, void* data, size_t byte_count) {
    uint8_t* p = reinterpret_cast<uint8_t*>(data);
    size_t remaining = byte_count;
    while (remaining > 0) {
        ssize_t n = TEMP_FAILURE_RETRY(read(fd, p, remaining));
        if (n == 0) {  // EOF
            errno = ENODATA;
            return false;
        }
        if (n == -1) return false;
        p += n;
        remaining -= n;
    }
    return true;
}
#if defined(__ANDROID__)
[[maybe_unused]]
#endif
static int getRandomBytes(uint8_t* data, size_t size) {
    int fd = TEMP_FAILURE_RETRY(open("/dev/urandom", O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (fd == -1) {
        return -errno;
    }

    if (!ReadFully(fd, data, size)) {
        return -errno;
    }
    return 0;
}

IdGenerator::IdGenerator(Source source) : mSource(source) {}

int32_t IdGenerator::nextId() const {
    constexpr uint32_t SEQUENCE_NUMBER_MASK = ~SOURCE_MASK;
    int32_t id = 0;

#if defined(__ANDROID__)
    // On device, prefer 'getrandom' to '/dev/urandom' because it's faster.
    constexpr size_t BUF_LEN = sizeof(id);
    size_t totalBytes = 0;
    while (totalBytes < BUF_LEN) {
        ssize_t bytes = TEMP_FAILURE_RETRY(getrandom(&id, BUF_LEN, GRND_NONBLOCK));
        if (CC_UNLIKELY(bytes < 0)) {
            LOGW("Failed to fill in random number for sequence number: %s.", strerror(errno));
            id = 0;
            break;
        }
        totalBytes += bytes;
    }
#else
#if defined(__linux__)
    // On host, <sys/random.h> / GRND_NONBLOCK is not available
    while (true) {
        int result = getRandomBytes(reinterpret_cast<uint8_t*>(&id), sizeof(id));
        if (result == 0/*OK*/) {
            break;
        }
    }
#endif // __linux__
#endif // __ANDROID__
    return (id & SEQUENCE_NUMBER_MASK) | static_cast<int32_t>(mSource);
}
}/*endof namespace*/
