#ifndef __CONFIGURTATION_H__
#define __CONFIGURTATION_H__
#include <string>
#include <vector>
#include <core/parcel.h>
#include <core/displaymetrics.h>
namespace cdroid{
/**
 * This class describes all device configuration information that can
 * impact the resources the application retrieves.  This includes both
 * user-specified configuration options (locale list and scaling) as well
 * as device configurations (such as input modes, screen size and screen orientation).
 * <p>You can acquire this object from {@link Resources}, using {@link
 * Resources#getConfiguration}. Thus, from an activity, you can get it by chaining the request
 * with {@link android.app.Activity#getResources}:</p>
 * <pre>Configuration config = getResources().getConfiguration();</pre>
 */
class Configuration final{
private:
    /**
     * Current user preference for the locale, corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#LocaleQualifier">locale</a>
     * resource qualifier.
     *
     * @deprecated Do not set or read this directly. Use {@link #getLocales()} and
     * {@link #setLocales(LocaleList)}. If only the primary locale is needed,
     * <code>getLocales().get(0)</code> is now the preferred accessor.
     */
    //@Deprecated public Locale locale;
    //LocaleList mLocaleList;
public:
	//static Configuration EMPTY = new Configuration();

    /**
     * Current user preference for the scaling factor for fonts, relative
     * to the base density scaling.
     */
    float fontScale;

    /**
     * IMSI MCC (Mobile Country Code), corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#MccQualifier">mcc</a>
     * resource qualifier.  0 if undefined.
     */
    int mcc;

    /**
     * IMSI MNC (Mobile Network Code), corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#MccQualifier">mnc</a>
     * resource qualifier.  0 if undefined. Note that the actual MNC may be 0; in order to check
     * for this use the {@link #MNC_ZERO} symbol.
     */
    int mnc;

    /**
     * Constant used to to represent MNC (Mobile Network Code) zero.
     * 0 cannot be used, since it is used to represent an undefined MNC.
     */
    static constexpr int MNC_ZERO = 0xffff;

    /**
     * Locale should persist on setting.  This is hidden because it is really
     * questionable whether this is the right way to expose the functionality.
     * @hide
     */
    bool userSetLocale;


    /** Constant for {@link #colorMode}: bits that encode whether the screen is wide gamut. */
    static constexpr int COLOR_MODE_WIDE_COLOR_GAMUT_MASK = 0x3;
    /**
     * Constant for {@link #colorMode}: a {@link #COLOR_MODE_WIDE_COLOR_GAMUT_MASK} value
     * indicating that it is unknown whether or not the screen is wide gamut.
     */
    static constexpr int COLOR_MODE_WIDE_COLOR_GAMUT_UNDEFINED = 0x0;
    /**
     * Constant for {@link #colorMode}: a {@link #COLOR_MODE_WIDE_COLOR_GAMUT_MASK} value
     * indicating that the screen is not wide gamut.
     * <p>Corresponds to the <code>-nowidecg</code> resource qualifier.</p>
     */
    static constexpr int COLOR_MODE_WIDE_COLOR_GAMUT_NO = 0x1;
    /**
     * Constant for {@link #colorMode}: a {@link #COLOR_MODE_WIDE_COLOR_GAMUT_MASK} value
     * indicating that the screen is wide gamut.
     * <p>Corresponds to the <code>-widecg</code> resource qualifier.</p>
     */
    static constexpr int COLOR_MODE_WIDE_COLOR_GAMUT_YES = 0x2;

    /** Constant for {@link #colorMode}: bits that encode the dynamic range of the screen. */
    static constexpr int COLOR_MODE_HDR_MASK = 0xc;
    /** Constant for {@link #colorMode}: bits shift to get the screen dynamic range. */
    static constexpr int COLOR_MODE_HDR_SHIFT = 2;
    /**
     * Constant for {@link #colorMode}: a {@link #COLOR_MODE_HDR_MASK} value
     * indicating that it is unknown whether or not the screen is HDR.
     */
    static constexpr int COLOR_MODE_HDR_UNDEFINED = 0x0;
    /**
     * Constant for {@link #colorMode}: a {@link #COLOR_MODE_HDR_MASK} value
     * indicating that the screen is not HDR (low/standard dynamic range).
     * <p>Corresponds to the <code>-lowdr</code> resource qualifier.</p>
     */
    static constexpr int COLOR_MODE_HDR_NO = 0x1 << COLOR_MODE_HDR_SHIFT;
    /**
     * Constant for {@link #colorMode}: a {@link #COLOR_MODE_HDR_MASK} value
     * indicating that the screen is HDR (dynamic range).
     * <p>Corresponds to the <code>-highdr</code> resource qualifier.</p>
     */
    static constexpr int COLOR_MODE_HDR_YES = 0x2 << COLOR_MODE_HDR_SHIFT;

    /** Constant for {@link #colorMode}: a value indicating that the color mode is undefined */
    static constexpr int COLOR_MODE_UNDEFINED = COLOR_MODE_WIDE_COLOR_GAMUT_UNDEFINED |
            COLOR_MODE_HDR_UNDEFINED;

    /**
     * Bit mask of color capabilities of the screen. Currently there are two fields:
     * <p>The {@link #COLOR_MODE_WIDE_COLOR_GAMUT_MASK} bits define the color gamut of
     * the screen. They may be one of
     * {@link #COLOR_MODE_WIDE_COLOR_GAMUT_NO} or {@link #COLOR_MODE_WIDE_COLOR_GAMUT_YES}.</p>
     *
     * <p>The {@link #COLOR_MODE_HDR_MASK} defines the dynamic range of the screen. They may be
     * one of {@link #COLOR_MODE_HDR_NO} or {@link #COLOR_MODE_HDR_YES}.</p>
     *
     * <p>See <a href="{@docRoot}guide/practices/screens_support.html">Supporting
     * Multiple Screens</a> for more information.</p>
     */
    int colorMode;

    /** Constant for {@link #screenLayout}: bits that encode the size. */
    static constexpr int SCREENLAYOUT_SIZE_MASK = 0x0f;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_SIZE_MASK}
     * value indicating that no size has been set. */
    static constexpr int SCREENLAYOUT_SIZE_UNDEFINED = 0x00;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_SIZE_MASK}
     * value indicating the screen is at least approximately 320x426 dp units,
     * corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenSizeQualifier">small</a>
     * resource qualifier.
     * See <a href="{@docRoot}guide/practices/screens_support.html">Supporting
     * Multiple Screens</a> for more information. */
    static constexpr int SCREENLAYOUT_SIZE_SMALL = 0x01;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_SIZE_MASK}
     * value indicating the screen is at least approximately 320x470 dp units,
     * corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenSizeQualifier">normal</a>
     * resource qualifier.
     * See <a href="{@docRoot}guide/practices/screens_support.html">Supporting
     * Multiple Screens</a> for more information. */
    static constexpr int SCREENLAYOUT_SIZE_NORMAL = 0x02;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_SIZE_MASK}
     * value indicating the screen is at least approximately 480x640 dp units,
     * corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenSizeQualifier">large</a>
     * resource qualifier.
     * See <a href="{@docRoot}guide/practices/screens_support.html">Supporting
     * Multiple Screens</a> for more information. */
    static constexpr int SCREENLAYOUT_SIZE_LARGE = 0x03;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_SIZE_MASK}
     * value indicating the screen is at least approximately 720x960 dp units,
     * corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenSizeQualifier">xlarge</a>
     * resource qualifier.
     * See <a href="{@docRoot}guide/practices/screens_support.html">Supporting
     * Multiple Screens</a> for more information.*/
    static constexpr int SCREENLAYOUT_SIZE_XLARGE = 0x04;

    /** Constant for {@link #screenLayout}: bits that encode the aspect ratio. */
    static constexpr int SCREENLAYOUT_LONG_MASK = 0x30;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_LONG_MASK}
     * value indicating that no size has been set. */
    static constexpr int SCREENLAYOUT_LONG_UNDEFINED = 0x00;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_LONG_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenAspectQualifier">notlong</a>
     * resource qualifier. */
    static constexpr int SCREENLAYOUT_LONG_NO = 0x10;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_LONG_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenAspectQualifier">long</a>
     * resource qualifier. */
    static constexpr int SCREENLAYOUT_LONG_YES = 0x20;

    /** Constant for {@link #screenLayout}: bits that encode the layout direction. */
    static constexpr int SCREENLAYOUT_LAYOUTDIR_MASK = 0xC0;
    /** Constant for {@link #screenLayout}: bits shift to get the layout direction. */
    static constexpr int SCREENLAYOUT_LAYOUTDIR_SHIFT = 6;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_LAYOUTDIR_MASK}
     * value indicating that no layout dir has been set. */
    static constexpr int SCREENLAYOUT_LAYOUTDIR_UNDEFINED = 0x00;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_LAYOUTDIR_MASK}
     * value indicating that a layout dir has been set to LTR. */
    static constexpr int SCREENLAYOUT_LAYOUTDIR_LTR = 0x01 << SCREENLAYOUT_LAYOUTDIR_SHIFT;
    /** Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_LAYOUTDIR_MASK}
     * value indicating that a layout dir has been set to RTL. */
    static constexpr int SCREENLAYOUT_LAYOUTDIR_RTL = 0x02 << SCREENLAYOUT_LAYOUTDIR_SHIFT;

    /** Constant for {@link #screenLayout}: bits that encode roundness of the screen. */
    static constexpr int SCREENLAYOUT_ROUND_MASK = 0x300;
    /** @hide Constant for {@link #screenLayout}: bit shift to get to screen roundness bits */
    static constexpr int SCREENLAYOUT_ROUND_SHIFT = 8;
    /**
     * Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_ROUND_MASK} value indicating
     * that it is unknown whether or not the screen has a round shape.
     */
    static constexpr int SCREENLAYOUT_ROUND_UNDEFINED = 0x00;
    /**
     * Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_ROUND_MASK} value indicating
     * that the screen does not have a rounded shape.
     */
    static constexpr int SCREENLAYOUT_ROUND_NO = 0x1 << SCREENLAYOUT_ROUND_SHIFT;
    /**
     * Constant for {@link #screenLayout}: a {@link #SCREENLAYOUT_ROUND_MASK} value indicating
     * that the screen has a rounded shape. Corners may not be visible to the user;
     * developers should pay special attention to the {@link android.view.WindowInsets} delivered
     * to views for more information about ensuring content is not obscured.
     *
     * <p>Corresponds to the <code>-round</code> resource qualifier.</p>
     */
    static constexpr int SCREENLAYOUT_ROUND_YES = 0x2 << SCREENLAYOUT_ROUND_SHIFT;

    /** Constant for {@link #screenLayout}: a value indicating that screenLayout is undefined */
    static constexpr int SCREENLAYOUT_UNDEFINED = SCREENLAYOUT_SIZE_UNDEFINED | SCREENLAYOUT_LONG_UNDEFINED |
            SCREENLAYOUT_LAYOUTDIR_UNDEFINED | SCREENLAYOUT_ROUND_UNDEFINED;

    /**
     * Special flag we generate to indicate that the screen layout requires
     * us to use a compatibility mode for apps that are not modern layout
     * aware.
     * @hide
     */
    static constexpr int SCREENLAYOUT_COMPAT_NEEDED = 0x10000000;

    /**
     * Bit mask of overall layout of the screen.  Currently there are four
     * fields:
     * <p>The {@link #SCREENLAYOUT_SIZE_MASK} bits define the overall size
     * of the screen.  They may be one of
     * {@link #SCREENLAYOUT_SIZE_SMALL}, {@link #SCREENLAYOUT_SIZE_NORMAL},
     * {@link #SCREENLAYOUT_SIZE_LARGE}, or {@link #SCREENLAYOUT_SIZE_XLARGE}.</p>
     *
     * <p>The {@link #SCREENLAYOUT_LONG_MASK} defines whether the screen
     * is wider/taller than normal.  They may be one of
     * {@link #SCREENLAYOUT_LONG_NO} or {@link #SCREENLAYOUT_LONG_YES}.</p>
     *
     * <p>The {@link #SCREENLAYOUT_LAYOUTDIR_MASK} defines whether the screen layout
     * is either LTR or RTL.  They may be one of
     * {@link #SCREENLAYOUT_LAYOUTDIR_LTR} or {@link #SCREENLAYOUT_LAYOUTDIR_RTL}.</p>
     *
     * <p>The {@link #SCREENLAYOUT_ROUND_MASK} defines whether the screen has a rounded
     * shape. They may be one of {@link #SCREENLAYOUT_ROUND_NO} or {@link #SCREENLAYOUT_ROUND_YES}.
     * </p>
     *
     * <p>See <a href="{@docRoot}guide/practices/screens_support.html">Supporting
     * Multiple Screens</a> for more information.</p>
     */
    int screenLayout;

    /**
     * Configuration relating to the windowing state of the object associated with this
     * Configuration. Contents of this field are not intended to affect resources, but need to be
     * communicated and propagated at the same time as the rest of Configuration.
     * @hide
     */
    //WindowConfiguration windowConfiguration = new WindowConfiguration();

    /** @hide */
    static int resetScreenLayout(int curLayout);

    /** @hide */
    static int reduceScreenLayout(int curLayout, int longSizeDp, int shortSizeDp);

    /** @hide */
    static std::string configurationDiffToString(int diff);
    /**
     * Check if the Configuration's current {@link #screenLayout} is at
     * least the given size.
     *
     * @param size The desired size, either {@link #SCREENLAYOUT_SIZE_SMALL},
     * {@link #SCREENLAYOUT_SIZE_NORMAL}, {@link #SCREENLAYOUT_SIZE_LARGE}, or
     * {@link #SCREENLAYOUT_SIZE_XLARGE}.
     * @return Returns true if the current screen layout size is at least
     * the given size.
     */
    bool isLayoutSizeAtLeast(int size)const;

    /** Constant for {@link #touchscreen}: a value indicating that no value has been set. */
    static constexpr int TOUCHSCREEN_UNDEFINED = 0;
    /** Constant for {@link #touchscreen}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#TouchscreenQualifier">notouch</a>
     * resource qualifier. */
    static constexpr int TOUCHSCREEN_NOTOUCH = 1;
    /** @deprecated Not currently supported or used. */
    static constexpr int TOUCHSCREEN_STYLUS = 2;
    /** Constant for {@link #touchscreen}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#TouchscreenQualifier">finger</a>
     * resource qualifier. */
    static constexpr int TOUCHSCREEN_FINGER = 3;

    /**
     * The kind of touch screen attached to the device.
     * One of: {@link #TOUCHSCREEN_NOTOUCH}, {@link #TOUCHSCREEN_FINGER}.
     */
    int touchscreen;

    /** Constant for {@link #keyboard}: a value indicating that no value has been set. */
    static constexpr int KEYBOARD_UNDEFINED = 0;
    /** Constant for {@link #keyboard}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ImeQualifier">nokeys</a>
     * resource qualifier. */
    static constexpr int KEYBOARD_NOKEYS = 1;
    /** Constant for {@link #keyboard}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ImeQualifier">qwerty</a>
     * resource qualifier. */
    static constexpr int KEYBOARD_QWERTY = 2;
    /** Constant for {@link #keyboard}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ImeQualifier">12key</a>
     * resource qualifier. */
    static constexpr int KEYBOARD_12KEY = 3;

    /**
     * The kind of keyboard attached to the device.
     * One of: {@link #KEYBOARD_NOKEYS}, {@link #KEYBOARD_QWERTY},
     * {@link #KEYBOARD_12KEY}.
     */
    int keyboard;

    /** Constant for {@link #keyboardHidden}: a value indicating that no value has been set. */
    static constexpr int KEYBOARDHIDDEN_UNDEFINED = 0;
    /** Constant for {@link #keyboardHidden}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#KeyboardAvailQualifier">keysexposed</a>
     * resource qualifier. */
    static constexpr int KEYBOARDHIDDEN_NO = 1;
    /** Constant for {@link #keyboardHidden}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#KeyboardAvailQualifier">keyshidden</a>
     * resource qualifier. */
    static constexpr int KEYBOARDHIDDEN_YES = 2;
    /** Constant matching actual resource implementation. {@hide} */
    static constexpr int KEYBOARDHIDDEN_SOFT = 3;

    /**
     * A flag indicating whether any keyboard is available.  Unlike
     * {@link #hardKeyboardHidden}, this also takes into account a soft
     * keyboard, so if the hard keyboard is hidden but there is soft
     * keyboard available, it will be set to NO.  Value is one of:
     * {@link #KEYBOARDHIDDEN_NO}, {@link #KEYBOARDHIDDEN_YES}.
     */
    int keyboardHidden;

    /** Constant for {@link #hardKeyboardHidden}: a value indicating that no value has been set. */
    static constexpr int HARDKEYBOARDHIDDEN_UNDEFINED = 0;
    /** Constant for {@link #hardKeyboardHidden}, value corresponding to the
     * physical keyboard being exposed. */
    static constexpr int HARDKEYBOARDHIDDEN_NO = 1;
    /** Constant for {@link #hardKeyboardHidden}, value corresponding to the
     * physical keyboard being hidden. */
    static constexpr int HARDKEYBOARDHIDDEN_YES = 2;

    /**
     * A flag indicating whether the hard keyboard has been hidden.  This will
     * be set on a device with a mechanism to hide the keyboard from the
     * user, when that mechanism is closed.  One of:
     * {@link #HARDKEYBOARDHIDDEN_NO}, {@link #HARDKEYBOARDHIDDEN_YES}.
     */
    int hardKeyboardHidden;

    /** Constant for {@link #navigation}: a value indicating that no value has been set. */
    static constexpr int NAVIGATION_UNDEFINED = 0;
    /** Constant for {@link #navigation}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NavigationQualifier">nonav</a>
     * resource qualifier. */
    static constexpr int NAVIGATION_NONAV = 1;
    /** Constant for {@link #navigation}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NavigationQualifier">dpad</a>
     * resource qualifier. */
    static constexpr int NAVIGATION_DPAD = 2;
    /** Constant for {@link #navigation}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NavigationQualifier">trackball</a>
     * resource qualifier. */
    static constexpr int NAVIGATION_TRACKBALL = 3;
    /** Constant for {@link #navigation}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NavigationQualifier">wheel</a>
     * resource qualifier. */
    static constexpr int NAVIGATION_WHEEL = 4;

    /**
     * The kind of navigation method available on the device.
     * One of: {@link #NAVIGATION_NONAV}, {@link #NAVIGATION_DPAD},
     * {@link #NAVIGATION_TRACKBALL}, {@link #NAVIGATION_WHEEL}.
     */
    int navigation;

    /** Constant for {@link #navigationHidden}: a value indicating that no value has been set. */
    static constexpr int NAVIGATIONHIDDEN_UNDEFINED = 0;
    /** Constant for {@link #navigationHidden}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NavAvailQualifier">navexposed</a>
     * resource qualifier. */
    static constexpr int NAVIGATIONHIDDEN_NO = 1;
    /** Constant for {@link #navigationHidden}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NavAvailQualifier">navhidden</a>
     * resource qualifier. */
    static constexpr int NAVIGATIONHIDDEN_YES = 2;

    /**
     * A flag indicating whether any 5-way or DPAD navigation available.
     * This will be set on a device with a mechanism to hide the navigation
     * controls from the user, when that mechanism is closed.  One of:
     * {@link #NAVIGATIONHIDDEN_NO}, {@link #NAVIGATIONHIDDEN_YES}.
     */
    int navigationHidden;

    /** Constant for {@link #orientation}: a value indicating that no value has been set. */
    static constexpr int ORIENTATION_UNDEFINED = 0;
    /** Constant for {@link #orientation}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#OrientationQualifier">port</a>
     * resource qualifier. */
    static constexpr int ORIENTATION_PORTRAIT = 1;
    /** Constant for {@link #orientation}, value corresponding to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#OrientationQualifier">land</a>
     * resource qualifier. */
    static constexpr int ORIENTATION_LANDSCAPE = 2;
    /** @deprecated Not currently supported or used. */
    static constexpr int ORIENTATION_SQUARE = 3;

    /**
     * Overall orientation of the screen.  May be one of
     * {@link #ORIENTATION_LANDSCAPE}, {@link #ORIENTATION_PORTRAIT}.
     */
    int orientation;

    /** Constant for {@link #uiMode}: bits that encode the mode type. */
    static constexpr int UI_MODE_TYPE_MASK = 0x0f;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value indicating that no mode type has been set. */
    static constexpr int UI_MODE_TYPE_UNDEFINED = 0x00;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">no
     * UI mode</a> resource qualifier specified. */
    static constexpr int UI_MODE_TYPE_NORMAL = 0x01;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">desk</a>
     * resource qualifier. */
    static constexpr int UI_MODE_TYPE_DESK = 0x02;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">car</a>
     * resource qualifier. */
    static constexpr int UI_MODE_TYPE_CAR = 0x03;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">television</a>
     * resource qualifier. */
    static constexpr int UI_MODE_TYPE_TELEVISION = 0x04;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">appliance</a>
     * resource qualifier. */
    static constexpr int UI_MODE_TYPE_APPLIANCE = 0x05;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">watch</a>
     * resource qualifier. */
    static constexpr int UI_MODE_TYPE_WATCH = 0x06;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_TYPE_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#UiModeQualifier">vrheadset</a>
     * resource qualifier. */
    static constexpr int UI_MODE_TYPE_VR_HEADSET = 0x07;

    /** Constant for {@link #uiMode}: bits that encode the night mode. */
    static constexpr int UI_MODE_NIGHT_MASK = 0x30;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_NIGHT_MASK}
     * value indicating that no mode type has been set. */
    static constexpr int UI_MODE_NIGHT_UNDEFINED = 0x00;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_NIGHT_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NightQualifier">notnight</a>
     * resource qualifier. */
    static constexpr int UI_MODE_NIGHT_NO = 0x10;
    /** Constant for {@link #uiMode}: a {@link #UI_MODE_NIGHT_MASK}
     * value that corresponds to the
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#NightQualifier">night</a>
     * resource qualifier. */
    static constexpr int UI_MODE_NIGHT_YES = 0x20;

    /**
     * Bit mask of the ui mode.  Currently there are two fields:
     * <p>The {@link #UI_MODE_TYPE_MASK} bits define the overall ui mode of the
     * device. They may be one of {@link #UI_MODE_TYPE_UNDEFINED},
     * {@link #UI_MODE_TYPE_NORMAL}, {@link #UI_MODE_TYPE_DESK},
     * {@link #UI_MODE_TYPE_CAR}, {@link #UI_MODE_TYPE_TELEVISION},
     * {@link #UI_MODE_TYPE_APPLIANCE}, {@link #UI_MODE_TYPE_WATCH},
     * or {@link #UI_MODE_TYPE_VR_HEADSET}.
     *
     * <p>The {@link #UI_MODE_NIGHT_MASK} defines whether the screen
     * is in a special mode. They may be one of {@link #UI_MODE_NIGHT_UNDEFINED},
     * {@link #UI_MODE_NIGHT_NO} or {@link #UI_MODE_NIGHT_YES}.
     */
    int uiMode;

    /**
     * Default value for {@link #screenWidthDp} indicating that no width
     * has been specified.
     */
    static constexpr int SCREEN_WIDTH_DP_UNDEFINED = 0;

    /**
     * The current width of the available screen space, in dp units,
     * corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenWidthQualifier">screen
     * width</a> resource qualifier.  Set to
     * {@link #SCREEN_WIDTH_DP_UNDEFINED} if no width is specified.
     */
    int screenWidthDp;

    /**
     * Default value for {@link #screenHeightDp} indicating that no width
     * has been specified.
     */
    static constexpr int SCREEN_HEIGHT_DP_UNDEFINED = 0;

    /**
     * The current height of the available screen space, in dp units,
     * corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#ScreenHeightQualifier">screen
     * height</a> resource qualifier.  Set to
     * {@link #SCREEN_HEIGHT_DP_UNDEFINED} if no height is specified.
     */
    int screenHeightDp;

    /**
     * Default value for {@link #smallestScreenWidthDp} indicating that no width
     * has been specified.
     */
    static constexpr int SMALLEST_SCREEN_WIDTH_DP_UNDEFINED = 0;

    /**
     * The smallest screen size an application will see in normal operation,
     * corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#SmallestScreenWidthQualifier">smallest
     * screen width</a> resource qualifier.
     * This is the smallest value of both screenWidthDp and screenHeightDp
     * in both portrait and landscape.  Set to
     * {@link #SMALLEST_SCREEN_WIDTH_DP_UNDEFINED} if no width is specified.
     */
    int smallestScreenWidthDp;

    /**
     * Default value for {@link #densityDpi} indicating that no width
     * has been specified.
     */
    static constexpr int DENSITY_DPI_UNDEFINED = 0;

    /**
     * Value for {@link #densityDpi} for resources that scale to any density (vector drawables).
     * {@hide}
     */
    static constexpr int DENSITY_DPI_ANY = 0xfffe;

    /**
     * Value for {@link #densityDpi} for resources that are not meant to be scaled.
     * {@hide}
     */
    static constexpr int DENSITY_DPI_NONE = 0xffff;

    /**
     * The target screen density being rendered to,
     * corresponding to
     * <a href="{@docRoot}guide/topics/resources/providing-resources.html#DensityQualifier">density</a>
     * resource qualifier.  Set to
     * {@link #DENSITY_DPI_UNDEFINED} if no density is specified.
     */
    int densityDpi;

    /** @hide Hack to get this information from WM to app running in compat mode. */
    int compatScreenWidthDp;
    /** @hide Hack to get this information from WM to app running in compat mode. */
    int compatScreenHeightDp;
    /** @hide Hack to get this information from WM to app running in compat mode. */
    int compatSmallestScreenWidthDp;

    /**
     * An undefined assetsSeq. This will not override an existing assetsSeq.
     * @hide
     */
    static constexpr int ASSETS_SEQ_UNDEFINED = 0;

    /**
     * Internal counter that allows us to piggyback off the configuration change mechanism to
     * signal to apps that the the assets for an Application have changed. A difference in these
     * between two Configurations will yield a diff flag of
     * {@link ActivityInfo#CONFIG_ASSETS_PATHS}.
     * @hide
     */
    int assetsSeq;

    /**
     * @hide Internal book-keeping.
     */
    int seq;


    /** @hide Native-specific bit mask for MCC config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_MCC = 0x0001;
    /** @hide Native-specific bit mask for MNC config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_MNC = 0x0002;
    /** @hide Native-specific bit mask for LOCALE config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_LOCALE = 0x0004;
    /** @hide Native-specific bit mask for TOUCHSCREEN config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_TOUCHSCREEN = 0x0008;
    /** @hide Native-specific bit mask for KEYBOARD config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_KEYBOARD = 0x0010;
    /** @hide Native-specific bit mask for KEYBOARD_HIDDEN config; DO NOT USE UNLESS YOU
     * ARE SURE. */
    static constexpr int NATIVE_CONFIG_KEYBOARD_HIDDEN = 0x0020;
    /** @hide Native-specific bit mask for NAVIGATION config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_NAVIGATION = 0x0040;
    /** @hide Native-specific bit mask for ORIENTATION config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_ORIENTATION = 0x0080;
    /** @hide Native-specific bit mask for DENSITY config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_DENSITY = 0x0100;
    /** @hide Native-specific bit mask for SCREEN_SIZE config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_SCREEN_SIZE = 0x0200;
    /** @hide Native-specific bit mask for VERSION config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_VERSION = 0x0400;
    /** @hide Native-specific bit mask for SCREEN_LAYOUT config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_SCREEN_LAYOUT = 0x0800;
    /** @hide Native-specific bit mask for UI_MODE config; DO NOT USE UNLESS YOU ARE SURE. */
    static constexpr int NATIVE_CONFIG_UI_MODE = 0x1000;
    /** @hide Native-specific bit mask for SMALLEST_SCREEN_SIZE config; DO NOT USE UNLESS YOU
     * ARE SURE. */
    static constexpr int NATIVE_CONFIG_SMALLEST_SCREEN_SIZE = 0x2000;
    /** @hide Native-specific bit mask for LAYOUTDIR config ; DO NOT USE UNLESS YOU ARE SURE.*/
    static constexpr int NATIVE_CONFIG_LAYOUTDIR = 0x4000;
    /** @hide Native-specific bit mask for COLOR_MODE config ; DO NOT USE UNLESS YOU ARE SURE.*/
    static constexpr int NATIVE_CONFIG_COLOR_MODE = 0x10000;

    /**
     * <p>Construct an invalid Configuration. This state is only suitable for constructing a
     * Configuration delta that will be applied to some valid Configuration object. In order to
     * create a valid standalone Configuration, you must call {@link #setToDefaults}. </p>
     *
     * <p>Example:</p>
     * <pre class="prettyprint">
     *     Configuration validConfig = new Configuration();
     *     validConfig.setToDefaults();
     *
     *     Configuration deltaOnlyConfig = new Configuration();
     *     deltaOnlyConfig.orientation = Configuration.ORIENTATION_LANDSCAPE;
     *
     *     validConfig.updateFrom(deltaOnlyConfig);
     * </pre>
     */
    /* This brings mLocaleList in sync with locale in case a user of the older API who doesn't know
     * about setLocales() has changed locale directly. */
private:
    void fixUpLocaleList();
    Configuration(Parcel& source);
    static int getScreenLayoutNoDirection(int screenLayout);
public:
    Configuration();

    /**
     * Makes a deep copy suitable for modification.
     */
    Configuration(Configuration& o);
    /**
     * Sets the fields in this object to those in the given Configuration.
     *
     * @param o The Configuration object used to set the values of this Configuration's fields.
     */
    void setTo(Configuration& o);
    std::string toString()const;

    /**
     * Write to a protocol buffer output stream.
     * Protocol buffer message definition at {@link android.content.ConfigurationProto}
     *
     * @param protoOutputStream Stream to write the Configuration object to.
     * @param fieldId           Field Id of the Configuration as defined in the parent message
     * @hide
     */
    //void writeToProto(ProtoOutputStream protoOutputStream, long fieldId);
    /**
     * Write full {@link android.content.ResourcesConfigurationProto} to protocol buffer output
     * stream.
     *
     * @param protoOutputStream Stream to write the Configuration object to.
     * @param fieldId           Field Id of the Configuration as defined in the parent message
     * @param metrics           Current display information
     * @hide
     */
    //void writeResConfigToProto(ProtoOutputStream protoOutputStream, long fieldId,const DisplayMetrics& metrics);

    /**
     * Convert the UI mode to a human readable format.
     * @hide
     */
    static std::string uiModeToString(int uiMode);

    /**
     * Set this object to the system defaults.
     */
    void setToDefaults();

    /**
     * Set this object to completely undefined.
     * @hide
     */
    void unset();

    /** {@hide} */
    void makeDefault();

    /**
     * Copies the fields from delta into this Configuration object, keeping
     * track of which ones have changed. Any undefined fields in {@code delta}
     * are ignored and not copied in to the current Configuration.
     *
     * @return a bit mask of the changed fields, as per {@link #diff}
     */
    int updateFrom(const Configuration& delta);

    /**
     * Return a bit mask of the differences between this Configuration
     * object and the given one.  Does not change the values of either.  Any
     * undefined fields in <var>delta</var> are ignored.
     * @return Returns a bit mask indicating which configuration
     * values has changed, containing any combination of
     * {@link android.content.pm.ActivityInfo#CONFIG_FONT_SCALE
     * PackageManager.ActivityInfo.CONFIG_FONT_SCALE},
     * {@link android.content.pm.ActivityInfo#CONFIG_MCC
     * PackageManager.ActivityInfo.CONFIG_MCC},
     * {@link android.content.pm.ActivityInfo#CONFIG_MNC
     * PackageManager.ActivityInfo.CONFIG_MNC},
     * {@link android.content.pm.ActivityInfo#CONFIG_LOCALE
     * PackageManager.ActivityInfo.CONFIG_LOCALE},
     * {@link android.content.pm.ActivityInfo#CONFIG_TOUCHSCREEN
     * PackageManager.ActivityInfo.CONFIG_TOUCHSCREEN},
     * {@link android.content.pm.ActivityInfo#CONFIG_KEYBOARD
     * PackageManager.ActivityInfo.CONFIG_KEYBOARD},
     * {@link android.content.pm.ActivityInfo#CONFIG_NAVIGATION
     * PackageManager.ActivityInfo.CONFIG_NAVIGATION},
     * {@link android.content.pm.ActivityInfo#CONFIG_ORIENTATION
     * PackageManager.ActivityInfo.CONFIG_ORIENTATION},
     * {@link android.content.pm.ActivityInfo#CONFIG_SCREEN_LAYOUT
     * PackageManager.ActivityInfo.CONFIG_SCREEN_LAYOUT}, or
     * {@link android.content.pm.ActivityInfo#CONFIG_SCREEN_SIZE
     * PackageManager.ActivityInfo.CONFIG_SCREEN_SIZE}, or
     * {@link android.content.pm.ActivityInfo#CONFIG_SMALLEST_SCREEN_SIZE
     * PackageManager.ActivityInfo.CONFIG_SMALLEST_SCREEN_SIZE}.
     * {@link android.content.pm.ActivityInfo#CONFIG_LAYOUT_DIRECTION
     * PackageManager.ActivityInfo.CONFIG_LAYOUT_DIRECTION}.
     */
    int diff(const Configuration& delta)const;

    /**
     * Returns the diff against the provided {@link Configuration} excluding values that would
     * publicly be equivalent, such as appBounds.
     * @param delta {@link Configuration} to compare to.
     *
     * TODO(b/36812336): Remove once appBounds has been moved out of Configuration.
     * {@hide}
     */
    int diffPublicOnly(const Configuration& delta)const;
    /**
     * Variation of {@link #diff(Configuration)} with an option to skip checks for undefined values.
     *
     * @hide
     */
    int diff(const Configuration& delta, bool compareUndefined, bool publicOnly)const;

    /**
     * Determines if a new resource needs to be loaded from the bit set of
     * configuration changes returned by {@link #updateFrom(Configuration)}.
     *
     * @param configChanges the mask of changes configurations as returned by
     *                      {@link #updateFrom(Configuration)}
     * @param interestingChanges the configuration changes that the resource
     *                           can handle as given in
     *                           {@link android.util.TypedValue#changingConfigurations}
     * @return {@code true} if the resource needs to be loaded, {@code false}
     *         otherwise
     */
    static bool needNewResources(int configChanges,int interestingChanges);

    /**
     * @hide Return true if the sequence of 'other' is better than this.  Assumes
     * that 'this' is your current sequence and 'other' is a new one you have
     * received some how and want to compare with what you have.
     */
    bool isOtherSeqNewer(const Configuration& other)const;

    /**
     * Parcelable methods
     */
    void writeToParcel(Parcel& dest, int flags);

    void readFromParcel(Parcel& source);

    int compareTo(const Configuration& that);

    /**
     * Get the locale list. This is the preferred way for getting the locales (instead of using
     * the direct accessor to {@link #locale}, which would only provide the primary locale).
     *
     * @return The locale list.
     */
    //LocaleList* getLocales();

    /**
     * Set the locale list. This is the preferred way for setting up the locales (instead of using
     * the direct accessor or {@link #setLocale(Locale)}). This will also set the layout direction
     * according to the first locale in the list.
     *
     * Note that the layout direction will always come from the first locale in the locale list,
     * even if the locale is not supported by the resources (the resources may only support
     * another locale further down the list which has a different direction).
     *
     * @param locales The locale list. If null, an empty LocaleList will be assigned.
     */
    //void setLocales(LocaleList* locales);
    /**
     * Set the locale list to a list of just one locale. This will also set the layout direction
     * according to the locale.
     *
     * Note that after this is run, calling <code>.equals()</code> on the input locale and the
     * {@link #locale} attribute would return <code>true</code> if they are not null, but there is
     * no guarantee that they would be the same object.
     *
     * See also the note about layout direction in {@link #setLocales(LocaleList)}.
     *
     * @param loc The locale. Can be null.
     */
    //void setLocale(Locale* loc);
    /**
     * @hide
     *
     * Clears the locale without changing layout direction.
     */
    void clearLocales();

    /**
     * Return the layout direction. Will be either {@link View#LAYOUT_DIRECTION_LTR} or
     * {@link View#LAYOUT_DIRECTION_RTL}.
     *
     * @return Returns {@link View#LAYOUT_DIRECTION_RTL} if the configuration
     * is {@link #SCREENLAYOUT_LAYOUTDIR_RTL}, otherwise {@link View#LAYOUT_DIRECTION_LTR}.
     */
    int getLayoutDirection()const;

    /**
     * Set the layout direction from a Locale.
     *
     * @param loc The Locale. If null will set the layout direction to
     * {@link View#LAYOUT_DIRECTION_LTR}. If not null will set it to the layout direction
     * corresponding to the Locale.
     *
     * @see View#LAYOUT_DIRECTION_LTR
     * @see View#LAYOUT_DIRECTION_RTL
     */
    //void setLayoutDirection(Locale loc);
    /**
     * Return whether the screen has a round shape. Apps may choose to change styling based
     * on this property, such as the alignment or layout of text or informational icons.
     *
     * @return true if the screen is rounded, false otherwise
     */
    bool isScreenRound()const;
    /**
     * Return whether the screen has a wide color gamut and wide color gamut rendering
     * is supported by this device.
     *
     * When true, it implies the screen is colorspace aware but not
     * necessarily color-managed. The final colors may still be changed by the
     * screen depending on user settings.
     *
     * @return true if the screen has a wide color gamut and wide color gamut rendering
     * is supported, false otherwise
     */
    bool isScreenWideColorGamut()const;
    /**
     * Return whether the screen has a high dynamic range.
     *
     * @return true if the screen has a high dynamic range, false otherwise
     */
    bool isScreenHdr()const;

    /**
     *
     * @hide
     */
    //static std::string localesToResourceQualifier(LocaleList locs);
    /**
     * Returns a string representation of the configuration that can be parsed
     * by build tools (like AAPT), without display metrics included
     *
     * @hide
     */
    static std::string resourceQualifierString(const Configuration& config);

    /**
     * Returns a string representation of the configuration that can be parsed
     * by build tools (like AAPT).
     *
     * @hide
     */
    static std::string resourceQualifierString(const Configuration& config,const DisplayMetrics* metrics);

    /**
     * Generate a delta Configuration between <code>base</code> and <code>change</code>. The
     * resulting delta can be used with {@link #updateFrom(Configuration)}.
     * <p />
     * Caveat: If the any of the Configuration's members becomes undefined, then
     * {@link #updateFrom(Configuration)} will treat it as a no-op and not update that member.
     *
     * This is fine for device configurations as no member is ever undefined.
     * {@hide}
     */
    static Configuration generateDelta(const Configuration& base,const Configuration& change);
    static void readXmlAttrs(XmlPullParser& parser, Configuration& configOut);
};
}/*endof namespace*/
#endif/*__CONFIGURTATION_H__*/
