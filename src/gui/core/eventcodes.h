/* Input event codes
 *
 *    *** IMPORTANT ***
 * This file is not only included from C-code but also from devicetree source
 * files. As such this file MUST only contain comments and defines.
 */
#ifndef _INPUT_EVENT_CODES_H
#define _INPUT_EVENT_CODES_H
/*
 * Device properties and quirks
 */

#define INPUT_PROP_POINTER      0x00    /* needs a pointer */
#define INPUT_PROP_DIRECT       0x01    /* direct input devices */
#define INPUT_PROP_BUTTONPAD    0x02    /* has button(s) under pad */
#define INPUT_PROP_SEMI_MT      0x03    /* touch rectangle only */
#define INPUT_PROP_TOPBUTTONPAD 0x04    /* softbuttons at top of pad */
#define INPUT_PROP_POINTING_STICK   0x05    /* is a pointing stick */
#define INPUT_PROP_ACCELEROMETER    0x06    /* has accelerometer */

#define INPUT_PROP_MAX          0x1f
#define INPUT_PROP_CNT          (INPUT_PROP_MAX + 1)

/*
 * Event types
 */

#define EV_SYN          0x00
#define EV_KEY          0x01
#define EV_REL          0x02
#define EV_ABS          0x03
#define EV_MSC          0x04
#define EV_SW           0x05
#define EV_LED          0x11
#define EV_SND          0x12
#define EV_REP          0x14
#define EV_FF           0x15
#define EV_PWR          0x16
#define EV_FF_STATUS    0x17
#define EV_MAX          0x1f
#define EV_CNT          (EV_MAX+1)

#ifndef EV_ADD
    #define EV_ADD          0xFE/*added by zhhou,used by INPUTEVENT's type, for device add*/
    #define EV_REMOVE       0xFF/*added by zhhou,used by INPUTEVENT's type, for device remove*/
#endif
/*
 * Synchronization events.
 */

#define SYN_REPORT      0
#define SYN_CONFIG      1
#define SYN_MT_REPORT   2
#define SYN_DROPPED     3
#define SYN_MAX         0xf
#define SYN_CNT         (SYN_MAX+1)

/*
 * Keys and buttons
 *
 * Most of the keys/buttons are modeled after USB HUT 1.12
 * (see http://www.usb.org/developers/hidpage).
 * Abbreviations in the comments:
 * AC - Application Control
 * AL - Application Launch Button
 * SC - System Control
 */
/* Code 255 is reserved for special needs of AT keyboard driver */
#ifndef KEY_OK
    #define KEY_OK    0x160
#endif
#define KEY_LEFT      KEY_DPAD_LEFT
#define KEY_RIGHT     KEY_DPAD_RIGHT
#define KEY_UP        KEY_DPAD_UP
#define KEY_DOWN      KEY_DPAD_DOWN
#define KEY_PAGEUP    KEY_PAGE_UP
#define KEY_PAGEDOWN  KEY_PAGE_DOWN
#define KEY_DELETE    KEY_DEL
#define KEY_BACKSPACE KEY_BACK
#define KEY_VOLUMEUP KEY_VOLUME_UP
#define KEY_VOLUMEDOWN KEY_VOLUME_DOWN
#define KEY_RED       KEY_PROG_RED
#define KEY_GREEN     KEY_PROG_GREEN
#define KEY_YELLOW    KEY_PROG_YELLOW
#define KEY_BLUE      KEY_PROG_BLUE

#define BTN_MISC        0x100
#define BTN_0           0x100
#define BTN_1           0x101
#define BTN_2           0x102
#define BTN_3           0x103
#define BTN_4           0x104
#define BTN_5           0x105
#define BTN_6           0x106
#define BTN_7           0x107
#define BTN_8           0x108
#define BTN_9           0x109

#define BTN_MOUSE       0x110
#define BTN_LEFT        0x110
#define BTN_RIGHT       0x111
#define BTN_MIDDLE      0x112
#define BTN_SIDE        0x113
#define BTN_EXTRA       0x114
#define BTN_FORWARD     0x115
#define BTN_BACK        0x116
#define BTN_TASK        0x117

#define BTN_JOYSTICK    0x120
#define BTN_TRIGGER     0x120
#define BTN_THUMB       0x121
#define BTN_THUMB2      0x122
#define BTN_TOP         0x123
#define BTN_TOP2        0x124
#define BTN_PINKIE      0x125
#define BTN_BASE        0x126
#define BTN_BASE2       0x127
#define BTN_BASE3       0x128
#define BTN_BASE4       0x129
#define BTN_BASE5       0x12a
#define BTN_BASE6       0x12b
#define BTN_DEAD        0x12f

#define BTN_GAMEPAD     0x130
#define BTN_SOUTH       0x130
#define BTN_A           BTN_SOUTH
#define BTN_EAST        0x131
#define BTN_B           BTN_EAST
#define BTN_C           0x132
#define BTN_NORTH       0x133
#define BTN_X           BTN_NORTH
#define BTN_WEST        0x134
#define BTN_Y           BTN_WEST
#define BTN_Z           0x135
#define BTN_TL          0x136
#define BTN_TR          0x137
#define BTN_TL2         0x138
#define BTN_TR2         0x139
#define BTN_SELECT      0x13a
#define BTN_START       0x13b
#define BTN_MODE        0x13c
#define BTN_THUMBL      0x13d
#define BTN_THUMBR      0x13e

#define BTN_DIGI            0x140
#define BTN_TOOL_PEN        0x140
#define BTN_TOOL_RUBBER     0x141
#define BTN_TOOL_BRUSH      0x142
#define BTN_TOOL_PENCIL     0x143
#define BTN_TOOL_AIRBRUSH   0x144
#define BTN_TOOL_FINGER     0x145
#define BTN_TOOL_MOUSE      0x146
#define BTN_TOOL_LENS       0x147
#define BTN_TOOL_QUINTTAP   0x148   /* Five fingers on trackpad */
#define BTN_TOUCH           0x14a
#define BTN_STYLUS          0x14b
#define BTN_STYLUS2         0x14c
#define BTN_TOOL_DOUBLETAP  0x14d
#define BTN_TOOL_TRIPLETAP  0x14e
#define BTN_TOOL_QUADTAP    0x14f   /* Four fingers on trackpad */

#define BTN_WHEEL       0x150
#define BTN_GEAR_DOWN   0x150
#define BTN_GEAR_UP     0x151

#if 0
#endif

#define BTN_TRIGGER_HAPPY       0x2c0
#define BTN_TRIGGER_HAPPY1      0x2c0
#define BTN_TRIGGER_HAPPY2      0x2c1
#define BTN_TRIGGER_HAPPY3      0x2c2
#define BTN_TRIGGER_HAPPY4      0x2c3
#define BTN_TRIGGER_HAPPY5      0x2c4
#define BTN_TRIGGER_HAPPY6      0x2c5
#define BTN_TRIGGER_HAPPY7      0x2c6
#define BTN_TRIGGER_HAPPY8      0x2c7
#define BTN_TRIGGER_HAPPY9      0x2c8
#define BTN_TRIGGER_HAPPY10     0x2c9
#define BTN_TRIGGER_HAPPY11     0x2ca
#define BTN_TRIGGER_HAPPY12     0x2cb
#define BTN_TRIGGER_HAPPY13     0x2cc
#define BTN_TRIGGER_HAPPY14     0x2cd
#define BTN_TRIGGER_HAPPY15     0x2ce
#define BTN_TRIGGER_HAPPY16     0x2cf
#define BTN_TRIGGER_HAPPY17     0x2d0
#define BTN_TRIGGER_HAPPY18     0x2d1
#define BTN_TRIGGER_HAPPY19     0x2d2
#define BTN_TRIGGER_HAPPY20     0x2d3
#define BTN_TRIGGER_HAPPY21     0x2d4
#define BTN_TRIGGER_HAPPY22     0x2d5
#define BTN_TRIGGER_HAPPY23     0x2d6
#define BTN_TRIGGER_HAPPY24     0x2d7
#define BTN_TRIGGER_HAPPY25     0x2d8
#define BTN_TRIGGER_HAPPY26     0x2d9
#define BTN_TRIGGER_HAPPY27     0x2da
#define BTN_TRIGGER_HAPPY28     0x2db
#define BTN_TRIGGER_HAPPY29     0x2dc
#define BTN_TRIGGER_HAPPY30     0x2dd
#define BTN_TRIGGER_HAPPY31     0x2de
#define BTN_TRIGGER_HAPPY32     0x2df
#define BTN_TRIGGER_HAPPY33     0x2e0
#define BTN_TRIGGER_HAPPY34     0x2e1
#define BTN_TRIGGER_HAPPY35     0x2e2
#define BTN_TRIGGER_HAPPY36     0x2e3
#define BTN_TRIGGER_HAPPY37     0x2e4
#define BTN_TRIGGER_HAPPY38     0x2e5
#define BTN_TRIGGER_HAPPY39     0x2e6
#define BTN_TRIGGER_HAPPY40     0x2e7

/* We avoid low common keys in module aliases so they don't get huge. */
#define KEY_MIN_INTERESTING KEY_MUTE
#define KEY_MAX         0x2ff
#define KEY_CNT         (KEY_MAX+1)

/*
 * Relative axes
 */

#define REL_X           0x00
#define REL_Y           0x01
#define REL_Z           0x02
#define REL_RX          0x03
#define REL_RY          0x04
#define REL_RZ          0x05
#define REL_HWHEEL      0x06
#define REL_DIAL        0x07
#define REL_WHEEL       0x08
#define REL_MISC        0x09
#define REL_MAX         0x0f
#define REL_CNT         (REL_MAX+1)

/*
 * Absolute axes
 */

#define ABS_X           0x00
#define ABS_Y           0x01
#define ABS_Z           0x02
#define ABS_RX          0x03
#define ABS_RY          0x04
#define ABS_RZ          0x05
#define ABS_THROTTLE    0x06
#define ABS_RUDDER      0x07
#define ABS_WHEEL       0x08
#define ABS_GAS         0x09
#define ABS_BRAKE       0x0a
#define ABS_HAT0X       0x10
#define ABS_HAT0Y       0x11
#define ABS_HAT1X       0x12
#define ABS_HAT1Y       0x13
#define ABS_HAT2X       0x14
#define ABS_HAT2Y       0x15
#define ABS_HAT3X       0x16
#define ABS_HAT3Y       0x17
#define ABS_PRESSURE    0x18
#define ABS_DISTANCE    0x19
#define ABS_TILT_X      0x1a
#define ABS_TILT_Y      0x1b
#define ABS_TOOL_WIDTH  0x1c

#define ABS_VOLUME      0x20

#define ABS_MISC        0x28

/*
 * 0x2e is reserved and should not be used in input drivers.
 * It was used by HID as ABS_MISC+6 and userspace needs to detect if
 * the next ABS_* event is correct or is just ABS_MISC + n.
 * We define here ABS_RESERVED so userspace can rely on it and detect
 * the situation described above.
 */
#define ABS_RESERVED        0x2e

#define ABS_MT_SLOT         0x2f    /* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR  0x30    /* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR  0x31    /* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR  0x32    /* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR  0x33    /* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION  0x34    /* Ellipse orientation */
#define ABS_MT_POSITION_X   0x35    /* Center X touch position */
#define ABS_MT_POSITION_Y   0x36    /* Center Y touch position */
#define ABS_MT_TOOL_TYPE    0x37    /* Type of touching device */
#define ABS_MT_BLOB_ID      0x38    /* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID  0x39    /* Unique ID of initiated contact */
#define ABS_MT_PRESSURE     0x3a    /* Pressure on contact area */
#define ABS_MT_DISTANCE     0x3b    /* Contact hover distance */
#define ABS_MT_TOOL_X       0x3c    /* Center X tool position */
#define ABS_MT_TOOL_Y       0x3d    /* Center Y tool position */


#define ABS_MAX             0x3f
#define ABS_CNT         (ABS_MAX+1)

/*
 * Switch events
 */

#define SW_LID               0x00  /* set = lid shut */
#define SW_TABLET_MODE       0x01  /* set = tablet mode */
#define SW_HEADPHONE_INSERT  0x02  /* set = inserted */
#define SW_RFKILL_ALL        0x03  /* rfkill master switch, type "any"
                     set = radio enabled */
#define SW_RADIO        SW_RFKILL_ALL   /* deprecated */
#define SW_MICROPHONE_INSERT 0x04  /* set = inserted */
#define SW_DOCK              0x05  /* set = plugged into dock */
#define SW_LINEOUT_INSERT    0x06  /* set = inserted */
#define SW_JACK_PHYSICAL_INSERT 0x07  /* set = mechanical switch set */
#define SW_VIDEOOUT_INSERT   0x08  /* set = inserted */
#define SW_CAMERA_LENS_COVER 0x09  /* set = lens covered */
#define SW_KEYPAD_SLIDE      0x0a  /* set = keypad slide out */
#define SW_FRONT_PROXIMITY   0x0b  /* set = front proximity sensor active */
#define SW_ROTATE_LOCK       0x0c  /* set = rotate locked/disabled */
#define SW_LINEIN_INSERT     0x0d  /* set = inserted */
#define SW_MUTE_DEVICE       0x0e  /* set = device disabled */
#define SW_MAX               0x0f
#define SW_CNT          (SW_MAX+1)

/*
 * Misc events
 */

#define MSC_SERIAL      0x00
#define MSC_PULSELED    0x01
#define MSC_GESTURE     0x02
#define MSC_RAW         0x03
#define MSC_SCAN        0x04
#define MSC_TIMESTAMP   0x05
#define MSC_MAX         0x07
#define MSC_CNT         (MSC_MAX+1)

/*
 * LEDs
 */

#define LED_NUM_LOCK    0x00
#define LED_CAPS_LOCK   0x01
#define LED_SCROLL_LOCK 0x02
#define LED_COMPOSE     0x03
#define LED_KANA        0x04
#define LED_SLEEP       0x05
#define LED_SUSPEND     0x06
#define LED_MUTE        0x07
#define LED_MISC        0x08
#define LED_MAIL        0x09
#define LED_CHARGING    0x0a
#define LED_MAX         0x0f
#define LED_CNT         (LED_MAX+1)

/*
 * Autorepeat values
 */

#define REP_DELAY       0x00
#define REP_PERIOD      0x01
#define REP_MAX         0x01
#define REP_CNT         (REP_MAX+1)

/*
 * Sounds
 */

#define SND_CLICK       0x00
#define SND_BELL        0x01
#define SND_TONE        0x02
#define SND_MAX         0x07
#define SND_CNT         (SND_MAX+1)

/*
 * Force feedback effect types
 */

#define FF_RUMBLE       0x50
#define FF_PERIODIC     0x51
#define FF_CONSTANT     0x52
#define FF_SPRING       0x53
#define FF_FRICTION     0x54
#define FF_DAMPER       0x55
#define FF_INERTIA      0x56
#define FF_RAMP         0x57

#define FF_EFFECT_MIN   FF_RUMBLE
#define FF_EFFECT_MAX   FF_RAMP

/*
 * Force feedback periodic effect types
 */

#define FF_SQUARE       0x58
#define FF_TRIANGLE     0x59
#define FF_SINE         0x5a
#define FF_SAW_UP       0x5b
#define FF_SAW_DOWN     0x5c
#define FF_CUSTOM       0x5d

#define FF_WAVEFORM_MIN FF_SQUARE
#define FF_WAVEFORM_MAX FF_CUSTOM

/*
 * Set ff device properties
 */

#define FF_GAIN         0x60
#define FF_AUTOCENTER   0x61

#define FF_MAX          0x7f
#define FF_CNT          (FF_MAX+1)

#endif
