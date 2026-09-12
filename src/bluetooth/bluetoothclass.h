#ifndef __CDROID_BLUETOOTH_CLASS_H__
#define __CDROID_BLUETOOTH_CLASS_H__

#include <cstdint>

namespace cdroid {

/**
 * Port of android.bluetooth.BluetoothClass (android-36): the 24-bit
 * Bluetooth Class of Device (CoD) — service classes, major and minor
 * device class components. A value type built from the CoD the stack
 * reports per remote device (BlueZ Device1 "Class", big-endian as the
 * spec encodes it, same integer the Java side receives).
 *
 * The accessors keep AOSP's masking semantics verbatim:
 *   hasService()          -> Service.BITMASK      (0xFFE000)
 *   getMajorDeviceClass() -> Device.Major.BITMASK (0x1F00)
 *   getDeviceClass()      -> Device.BITMASK       (0x1FFC)
 */
class BluetoothClass {
public:
    static constexpr int ERROR = 0xFF000000;

    /** Service class bit constants (BluetoothClass.Service). */
    class Service {
    public:
        static constexpr int BITMASK = 0xFFE000;
        static constexpr int LIMITED_DISCOVERABILITY = 0x002000;
        static constexpr int LE_AUDIO = 0x004000;
        static constexpr int POSITIONING = 0x010000;
        static constexpr int NETWORKING = 0x020000;
        static constexpr int RENDER = 0x040000;
        static constexpr int CAPTURE = 0x080000;
        static constexpr int OBJECT_TRANSFER = 0x100000;
        static constexpr int AUDIO = 0x200000;
        static constexpr int TELEPHONY = 0x400000;
        static constexpr int INFORMATION = 0x800000;
    };

    /** Device class constants — major + minor (BluetoothClass.Device). */
    class Device {
    public:
        static constexpr int BITMASK = 0x1FFC;

        /** Major device class component (BluetoothClass.Device.Major). */
        class Major {
        public:
            static constexpr int BITMASK = 0x1F00;
            static constexpr int MISC = 0x0000;
            static constexpr int COMPUTER = 0x0100;
            static constexpr int PHONE = 0x0200;
            static constexpr int NETWORKING = 0x0300;
            static constexpr int AUDIO_VIDEO = 0x0400;
            static constexpr int PERIPHERAL = 0x0500;
            static constexpr int IMAGING = 0x0600;
            static constexpr int WEARABLE = 0x0700;
            static constexpr int TOY = 0x0800;
            static constexpr int HEALTH = 0x0900;
            static constexpr int UNCATEGORIZED = 0x1F00;
        };

        // Devices in the COMPUTER major class
        static constexpr int COMPUTER_UNCATEGORIZED = 0x0100;
        static constexpr int COMPUTER_DESKTOP = 0x0104;
        static constexpr int COMPUTER_SERVER = 0x0108;
        static constexpr int COMPUTER_LAPTOP = 0x010C;
        static constexpr int COMPUTER_HANDHELD_PC_PDA = 0x0110;
        static constexpr int COMPUTER_PALM_SIZE_PC_PDA = 0x0114;
        static constexpr int COMPUTER_WEARABLE = 0x0118;

        // Devices in the PHONE major class
        static constexpr int PHONE_UNCATEGORIZED = 0x0200;
        static constexpr int PHONE_CELLULAR = 0x0204;
        static constexpr int PHONE_CORDLESS = 0x0208;
        static constexpr int PHONE_SMART = 0x020C;
        static constexpr int PHONE_MODEM_OR_GATEWAY = 0x0210;
        static constexpr int PHONE_ISDN = 0x0214;

        // Minor classes for the AUDIO_VIDEO major class
        static constexpr int AUDIO_VIDEO_UNCATEGORIZED = 0x0400;
        static constexpr int AUDIO_VIDEO_WEARABLE_HEADSET = 0x0404;
        static constexpr int AUDIO_VIDEO_HANDSFREE = 0x0408;
        static constexpr int AUDIO_VIDEO_MICROPHONE = 0x0410;
        static constexpr int AUDIO_VIDEO_LOUDSPEAKER = 0x0414;
        static constexpr int AUDIO_VIDEO_HEADPHONES = 0x0418;
        static constexpr int AUDIO_VIDEO_PORTABLE_AUDIO = 0x041C;
        static constexpr int AUDIO_VIDEO_CAR_AUDIO = 0x0420;
        static constexpr int AUDIO_VIDEO_SET_TOP_BOX = 0x0424;
        static constexpr int AUDIO_VIDEO_HIFI_AUDIO = 0x0428;
        static constexpr int AUDIO_VIDEO_VCR = 0x042C;
        static constexpr int AUDIO_VIDEO_VIDEO_CAMERA = 0x0430;
        static constexpr int AUDIO_VIDEO_CAMCORDER = 0x0434;
        static constexpr int AUDIO_VIDEO_VIDEO_MONITOR = 0x0438;
        static constexpr int AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER = 0x043C;
        static constexpr int AUDIO_VIDEO_VIDEO_CONFERENCING = 0x0440;
        static constexpr int AUDIO_VIDEO_VIDEO_GAMING_TOY = 0x0448;

        // Devices in the WEARABLE major class
        static constexpr int WEARABLE_UNCATEGORIZED = 0x0700;
        static constexpr int WEARABLE_WRIST_WATCH = 0x0704;
        static constexpr int WEARABLE_PAGER = 0x0708;
        static constexpr int WEARABLE_JACKET = 0x070C;
        static constexpr int WEARABLE_HELMET = 0x0710;
        static constexpr int WEARABLE_GLASSES = 0x0714;

        // Devices in the TOY major class
        static constexpr int TOY_UNCATEGORIZED = 0x0800;
        static constexpr int TOY_ROBOT = 0x0804;
        static constexpr int TOY_VEHICLE = 0x0808;
        static constexpr int TOY_DOLL_ACTION_FIGURE = 0x080C;
        static constexpr int TOY_CONTROLLER = 0x0810;
        static constexpr int TOY_GAME = 0x0814;

        // Devices in the HEALTH major class
        static constexpr int HEALTH_UNCATEGORIZED = 0x0900;
        static constexpr int HEALTH_BLOOD_PRESSURE = 0x0904;
        static constexpr int HEALTH_THERMOMETER = 0x0908;
        static constexpr int HEALTH_WEIGHING = 0x090C;
        static constexpr int HEALTH_GLUCOSE = 0x0910;
        static constexpr int HEALTH_PULSE_OXIMETER = 0x0914;
        static constexpr int HEALTH_PULSE_RATE = 0x0918;
        static constexpr int HEALTH_DATA_DISPLAY = 0x091C;

        // Devices in PERIPHERAL major class
        static constexpr int PERIPHERAL_NON_KEYBOARD_NON_POINTING = 0x0500;
        static constexpr int PERIPHERAL_KEYBOARD = 0x0540;
        static constexpr int PERIPHERAL_POINTING = 0x0580;
        static constexpr int PERIPHERAL_KEYBOARD_POINTING = 0x05C0;
    };

    explicit BluetoothClass(int cod) : mClass(cod) {}

    /** True when the CoD carries the given service class bit(s). */
    bool hasService(int service) const {
        return (mClass & Service::BITMASK & service) != 0;
    }

    /** Major device class component — compare against Device::Major::*. */
    int getMajorDeviceClass() const {
        return mClass & Device::Major::BITMASK;
    }

    /** Combined major+minor device class — compare against Device::*. */
    int getDeviceClass() const {
        return mClass & Device::BITMASK;
    }

    int getClassOfDevice() const { return mClass; }

    bool operator==(const BluetoothClass& other) const { return mClass == other.mClass; }
    bool operator!=(const BluetoothClass& other) const { return mClass != other.mClass; }

private:
    const int mClass;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_CLASS_H__ */
