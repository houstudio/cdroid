#include <core/build.h>
#include <utils/textutils.h>
#include <view/configuration.h>
#include <view/view.h>
namespace cdroid{

//public final WindowConfiguration windowConfiguration = new WindowConfiguration();

int Configuration::resetScreenLayout(int curLayout) {
	return (curLayout&~(SCREENLAYOUT_LONG_MASK | SCREENLAYOUT_SIZE_MASK
					| SCREENLAYOUT_COMPAT_NEEDED))
			| (SCREENLAYOUT_LONG_YES | SCREENLAYOUT_SIZE_XLARGE);
}

/** @hide */
int Configuration::reduceScreenLayout(int curLayout, int longSizeDp, int shortSizeDp) {
	int screenLayoutSize;
	bool screenLayoutLong;
	bool screenLayoutCompatNeeded;

	// These semi-magic numbers define our compatibility modes for
	// applications with different screens.  These are guarantees to
	// app developers about the space they can expect for a particular
	// configuration.  DO NOT CHANGE!
	if (longSizeDp < 470) {
		// This is shorter than an HVGA normal density screen (which
		// is 480 pixels on its long side).
		screenLayoutSize = SCREENLAYOUT_SIZE_SMALL;
		screenLayoutLong = false;
		screenLayoutCompatNeeded = false;
	} else {
		// What size is this screen screen?
		if (longSizeDp >= 960 && shortSizeDp >= 720) {
			// 1.5xVGA or larger screens at medium density are the point
			// at which we consider it to be an extra large screen.
			screenLayoutSize = SCREENLAYOUT_SIZE_XLARGE;
		} else if (longSizeDp >= 640 && shortSizeDp >= 480) {
			// VGA or larger screens at medium density are the point
			// at which we consider it to be a large screen.
			screenLayoutSize = SCREENLAYOUT_SIZE_LARGE;
		} else {
			screenLayoutSize = SCREENLAYOUT_SIZE_NORMAL;
		}

		// If this screen is wider than normal HVGA, or taller
		// than FWVGA, then for old apps we want to run in size
		// compatibility mode.
		if (shortSizeDp > 321 || longSizeDp > 570) {
			screenLayoutCompatNeeded = true;
		} else {
			screenLayoutCompatNeeded = false;
		}

		// Is this a long screen?
		if (((longSizeDp*3)/5) >= (shortSizeDp-1)) {
			// Anything wider than WVGA (5:3) is considering to be long.
			screenLayoutLong = true;
		} else {
			screenLayoutLong = false;
		}
	}

	// Now reduce the last screenLayout to not be better than what we
	// have found.
	if (!screenLayoutLong) {
		curLayout = (curLayout&~SCREENLAYOUT_LONG_MASK) | SCREENLAYOUT_LONG_NO;
	}
	if (screenLayoutCompatNeeded) {
		curLayout |= Configuration::SCREENLAYOUT_COMPAT_NEEDED;
	}
	int curSize = curLayout&SCREENLAYOUT_SIZE_MASK;
	if (screenLayoutSize < curSize) {
		curLayout = (curLayout&~SCREENLAYOUT_SIZE_MASK) | screenLayoutSize;
	}
	return curLayout;
}

std::string Configuration::configurationDiffToString(int diff) {
	std::vector<std::string> list;
	/*if ((diff & ActivityInfo.CONFIG_MCC) != 0) {
		list.push_back("CONFIG_MCC");
	}
	if ((diff & ActivityInfo.CONFIG_MNC) != 0) {
		list.push_back("CONFIG_MNC");
	}
	if ((diff & ActivityInfo.CONFIG_LOCALE) != 0) {
		list.push_back("CONFIG_LOCALE");
	}
	if ((diff & ActivityInfo.CONFIG_TOUCHSCREEN) != 0) {
		list.push_back("CONFIG_TOUCHSCREEN");
	}
	if ((diff & ActivityInfo.CONFIG_KEYBOARD) != 0) {
		list.push_back("CONFIG_KEYBOARD");
	}
	if ((diff & ActivityInfo.CONFIG_KEYBOARD_HIDDEN) != 0) {
		list.push_back("CONFIG_KEYBOARD_HIDDEN");
	}
	if ((diff & ActivityInfo.CONFIG_NAVIGATION) != 0) {
		list.push_back("CONFIG_NAVIGATION");
	}
	if ((diff & ActivityInfo.CONFIG_ORIENTATION) != 0) {
		list.push_back("CONFIG_ORIENTATION");
	}
	if ((diff & ActivityInfo.CONFIG_SCREEN_LAYOUT) != 0) {
		list.push_back("CONFIG_SCREEN_LAYOUT");
	}
	if ((diff & ActivityInfo.CONFIG_COLOR_MODE) != 0) {
		list.push_back("CONFIG_COLOR_MODE");
	}
	if ((diff & ActivityInfo.CONFIG_UI_MODE) != 0) {
		list.push_back("CONFIG_UI_MODE");
	}
	if ((diff & ActivityInfo.CONFIG_SCREEN_SIZE) != 0) {
		list.push_back("CONFIG_SCREEN_SIZE");
	}
	if ((diff & ActivityInfo.CONFIG_SMALLEST_SCREEN_SIZE) != 0) {
		list.push_back("CONFIG_SMALLEST_SCREEN_SIZE");
	}
	if ((diff & ActivityInfo.CONFIG_LAYOUT_DIRECTION) != 0) {
		list.push_back("CONFIG_LAYOUT_DIRECTION");
	}
	if ((diff & ActivityInfo.CONFIG_FONT_SCALE) != 0) {
		list.push_back("CONFIG_FONT_SCALE");
	}
	if ((diff & ActivityInfo.CONFIG_ASSETS_PATHS) != 0) {
		list.push_back("CONFIG_ASSETS_PATHS");
	}*/
    std::ostringstream oss;
	for (int i = 0, n = list.size(); i < n; i++) {
		oss<<list.at(i);
		if (i != n - 1) {
			oss<<", ";
		}
	}
	oss<<"}";
	return oss.str();
}

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
bool Configuration::isLayoutSizeAtLeast(int size) const{
	int cur = screenLayout&SCREENLAYOUT_SIZE_MASK;
	if (cur == SCREENLAYOUT_SIZE_UNDEFINED) return false;
	return cur >= size;
}


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
Configuration::Configuration() {
	unset();
}

/**
 * Makes a deep copy suitable for modification.
 */
Configuration::Configuration(Configuration& o) {
	setTo(o);
}

/* This brings mLocaleList in sync with locale in case a user of the older API who doesn't know
 * about setLocales() has changed locale directly. */
void Configuration::fixUpLocaleList() {
	/*if ((locale == nullptr && !mLocaleList.isEmpty()) ||
			(locale && !locale.equals(mLocaleList.get(0)))) {
		mLocaleList = locale == nullptr ? LocaleList.getEmptyLocaleList() : new LocaleList(locale);
	}*/
}

/**
 * Sets the fields in this object to those in the given Configuration.
 *
 * @param o The Configuration object used to set the values of this Configuration's fields.
 */
void Configuration::setTo(Configuration& o) {
	fontScale = o.fontScale;
	mcc = o.mcc;
	mnc = o.mnc;
	//locale = o.locale == null ? null : (Locale) o.locale.clone();
	o.fixUpLocaleList();
	//mLocaleList = o.mLocaleList;
	userSetLocale = o.userSetLocale;
	touchscreen = o.touchscreen;
	keyboard = o.keyboard;
	keyboardHidden = o.keyboardHidden;
	hardKeyboardHidden = o.hardKeyboardHidden;
	navigation = o.navigation;
	navigationHidden = o.navigationHidden;
	orientation = o.orientation;
	screenLayout = o.screenLayout;
	colorMode = o.colorMode;
	uiMode = o.uiMode;
	screenWidthDp = o.screenWidthDp;
	screenHeightDp = o.screenHeightDp;
	smallestScreenWidthDp = o.smallestScreenWidthDp;
	densityDpi = o.densityDpi;
	compatScreenWidthDp = o.compatScreenWidthDp;
	compatScreenHeightDp = o.compatScreenHeightDp;
	compatSmallestScreenWidthDp = o.compatSmallestScreenWidthDp;
	assetsSeq = o.assetsSeq;
	seq = o.seq;
	//windowConfiguration.setTo(o.windowConfiguration);
}

std::string Configuration::toString()const {
    std::ostringstream  sb;
	sb<<"{";
	sb<<fontScale;
	sb<<" ";
	if (mcc != 0) {
		sb<<mcc;
		sb<<"mcc";
	} else {
		sb<<"?mcc";
	}
	if (mnc != 0) {
		sb<<mnc;
		sb<<"mnc";
	} else {
		sb<<"?mnc";
	}
	/*fixUpLocaleList();
	if (!mLocaleList.isEmpty()) {
		sb<<" ";
		sb<<mLocaleList;
	} else {
		sb<<" ?localeList";
	}*/
	const int layoutDir = (screenLayout&SCREENLAYOUT_LAYOUTDIR_MASK);
	switch (layoutDir) {
		case SCREENLAYOUT_LAYOUTDIR_UNDEFINED: sb<<" ?layoutDir"; break;
		case SCREENLAYOUT_LAYOUTDIR_LTR: sb<<" ldltr"; break;
		case SCREENLAYOUT_LAYOUTDIR_RTL: sb<<" ldrtl"; break;
		default: sb<<" layoutDir=";
			sb<< (layoutDir >> SCREENLAYOUT_LAYOUTDIR_SHIFT); break;
	}
	if (smallestScreenWidthDp != SMALLEST_SCREEN_WIDTH_DP_UNDEFINED) {
		sb<<" sw"; sb<<smallestScreenWidthDp; sb<<"dp";
	} else {
		sb<<" ?swdp";
	}
	if (screenWidthDp != SCREEN_WIDTH_DP_UNDEFINED) {
		sb<<" w"; sb<<screenWidthDp; sb<<"dp";
	} else {
		sb<<" ?wdp";
	}
	if (screenHeightDp != SCREEN_HEIGHT_DP_UNDEFINED) {
		sb<<" h"; sb<<screenHeightDp; sb<<"dp";
	} else {
		sb<<" ?hdp";
	}
	if (densityDpi != DENSITY_DPI_UNDEFINED) {
		sb<<" "; sb<<densityDpi; sb<<"dpi";
	} else {
		sb<<" ?density";
	}
	switch ((screenLayout&SCREENLAYOUT_SIZE_MASK)) {
		case SCREENLAYOUT_SIZE_UNDEFINED: sb<<" ?lsize"; break;
		case SCREENLAYOUT_SIZE_SMALL: sb<<" smll"; break;
		case SCREENLAYOUT_SIZE_NORMAL: sb<<" nrml"; break;
		case SCREENLAYOUT_SIZE_LARGE: sb<<" lrg"; break;
		case SCREENLAYOUT_SIZE_XLARGE: sb<<" xlrg"; break;
		default: sb<<" layoutSize=";
				sb<< (screenLayout&SCREENLAYOUT_SIZE_MASK); break;
	}
	switch ((screenLayout&SCREENLAYOUT_LONG_MASK)) {
		case SCREENLAYOUT_LONG_UNDEFINED: sb<<" ?long"; break;
		case SCREENLAYOUT_LONG_NO: /* not-long is not interesting to print */ break;
		case SCREENLAYOUT_LONG_YES: sb<<" long"; break;
		default: sb<<" layoutLong=";
				sb<< (screenLayout&SCREENLAYOUT_LONG_MASK); break;
	}
	switch ((colorMode &COLOR_MODE_HDR_MASK)) {
		case COLOR_MODE_HDR_UNDEFINED: sb<<" ?ldr"; break; // most likely not HDR
		case COLOR_MODE_HDR_NO: /* ldr is not interesting to print */ break;
		case COLOR_MODE_HDR_YES: sb<<" hdr"; break;
		default: sb<<" dynamicRange=";
			sb<< (colorMode &COLOR_MODE_HDR_MASK); break;
	}
	switch ((colorMode &COLOR_MODE_WIDE_COLOR_GAMUT_MASK)) {
		case COLOR_MODE_WIDE_COLOR_GAMUT_UNDEFINED: sb<<" ?wideColorGamut"; break;
		case COLOR_MODE_WIDE_COLOR_GAMUT_NO: /* not wide is not interesting to print */ break;
		case COLOR_MODE_WIDE_COLOR_GAMUT_YES: sb<<" widecg"; break;
		default: sb<<" wideColorGamut=";
			sb<< (colorMode &COLOR_MODE_WIDE_COLOR_GAMUT_MASK); break;
	}
	switch (orientation) {
		case ORIENTATION_UNDEFINED: sb<<" ?orien"; break;
		case ORIENTATION_LANDSCAPE: sb<<" land"; break;
		case ORIENTATION_PORTRAIT: sb<<" port"; break;
		default: sb<<" orien=" <<orientation; break;
	}
	switch ((uiMode&UI_MODE_TYPE_MASK)) {
		case UI_MODE_TYPE_UNDEFINED: sb<<" ?uimode"; break;
		case UI_MODE_TYPE_NORMAL: /* normal is not interesting to print */ break;
		case UI_MODE_TYPE_DESK: sb<<" desk"; break;
		case UI_MODE_TYPE_CAR: sb<<" car"; break;
		case UI_MODE_TYPE_TELEVISION: sb<<" television"; break;
		case UI_MODE_TYPE_APPLIANCE: sb<<" appliance"; break;
		case UI_MODE_TYPE_WATCH: sb<<" watch"; break;
		case UI_MODE_TYPE_VR_HEADSET: sb<<" vrheadset"; break;
		default: sb<<" uimode="<< (uiMode&UI_MODE_TYPE_MASK); break;
	}
	switch ((uiMode&UI_MODE_NIGHT_MASK)) {
		case UI_MODE_NIGHT_UNDEFINED: sb<<" ?night"; break;
		case UI_MODE_NIGHT_NO: /* not-night is not interesting to print */ break;
		case UI_MODE_NIGHT_YES: sb<<" night"; break;
		default: sb<<" night="<< (uiMode&UI_MODE_NIGHT_MASK); break;
	}
	switch (touchscreen) {
		case TOUCHSCREEN_UNDEFINED: sb<<" ?touch"; break;
		case TOUCHSCREEN_NOTOUCH: sb<<" -touch"; break;
		case TOUCHSCREEN_STYLUS: sb<<" stylus"; break;
		case TOUCHSCREEN_FINGER: sb<<" finger"; break;
		default: sb<<" touch="<<touchscreen; break;
	}
	switch (keyboard) {
		case KEYBOARD_UNDEFINED: sb<<" ?keyb"; break;
		case KEYBOARD_NOKEYS: sb<<" -keyb"; break;
		case KEYBOARD_QWERTY: sb<<" qwerty"; break;
		case KEYBOARD_12KEY: sb<<" 12key"; break;
		default: sb<<" keys="<<keyboard; break;
	}
	switch (keyboardHidden) {
		case KEYBOARDHIDDEN_UNDEFINED: sb<<"/?"; break;
		case KEYBOARDHIDDEN_NO: sb<<"/v"; break;
		case KEYBOARDHIDDEN_YES: sb<<"/h"; break;
		case KEYBOARDHIDDEN_SOFT: sb<<"/s"; break;
		default: sb<<"/"<<keyboardHidden; break;
	}
	switch (hardKeyboardHidden) {
		case HARDKEYBOARDHIDDEN_UNDEFINED: sb<<"/?"; break;
		case HARDKEYBOARDHIDDEN_NO: sb<<"/v"; break;
		case HARDKEYBOARDHIDDEN_YES: sb<<"/h"; break;
		default: sb<<"/"<<hardKeyboardHidden; break;
	}
	switch (navigation) {
		case NAVIGATION_UNDEFINED: sb<<" ?nav"; break;
		case NAVIGATION_NONAV: sb<<" -nav"; break;
		case NAVIGATION_DPAD: sb<<" dpad"; break;
		case NAVIGATION_TRACKBALL: sb<<" tball"; break;
		case NAVIGATION_WHEEL: sb<<" wheel"; break;
		default: sb<<" nav=" << navigation; break;
	}
	switch (navigationHidden) {
		case NAVIGATIONHIDDEN_UNDEFINED: sb<<"/?"; break;
		case NAVIGATIONHIDDEN_NO: sb<<"/v"; break;
		case NAVIGATIONHIDDEN_YES: sb<<"/h"; break;
		default: sb<<"/" << navigationHidden; break;
	}
	//sb<<" winConfig=" << windowConfiguration;
	if (assetsSeq != 0) {
		sb<<" as."<<assetsSeq;
	}
	if (seq != 0) {
		sb<<" s."<<seq;
	}
	sb<<'}';
	return sb.str();
}

/**
 * Write to a protocol buffer output stream.
 * Protocol buffer message definition at {@link android.content.ConfigurationProto}
 *
 * @param protoOutputStream Stream to write the Configuration object to.
 * @param fieldId           Field Id of the Configuration as defined in the parent message
 * @hide
 */
#if 0
void Configuration::writeToProto(ProtoOutputStream protoOutputStream, long fieldId) {
	const long token = protoOutputStream.start(fieldId);
	protoOutputStream.write(FONT_SCALE, fontScale);
	protoOutputStream.write(MCC, mcc);
	protoOutputStream.write(MNC, mnc);
	mLocaleList.writeToProto(protoOutputStream, LOCALES);
	protoOutputStream.write(SCREEN_LAYOUT, screenLayout);
	protoOutputStream.write(COLOR_MODE, colorMode);
	protoOutputStream.write(TOUCHSCREEN, touchscreen);
	protoOutputStream.write(KEYBOARD, keyboard);
	protoOutputStream.write(KEYBOARD_HIDDEN, keyboardHidden);
	protoOutputStream.write(HARD_KEYBOARD_HIDDEN, hardKeyboardHidden);
	protoOutputStream.write(NAVIGATION, navigation);
	protoOutputStream.write(NAVIGATION_HIDDEN, navigationHidden);
	protoOutputStream.write(ORIENTATION, orientation);
	protoOutputStream.write(UI_MODE, uiMode);
	protoOutputStream.write(SCREEN_WIDTH_DP, screenWidthDp);
	protoOutputStream.write(SCREEN_HEIGHT_DP, screenHeightDp);
	protoOutputStream.write(SMALLEST_SCREEN_WIDTH_DP, smallestScreenWidthDp);
	protoOutputStream.write(DENSITY_DPI, densityDpi);
	windowConfiguration.writeToProto(protoOutputStream, WINDOW_CONFIGURATION);
	protoOutputStream.end(token);
}

/**
 * Write full {@link android.content.ResourcesConfigurationProto} to protocol buffer output
 * stream.
 *
 * @param protoOutputStream Stream to write the Configuration object to.
 * @param fieldId           Field Id of the Configuration as defined in the parent message
 * @param metrics           Current display information
 * @hide
 */
void Configuration::writeResConfigToProto(ProtoOutputStream protoOutputStream, long fieldId,
		DisplayMetrics metrics) {
	int width, height;
	if (metrics.widthPixels >= metrics.heightPixels) {
		width = metrics.widthPixels;
		height = metrics.heightPixels;
	} else {
		//noinspection SuspiciousNameCombination
		width = metrics.heightPixels;
		//noinspection SuspiciousNameCombination
		height = metrics.widthPixels;
	}

	const long token = protoOutputStream.start(fieldId);
	writeToProto(protoOutputStream, CONFIGURATION);
	protoOutputStream.write(SDK_VERSION, Build.VERSION.RESOURCES_SDK_INT);
	protoOutputStream.write(SCREEN_WIDTH_PX, width);
	protoOutputStream.write(SCREEN_HEIGHT_PX, height);
	protoOutputStream.end(token);
}
#endif
/**
 * Convert the UI mode to a human readable format.
 * @hide
 */
std::string Configuration::uiModeToString(int uiMode) {
	switch (uiMode) {
		case UI_MODE_TYPE_UNDEFINED:
			return "UI_MODE_TYPE_UNDEFINED";
		case UI_MODE_TYPE_NORMAL:
			return "UI_MODE_TYPE_NORMAL";
		case UI_MODE_TYPE_DESK:
			return "UI_MODE_TYPE_DESK";
		case UI_MODE_TYPE_CAR:
			return "UI_MODE_TYPE_CAR";
		case UI_MODE_TYPE_TELEVISION:
			return "UI_MODE_TYPE_TELEVISION";
		case UI_MODE_TYPE_APPLIANCE:
			return "UI_MODE_TYPE_APPLIANCE";
		case UI_MODE_TYPE_WATCH:
			return "UI_MODE_TYPE_WATCH";
		case UI_MODE_TYPE_VR_HEADSET:
			return "UI_MODE_TYPE_VR_HEADSET";
		default:
			return std::to_string(uiMode);
	}
}

/**
 * Set this object to the system defaults.
 */
void Configuration::setToDefaults() {
	fontScale = 1;
	mcc = mnc = 0;
	//mLocaleList = LocaleList.getEmptyLocaleList();
	//locale = nullptr;
	userSetLocale = false;
	touchscreen = TOUCHSCREEN_UNDEFINED;
	keyboard = KEYBOARD_UNDEFINED;
	keyboardHidden = KEYBOARDHIDDEN_UNDEFINED;
	hardKeyboardHidden = HARDKEYBOARDHIDDEN_UNDEFINED;
	navigation = NAVIGATION_UNDEFINED;
	navigationHidden = NAVIGATIONHIDDEN_UNDEFINED;
	orientation = ORIENTATION_UNDEFINED;
	screenLayout = SCREENLAYOUT_UNDEFINED;
	colorMode = COLOR_MODE_UNDEFINED;
	uiMode = UI_MODE_TYPE_UNDEFINED;
	screenWidthDp = compatScreenWidthDp = SCREEN_WIDTH_DP_UNDEFINED;
	screenHeightDp = compatScreenHeightDp = SCREEN_HEIGHT_DP_UNDEFINED;
	smallestScreenWidthDp = compatSmallestScreenWidthDp = SMALLEST_SCREEN_WIDTH_DP_UNDEFINED;
	densityDpi = DENSITY_DPI_UNDEFINED;
	assetsSeq = ASSETS_SEQ_UNDEFINED;
	seq = 0;
	//windowConfiguration.setToDefaults();
}

/**
 * Set this object to completely undefined.
 * @hide
 */
void Configuration::unset() {
	setToDefaults();
	fontScale = 0;
}

/** {@hide} */
void Configuration::makeDefault() {
	setToDefaults();
}

/**
 * Copies the fields from delta into this Configuration object, keeping
 * track of which ones have changed. Any undefined fields in {@code delta}
 * are ignored and not copied in to the current Configuration.
 *
 * @return a bit mask of the changed fields, as per {@link #diff}
 */
int Configuration::updateFrom(const Configuration& delta) {
	int changed = 0;
#if 0
	if (delta.fontScale > 0 && fontScale != delta.fontScale) {
		changed |= ActivityInfo.CONFIG_FONT_SCALE;
		fontScale = delta.fontScale;
	}
	if (delta.mcc != 0 && mcc != delta.mcc) {
		changed |= ActivityInfo.CONFIG_MCC;
		mcc = delta.mcc;
	}
	if (delta.mnc != 0 && mnc != delta.mnc) {
		changed |= ActivityInfo.CONFIG_MNC;
		mnc = delta.mnc;
	}
	//fixUpLocaleList();
	//delta.fixUpLocaleList();
	if (!delta.mLocaleList.isEmpty() && !mLocaleList.equals(delta.mLocaleList)) {
		changed |= ActivityInfo.CONFIG_LOCALE;
		mLocaleList = delta.mLocaleList;
		// delta.locale can't be null, since delta.mLocaleList is not empty.
		if (!delta.locale.equals(locale)) {
			locale = (Locale) delta.locale.clone();
			// If locale has changed, then layout direction is also changed ...
			changed |= ActivityInfo.CONFIG_LAYOUT_DIRECTION;
			// ... and we need to update the layout direction (represented by the first
			// 2 most significant bits in screenLayout).
			setLayoutDirection(locale);
		}
	}
	const int deltaScreenLayoutDir = delta.screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK;
	if (deltaScreenLayoutDir != SCREENLAYOUT_LAYOUTDIR_UNDEFINED &&
			deltaScreenLayoutDir != (screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK)) {
		screenLayout = (screenLayout & ~SCREENLAYOUT_LAYOUTDIR_MASK) | deltaScreenLayoutDir;
		changed |= ActivityInfo.CONFIG_LAYOUT_DIRECTION;
	}
	if (delta.userSetLocale && (!userSetLocale || ((changed & ActivityInfo.CONFIG_LOCALE) != 0)))
	{
		changed |= ActivityInfo.CONFIG_LOCALE;
		userSetLocale = true;
	}
	if (delta.touchscreen != TOUCHSCREEN_UNDEFINED
			&& touchscreen != delta.touchscreen) {
		changed |= ActivityInfo.CONFIG_TOUCHSCREEN;
		touchscreen = delta.touchscreen;
	}
	if (delta.keyboard != KEYBOARD_UNDEFINED
			&& keyboard != delta.keyboard) {
		changed |= ActivityInfo.CONFIG_KEYBOARD;
		keyboard = delta.keyboard;
	}
	if (delta.keyboardHidden != KEYBOARDHIDDEN_UNDEFINED
			&& keyboardHidden != delta.keyboardHidden) {
		changed |= ActivityInfo.CONFIG_KEYBOARD_HIDDEN;
		keyboardHidden = delta.keyboardHidden;
	}
	if (delta.hardKeyboardHidden != HARDKEYBOARDHIDDEN_UNDEFINED
			&& hardKeyboardHidden != delta.hardKeyboardHidden) {
		changed |= ActivityInfo.CONFIG_KEYBOARD_HIDDEN;
		hardKeyboardHidden = delta.hardKeyboardHidden;
	}
	if (delta.navigation != NAVIGATION_UNDEFINED
			&& navigation != delta.navigation) {
		changed |= ActivityInfo.CONFIG_NAVIGATION;
		navigation = delta.navigation;
	}
	if (delta.navigationHidden != NAVIGATIONHIDDEN_UNDEFINED
			&& navigationHidden != delta.navigationHidden) {
		changed |= ActivityInfo.CONFIG_KEYBOARD_HIDDEN;
		navigationHidden = delta.navigationHidden;
	}
	if (delta.orientation != ORIENTATION_UNDEFINED
			&& orientation != delta.orientation) {
		changed |= ActivityInfo.CONFIG_ORIENTATION;
		orientation = delta.orientation;
	}
	if (((delta.screenLayout & SCREENLAYOUT_SIZE_MASK) != SCREENLAYOUT_SIZE_UNDEFINED)
			&& (delta.screenLayout & SCREENLAYOUT_SIZE_MASK)
			!= (screenLayout & SCREENLAYOUT_SIZE_MASK)) {
		changed |= ActivityInfo.CONFIG_SCREEN_LAYOUT;
		screenLayout = (screenLayout & ~SCREENLAYOUT_SIZE_MASK)
				| (delta.screenLayout & SCREENLAYOUT_SIZE_MASK);
	}
	if (((delta.screenLayout & SCREENLAYOUT_LONG_MASK) != SCREENLAYOUT_LONG_UNDEFINED)
			&& (delta.screenLayout & SCREENLAYOUT_LONG_MASK)
			!= (screenLayout & SCREENLAYOUT_LONG_MASK)) {
		changed |= ActivityInfo.CONFIG_SCREEN_LAYOUT;
		screenLayout = (screenLayout & ~SCREENLAYOUT_LONG_MASK)
				| (delta.screenLayout & SCREENLAYOUT_LONG_MASK);
	}
	if (((delta.screenLayout & SCREENLAYOUT_ROUND_MASK) != SCREENLAYOUT_ROUND_UNDEFINED)
			&& (delta.screenLayout & SCREENLAYOUT_ROUND_MASK)
			!= (screenLayout & SCREENLAYOUT_ROUND_MASK)) {
		changed |= ActivityInfo.CONFIG_SCREEN_LAYOUT;
		screenLayout = (screenLayout & ~SCREENLAYOUT_ROUND_MASK)
				| (delta.screenLayout & SCREENLAYOUT_ROUND_MASK);
	}
	if ((delta.screenLayout & SCREENLAYOUT_COMPAT_NEEDED)
			!= (screenLayout & SCREENLAYOUT_COMPAT_NEEDED)
			&& delta.screenLayout != 0) {
		changed |= ActivityInfo.CONFIG_SCREEN_LAYOUT;
		screenLayout = (screenLayout & ~SCREENLAYOUT_COMPAT_NEEDED)
			| (delta.screenLayout & SCREENLAYOUT_COMPAT_NEEDED);
	}

	if (((delta.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK) !=
				 COLOR_MODE_WIDE_COLOR_GAMUT_UNDEFINED)
			&& (delta.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK)
			!= (colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK)) {
		changed |= ActivityInfo.CONFIG_COLOR_MODE;
		colorMode = (colorMode & ~COLOR_MODE_WIDE_COLOR_GAMUT_MASK)
				| (delta.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK);
	}

	if (((delta.colorMode & COLOR_MODE_HDR_MASK) != COLOR_MODE_HDR_UNDEFINED)
			&& (delta.colorMode & COLOR_MODE_HDR_MASK)
			!= (colorMode & COLOR_MODE_HDR_MASK)) {
		changed |= ActivityInfo.CONFIG_COLOR_MODE;
		colorMode = (colorMode & ~COLOR_MODE_HDR_MASK)
				| (delta.colorMode & COLOR_MODE_HDR_MASK);
	}

	if (delta.uiMode != (UI_MODE_TYPE_UNDEFINED|UI_MODE_NIGHT_UNDEFINED)
			&& uiMode != delta.uiMode) {
		changed |= ActivityInfo.CONFIG_UI_MODE;
		if ((delta.uiMode&UI_MODE_TYPE_MASK) != UI_MODE_TYPE_UNDEFINED) {
			uiMode = (uiMode&~UI_MODE_TYPE_MASK)
					| (delta.uiMode&UI_MODE_TYPE_MASK);
		}
		if ((delta.uiMode&UI_MODE_NIGHT_MASK) != UI_MODE_NIGHT_UNDEFINED) {
			uiMode = (uiMode&~UI_MODE_NIGHT_MASK)
					| (delta.uiMode&UI_MODE_NIGHT_MASK);
		}
	}
	if (delta.screenWidthDp != SCREEN_WIDTH_DP_UNDEFINED
			&& screenWidthDp != delta.screenWidthDp) {
		changed |= ActivityInfo.CONFIG_SCREEN_SIZE;
		screenWidthDp = delta.screenWidthDp;
	}
	if (delta.screenHeightDp != SCREEN_HEIGHT_DP_UNDEFINED
			&& screenHeightDp != delta.screenHeightDp) {
		changed |= ActivityInfo.CONFIG_SCREEN_SIZE;
		screenHeightDp = delta.screenHeightDp;
	}
	if (delta.smallestScreenWidthDp != SMALLEST_SCREEN_WIDTH_DP_UNDEFINED
			&& smallestScreenWidthDp != delta.smallestScreenWidthDp) {
		changed |= ActivityInfo.CONFIG_SMALLEST_SCREEN_SIZE;
		smallestScreenWidthDp = delta.smallestScreenWidthDp;
	}
	if (delta.densityDpi != DENSITY_DPI_UNDEFINED &&
			densityDpi != delta.densityDpi) {
		changed |= ActivityInfo.CONFIG_DENSITY;
		densityDpi = delta.densityDpi;
	}
	if (delta.compatScreenWidthDp != SCREEN_WIDTH_DP_UNDEFINED) {
		compatScreenWidthDp = delta.compatScreenWidthDp;
	}
	if (delta.compatScreenHeightDp != SCREEN_HEIGHT_DP_UNDEFINED) {
		compatScreenHeightDp = delta.compatScreenHeightDp;
	}
	if (delta.compatSmallestScreenWidthDp != SMALLEST_SCREEN_WIDTH_DP_UNDEFINED) {
		compatSmallestScreenWidthDp = delta.compatSmallestScreenWidthDp;
	}
	if (delta.assetsSeq != ASSETS_SEQ_UNDEFINED && delta.assetsSeq != assetsSeq) {
		changed |= ActivityInfo.CONFIG_ASSETS_PATHS;
		assetsSeq = delta.assetsSeq;
	}
	if (delta.seq != 0) {
		seq = delta.seq;
	}
	if (windowConfiguration.updateFrom(delta.windowConfiguration) != 0) {
		changed |= ActivityInfo.CONFIG_WINDOW_CONFIGURATION;
	}
#endif
	return changed;
}

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
int Configuration::diff(const Configuration& delta)const {
	return diff(delta, false /* compareUndefined */, false /* publicOnly */);
}

/**
 * Returns the diff against the provided {@link Configuration} excluding values that would
 * publicly be equivalent, such as appBounds.
 * @param delta {@link Configuration} to compare to.
 *
 * TODO(b/36812336): Remove once appBounds has been moved out of Configuration.
 * {@hide}
 */
int Configuration::diffPublicOnly(const Configuration& delta)const {
	return diff(delta, false /* compareUndefined */, true /* publicOnly */);
}

/**
 * Variation of {@link #diff(Configuration)} with an option to skip checks for undefined values.
 *
 * @hide
 */
int Configuration::diff(const Configuration& delta, bool compareUndefined, bool publicOnly)const {
	int changed = 0;
#if 0
	if ((compareUndefined || delta.fontScale > 0) && fontScale != delta.fontScale) {
		changed |= ActivityInfo.CONFIG_FONT_SCALE;
	}
	if ((compareUndefined || delta.mcc != 0) && mcc != delta.mcc) {
		changed |= ActivityInfo.CONFIG_MCC;
	}
	if ((compareUndefined || delta.mnc != 0) && mnc != delta.mnc) {
		changed |= ActivityInfo.CONFIG_MNC;
	}
	fixUpLocaleList();
	delta.fixUpLocaleList();
	if ((compareUndefined || !delta.mLocaleList.isEmpty())
			&& !mLocaleList.equals(delta.mLocaleList)) {
		changed |= ActivityInfo.CONFIG_LOCALE;
		changed |= ActivityInfo.CONFIG_LAYOUT_DIRECTION;
	}
	const int deltaScreenLayoutDir = delta.screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK;
	if ((compareUndefined || deltaScreenLayoutDir != SCREENLAYOUT_LAYOUTDIR_UNDEFINED)
			&& deltaScreenLayoutDir != (screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK)) {
		changed |= ActivityInfo.CONFIG_LAYOUT_DIRECTION;
	}
	if ((compareUndefined || delta.touchscreen != TOUCHSCREEN_UNDEFINED)
			&& touchscreen != delta.touchscreen) {
		changed |= ActivityInfo.CONFIG_TOUCHSCREEN;
	}
	if ((compareUndefined || delta.keyboard != KEYBOARD_UNDEFINED)
			&& keyboard != delta.keyboard) {
		changed |= ActivityInfo.CONFIG_KEYBOARD;
	}
	if ((compareUndefined || delta.keyboardHidden != KEYBOARDHIDDEN_UNDEFINED)
			&& keyboardHidden != delta.keyboardHidden) {
		changed |= ActivityInfo.CONFIG_KEYBOARD_HIDDEN;
	}
	if ((compareUndefined || delta.hardKeyboardHidden != HARDKEYBOARDHIDDEN_UNDEFINED)
			&& hardKeyboardHidden != delta.hardKeyboardHidden) {
		changed |= ActivityInfo.CONFIG_KEYBOARD_HIDDEN;
	}
	if ((compareUndefined || delta.navigation != NAVIGATION_UNDEFINED)
			&& navigation != delta.navigation) {
		changed |= ActivityInfo.CONFIG_NAVIGATION;
	}
	if ((compareUndefined || delta.navigationHidden != NAVIGATIONHIDDEN_UNDEFINED)
			&& navigationHidden != delta.navigationHidden) {
		changed |= ActivityInfo.CONFIG_KEYBOARD_HIDDEN;
	}
	if ((compareUndefined || delta.orientation != ORIENTATION_UNDEFINED)
			&& orientation != delta.orientation) {
		changed |= ActivityInfo.CONFIG_ORIENTATION;
	}
	if ((compareUndefined || getScreenLayoutNoDirection(delta.screenLayout) !=
			(SCREENLAYOUT_SIZE_UNDEFINED | SCREENLAYOUT_LONG_UNDEFINED))
			&& getScreenLayoutNoDirection(screenLayout) !=
			getScreenLayoutNoDirection(delta.screenLayout)) {
		changed |= ActivityInfo.CONFIG_SCREEN_LAYOUT;
	}
	if ((compareUndefined || (delta.colorMode & COLOR_MODE_HDR_MASK) != COLOR_MODE_HDR_UNDEFINED)
			&& (colorMode & COLOR_MODE_HDR_MASK) !=	(delta.colorMode & COLOR_MODE_HDR_MASK)) {
		changed |= ActivityInfo.CONFIG_COLOR_MODE;
	}
	if ((compareUndefined || (delta.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK) !=
		 COLOR_MODE_WIDE_COLOR_GAMUT_UNDEFINED)	&& (colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK) !=
					(delta.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK)) {
		changed |= ActivityInfo.CONFIG_COLOR_MODE;
	}
	if ((compareUndefined || delta.uiMode != (UI_MODE_TYPE_UNDEFINED|UI_MODE_NIGHT_UNDEFINED))
			&& uiMode != delta.uiMode) {
		changed |= ActivityInfo.CONFIG_UI_MODE;
	}
	if ((compareUndefined || delta.screenWidthDp != SCREEN_WIDTH_DP_UNDEFINED)
			&& screenWidthDp != delta.screenWidthDp) {
		changed |= ActivityInfo.CONFIG_SCREEN_SIZE;
	}
	if ((compareUndefined || delta.screenHeightDp != SCREEN_HEIGHT_DP_UNDEFINED)
			&& screenHeightDp != delta.screenHeightDp) {
		changed |= ActivityInfo.CONFIG_SCREEN_SIZE;
	}
	if ((compareUndefined || delta.smallestScreenWidthDp != SMALLEST_SCREEN_WIDTH_DP_UNDEFINED)
			&& smallestScreenWidthDp != delta.smallestScreenWidthDp) {
		changed |= ActivityInfo.CONFIG_SMALLEST_SCREEN_SIZE;
	}
	if ((compareUndefined || delta.densityDpi != DENSITY_DPI_UNDEFINED)
			&& densityDpi != delta.densityDpi) {
		changed |= ActivityInfo.CONFIG_DENSITY;
	}
	if ((compareUndefined || delta.assetsSeq != ASSETS_SEQ_UNDEFINED)
			&& assetsSeq != delta.assetsSeq) {
		changed |= ActivityInfo.CONFIG_ASSETS_PATHS;
	}

	// WindowConfiguration differences aren't considered public...
	if (!publicOnly && windowConfiguration.diff(delta.windowConfiguration, compareUndefined) != 0) {
		changed |= ActivityInfo.CONFIG_WINDOW_CONFIGURATION;
	}
#endif
	return changed;
}

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
bool Configuration::needNewResources(int configChanges, int interestingChanges) {
	// CONFIG_ASSETS_PATHS and CONFIG_FONT_SCALE are higher level configuration changes that
	// all resources are subject to change with.
	interestingChanges = 0;//interestingChanges | ActivityInfo::CONFIG_ASSETS_PATHS | ActivityInfo::CONFIG_FONT_SCALE;
	return (configChanges & interestingChanges) != 0;
}

/**
 * @hide Return true if the sequence of 'other' is better than this.  Assumes
 * that 'this' is your current sequence and 'other' is a new one you have
 * received some how and want to compare with what you have.
 */
bool Configuration::isOtherSeqNewer(const Configuration& other) const{
	/*if (other == nullptr) {
		// Sanity check.
		return false;
	}*/
	if (other.seq == 0) {
		// If the other sequence is not specified, then we must assume
		// it is newer since we don't know any better.
		return true;
	}
	if (seq == 0) {
		// If this sequence is not specified, then we also consider the
		// other is better.  Yes we have a preference for other.  Sue us.
		return true;
	}
	const int diff = other.seq - seq;
	if (diff > 0x10000) {
		// If there has been a sufficiently large jump, assume the
		// sequence has wrapped around.
		return false;
	}
	return diff > 0;
}

void Configuration::writeToParcel(Parcel& dest, int flags) {
	dest.writeFloat(fontScale);
	dest.writeInt(mcc);
	dest.writeInt(mnc);

	fixUpLocaleList();
	//dest.writeParcelable(mLocaleList, flags);

	if(userSetLocale) {
		dest.writeInt(1);
	} else {
		dest.writeInt(0);
	}
	dest.writeInt(touchscreen);
	dest.writeInt(keyboard);
	dest.writeInt(keyboardHidden);
	dest.writeInt(hardKeyboardHidden);
	dest.writeInt(navigation);
	dest.writeInt(navigationHidden);
	dest.writeInt(orientation);
	dest.writeInt(screenLayout);
	dest.writeInt(colorMode);
	dest.writeInt(uiMode);
	dest.writeInt(screenWidthDp);
	dest.writeInt(screenHeightDp);
	dest.writeInt(smallestScreenWidthDp);
	dest.writeInt(densityDpi);
	dest.writeInt(compatScreenWidthDp);
	dest.writeInt(compatScreenHeightDp);
	dest.writeInt(compatSmallestScreenWidthDp);
	//dest.writeValue(windowConfiguration);
	dest.writeInt(assetsSeq);
	dest.writeInt(seq);
}

void Configuration::readFromParcel(Parcel& source) {
	fontScale = source.readFloat();
	mcc = source.readInt();
	mnc = source.readInt();

	//mLocaleList = source.readParcelable(LocaleList.class.getClassLoader());
	//locale = mLocaleList.get(0);

	userSetLocale = (source.readInt()==1);
	touchscreen = source.readInt();
	keyboard = source.readInt();
	keyboardHidden = source.readInt();
	hardKeyboardHidden = source.readInt();
	navigation = source.readInt();
	navigationHidden = source.readInt();
	orientation = source.readInt();
	screenLayout = source.readInt();
	colorMode = source.readInt();
	uiMode = source.readInt();
	screenWidthDp = source.readInt();
	screenHeightDp = source.readInt();
	smallestScreenWidthDp = source.readInt();
	densityDpi = source.readInt();
	compatScreenWidthDp = source.readInt();
	compatScreenHeightDp = source.readInt();
	compatSmallestScreenWidthDp = source.readInt();
	//windowConfiguration.setTo((WindowConfiguration) source.readValue(null));
	assetsSeq = source.readInt();
	seq = source.readInt();
}

/**
 * Construct this Configuration object, reading from the Parcel.
 */
Configuration::Configuration(Parcel& source) {
	readFromParcel(source);
}

int Configuration::compareTo(const Configuration& that) {
	int n;
	float a = this->fontScale;
	float b = that.fontScale;
	if (a < b) return -1;
	if (a > b) return 1;
	n = this->mcc - that.mcc;
	if (n != 0) return n;
	n = this->mnc - that.mnc;
	if (n != 0) return n;

	/*fixUpLocaleList();
	that.fixUpLocaleList();
	// for backward compatibility, we consider an empty locale list to be greater
	// than any non-empty locale list.
	if (this->mLocaleList.isEmpty()) {
		if (!that.mLocaleList.isEmpty()) return 1;
	} else if (that.mLocaleList.isEmpty()) {
		return -1;
	} else {
		const int minSize = std::min(this->mLocaleList.size(), that.mLocaleList.size());
		for (int i = 0; i < minSize; ++i) {
			Locale thisLocale = this->mLocaleList.get(i);
			Locale thatLocale = that.mLocaleList.get(i);
			n = thisLocale.getLanguage().compareTo(thatLocale.getLanguage());
			if (n != 0) return n;
			n = thisLocale.getCountry().compareTo(thatLocale.getCountry());
			if (n != 0) return n;
			n = thisLocale.getVariant().compareTo(thatLocale.getVariant());
			if (n != 0) return n;
			n = thisLocale.toLanguageTag().compareTo(thatLocale.toLanguageTag());
			if (n != 0) return n;
		}
		n = this->mLocaleList.size() - that.mLocaleList.size();
		if (n != 0) return n;
	}*/

	n = this->touchscreen - that.touchscreen;
	if (n != 0) return n;
	n = this->keyboard - that.keyboard;
	if (n != 0) return n;
	n = this->keyboardHidden - that.keyboardHidden;
	if (n != 0) return n;
	n = this->hardKeyboardHidden - that.hardKeyboardHidden;
	if (n != 0) return n;
	n = this->navigation - that.navigation;
	if (n != 0) return n;
	n = this->navigationHidden - that.navigationHidden;
	if (n != 0) return n;
	n = this->orientation - that.orientation;
	if (n != 0) return n;
	n = this->colorMode - that.colorMode;
	if (n != 0) return n;
	n = this->screenLayout - that.screenLayout;
	if (n != 0) return n;
	n = this->uiMode - that.uiMode;
	if (n != 0) return n;
	n = this->screenWidthDp - that.screenWidthDp;
	if (n != 0) return n;
	n = this->screenHeightDp - that.screenHeightDp;
	if (n != 0) return n;
	n = this->smallestScreenWidthDp - that.smallestScreenWidthDp;
	if (n != 0) return n;
	n = this->densityDpi - that.densityDpi;
	if (n != 0) return n;
	n = this->assetsSeq - that.assetsSeq;
	if (n != 0) return n;
	//n = windowConfiguration.compareTo(that.windowConfiguration);
	if (n != 0) return n;

	// if (n != 0) return n;
	return n;
}

/*bool Configuration::equals(const Configuration& that) {
	if (that == null) return false;
	if (that == this) return true;
	return this->compareTo(that) == 0;
}*/


/**
 * Get the locale list. This is the preferred way for getting the locales (instead of using
 * the direct accessor to {@link #locale}, which would only provide the primary locale).
 *
 * @return The locale list.
 */
#if 0
LocaleList* Configuration::getLocales() {
	fixUpLocaleList();
	return mLocaleList;
}

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
void Configuration::setLocales(LocaleList* locales) {
	mLocaleList = locales == nullptr ? LocaleList.getEmptyLocaleList() : locales;
	locale = mLocaleList.get(0);
	setLayoutDirection(locale);
}

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
void Configuration::setLocale(Locale* loc) {
	setLocales(loc == nullptr ? LocaleList.getEmptyLocaleList() : new LocaleList(loc));
}
#endif
/**
 * @hide
 *
 * Clears the locale without changing layout direction.
 */
void Configuration::clearLocales() {
	//mLocaleList = LocaleList.getEmptyLocaleList();
	//locale = null;
}

/**
 * Return the layout direction. Will be either {@link View#LAYOUT_DIRECTION_LTR} or
 * {@link View#LAYOUT_DIRECTION_RTL}.
 *
 * @return Returns {@link View#LAYOUT_DIRECTION_RTL} if the configuration
 * is {@link #SCREENLAYOUT_LAYOUTDIR_RTL}, otherwise {@link View#LAYOUT_DIRECTION_LTR}.
 */
int Configuration::getLayoutDirection() const{
	return (screenLayout&SCREENLAYOUT_LAYOUTDIR_MASK) == SCREENLAYOUT_LAYOUTDIR_RTL
			? View::LAYOUT_DIRECTION_RTL : View::LAYOUT_DIRECTION_LTR;
}

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
#if 0
void Configuration::setLayoutDirection(Locale loc) {
	// There is a "1" difference between the configuration values for
	// layout direction and View constants for layout direction, just add "1".
	const int layoutDirection = 1 + TextUtils.getLayoutDirectionFromLocale(loc);
	screenLayout = (screenLayout&~SCREENLAYOUT_LAYOUTDIR_MASK)|(layoutDirection << SCREENLAYOUT_LAYOUTDIR_SHIFT);
}
#endif

int Configuration::getScreenLayoutNoDirection(int screenLayout){
	return screenLayout&~SCREENLAYOUT_LAYOUTDIR_MASK;
}

/**
 * Return whether the screen has a round shape. Apps may choose to change styling based
 * on this property, such as the alignment or layout of text or informational icons.
 *
 * @return true if the screen is rounded, false otherwise
 */
bool Configuration::isScreenRound()const {
	return (screenLayout & SCREENLAYOUT_ROUND_MASK) == SCREENLAYOUT_ROUND_YES;
}

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
bool Configuration::isScreenWideColorGamut()const {
	return (colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK) == COLOR_MODE_WIDE_COLOR_GAMUT_YES;
}

/**
 * Return whether the screen has a high dynamic range.
 *
 * @return true if the screen has a high dynamic range, false otherwise
 */
bool Configuration::isScreenHdr()const {
	return (colorMode & COLOR_MODE_HDR_MASK) == COLOR_MODE_HDR_YES;
}

#if 0
std::string localesToResourceQualifier(LocaleList locs) {
    std::ostringstreamr sb;
	for (int i = 0; i < locs.size(); i++) {
		final Locale loc = locs.get(i);
		final int l = loc.getLanguage().length();
		if (l == 0) {
			continue;
		}
		final int s = loc.getScript().length();
		final int c = loc.getCountry().length();
		final int v = loc.getVariant().length();
		// We ignore locale extensions, since they are not supported by AAPT

		if (sb.length() != 0) {
			sb<<",";
		}
		if (l == 2 && s == 0 && (c == 0 || c == 2) && v == 0) {
			// Traditional locale format: xx or xx-rYY
			sb<<loc.getLanguage();
			if (c == 2) {
				sb<<"-r"<<loc.getCountry();
			}
		} else {
			sb<<"b+";
			sb<<loc.getLanguage();
			if (s != 0) {
				sb<<"+";
				sb<<loc.getScript();
			}
			if (c != 0) {
				sb<<"+";
				sb<<loc.getCountry();
			}
			if (v != 0) {
				sb<<"+";
				sb<<loc.getVariant();
			}
		}
	}
	return sb.toString();
}
#endif

/**
 * Returns a string representation of the configuration that can be parsed
 * by build tools (like AAPT), without display metrics included
 *
 * @hide
 */
std::string Configuration::resourceQualifierString(const Configuration& config) {
	return resourceQualifierString(config, nullptr);
}

/**
 * Returns a string representation of the configuration that can be parsed
 * by build tools (like AAPT).
 *
 * @hide
 */
std::string Configuration::resourceQualifierString(const Configuration& config,const DisplayMetrics* metrics) {
	std::vector<std::string> parts;

	if (config.mcc != 0) {
		parts.push_back("mcc" + std::to_string(config.mcc));
		if (config.mnc != 0) {
			parts.push_back("mnc" + std::to_string(config.mnc));
		}
	}

	/*if (!config.mLocaleList.isEmpty()) {
        std::string resourceQualifier = localesToResourceQualifier(config.mLocaleList);
		if (!resourceQualifier.empty()) {
			parts.push_back(resourceQualifier);
		}
	}*/

	switch (config.screenLayout & Configuration::SCREENLAYOUT_LAYOUTDIR_MASK) {
        case Configuration::SCREENLAYOUT_LAYOUTDIR_LTR:
			parts.push_back("ldltr");
			break;
        case Configuration::SCREENLAYOUT_LAYOUTDIR_RTL:
			parts.push_back("ldrtl");
			break;
		default:
			break;
	}

	if (config.smallestScreenWidthDp != 0) {
		parts.push_back("sw" + std::to_string(config.smallestScreenWidthDp) + "dp");
	}

	if (config.screenWidthDp != 0) {
		parts.push_back("w" + std::to_string(config.screenWidthDp) + "dp");
	}

	if (config.screenHeightDp != 0) {
		parts.push_back("h" + std::to_string(config.screenHeightDp) + "dp");
	}

	switch (config.screenLayout & Configuration::SCREENLAYOUT_SIZE_MASK) {
        case Configuration::SCREENLAYOUT_SIZE_SMALL:
			parts.push_back("small");
			break;
        case Configuration::SCREENLAYOUT_SIZE_NORMAL:
			parts.push_back("normal");
			break;
        case Configuration::SCREENLAYOUT_SIZE_LARGE:
			parts.push_back("large");
			break;
        case Configuration::SCREENLAYOUT_SIZE_XLARGE:
			parts.push_back("xlarge");
			break;
		default:
			break;
	}

	switch (config.screenLayout & Configuration::SCREENLAYOUT_LONG_MASK) {
        case Configuration::SCREENLAYOUT_LONG_YES:
			parts.push_back("long");
			break;
        case Configuration::SCREENLAYOUT_LONG_NO:
			parts.push_back("notlong");
			break;
		default:
			break;
	}

	switch (config.screenLayout & Configuration::SCREENLAYOUT_ROUND_MASK) {
        case Configuration::SCREENLAYOUT_ROUND_YES:
			parts.push_back("round");
			break;
        case Configuration::SCREENLAYOUT_ROUND_NO:
			parts.push_back("notround");
			break;
		default:
			break;
	}

	switch (config.colorMode & Configuration::COLOR_MODE_HDR_MASK) {
        case Configuration::COLOR_MODE_HDR_YES:
			parts.push_back("highdr");
			break;
        case Configuration::COLOR_MODE_HDR_NO:
			parts.push_back("lowdr");
			break;
		default:
			break;
	}

	switch (config.colorMode & Configuration::COLOR_MODE_WIDE_COLOR_GAMUT_MASK) {
        case Configuration::COLOR_MODE_WIDE_COLOR_GAMUT_YES:
			parts.push_back("widecg");
			break;
        case Configuration::COLOR_MODE_WIDE_COLOR_GAMUT_NO:
			parts.push_back("nowidecg");
			break;
		default:
			break;
	}

	switch (config.orientation) {
        case Configuration::ORIENTATION_LANDSCAPE:
			parts.push_back("land");
			break;
        case Configuration::ORIENTATION_PORTRAIT:
			parts.push_back("port");
			break;
		default:
			break;
	}

	switch (config.uiMode & Configuration::UI_MODE_TYPE_MASK) {
        case Configuration::UI_MODE_TYPE_APPLIANCE:
			parts.push_back("appliance");
			break;
        case Configuration::UI_MODE_TYPE_DESK:
			parts.push_back("desk");
			break;
        case Configuration::UI_MODE_TYPE_TELEVISION:
			parts.push_back("television");
			break;
        case Configuration::UI_MODE_TYPE_CAR:
			parts.push_back("car");
			break;
        case Configuration::UI_MODE_TYPE_WATCH:
			parts.push_back("watch");
			break;
        case Configuration::UI_MODE_TYPE_VR_HEADSET:
			parts.push_back("vrheadset");
			break;
		default:
			break;
	}

	switch (config.uiMode & Configuration::UI_MODE_NIGHT_MASK) {
        case Configuration::UI_MODE_NIGHT_YES:
			parts.push_back("night");
			break;
        case Configuration::UI_MODE_NIGHT_NO:
			parts.push_back("notnight");
			break;
		default:
			break;
	}

	switch (config.densityDpi) {
		case DENSITY_DPI_UNDEFINED:
			break;
		case 120:
			parts.push_back("ldpi");
			break;
		case 160:
			parts.push_back("mdpi");
			break;
		case 213:
			parts.push_back("tvdpi");
			break;
		case 240:
			parts.push_back("hdpi");
			break;
		case 320:
			parts.push_back("xhdpi");
			break;
		case 480:
			parts.push_back("xxhdpi");
			break;
		case 640:
			parts.push_back("xxxhdpi");
			break;
		case DENSITY_DPI_ANY:
			parts.push_back("anydpi");
			break;
		case DENSITY_DPI_NONE:
			parts.push_back("nodpi");
			break;
		default:
			parts.push_back(std::to_string(config.densityDpi) + "dpi");
			break;
	}

	switch (config.touchscreen) {
        case Configuration::TOUCHSCREEN_NOTOUCH:
			parts.push_back("notouch");
			break;
        case Configuration::TOUCHSCREEN_FINGER:
			parts.push_back("finger");
			break;
		default:
			break;
	}

	switch (config.keyboardHidden) {
        case Configuration::KEYBOARDHIDDEN_NO:
			parts.push_back("keysexposed");
			break;
        case Configuration::KEYBOARDHIDDEN_YES:
			parts.push_back("keyshidden");
			break;
        case Configuration::KEYBOARDHIDDEN_SOFT:
			parts.push_back("keyssoft");
			break;
		default:
			break;
	}

	switch (config.keyboard) {
        case Configuration::KEYBOARD_NOKEYS:
			parts.push_back("nokeys");
			break;
        case Configuration::KEYBOARD_QWERTY:
			parts.push_back("qwerty");
			break;
        case Configuration::KEYBOARD_12KEY:
			parts.push_back("12key");
			break;
		default:
			break;
	}

	switch (config.navigationHidden) {
        case Configuration::NAVIGATIONHIDDEN_NO:
			parts.push_back("navexposed");
			break;
        case Configuration::NAVIGATIONHIDDEN_YES:
			parts.push_back("navhidden");
			break;
		default:
			break;
	}

	switch (config.navigation) {
        case Configuration::NAVIGATION_NONAV:
			parts.push_back("nonav");
			break;
        case Configuration::NAVIGATION_DPAD:
			parts.push_back("dpad");
			break;
        case Configuration::NAVIGATION_TRACKBALL:
			parts.push_back("trackball");
			break;
        case Configuration::NAVIGATION_WHEEL:
			parts.push_back("wheel");
			break;
		default:
			break;
	}

	if (metrics != nullptr) {
		int width, height;
		if (metrics->widthPixels >= metrics->heightPixels) {
			width = metrics->widthPixels;
			height = metrics->heightPixels;
		} else {
			//noinspection SuspiciousNameCombination
			width = metrics->heightPixels;
			//noinspection SuspiciousNameCombination
			height = metrics->widthPixels;
		}
		parts.push_back(std::to_string(width) + "x" + std::to_string(height));
	}

	parts.push_back(std::string("v") + Build::VERSION::Release);//RESOURCES_SDK_INT);
	return TextUtils::join("-", parts);
}

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
Configuration Configuration::generateDelta(const Configuration& base,const Configuration& change) {
	Configuration delta;
	if (base.fontScale != change.fontScale) {
		delta.fontScale = change.fontScale;
	}

	if (base.mcc != change.mcc) {
		delta.mcc = change.mcc;
	}

	if (base.mnc != change.mnc) {
		delta.mnc = change.mnc;
	}

	/*base.fixUpLocaleList();
	change.fixUpLocaleList();
	if (!base.mLocaleList.equals(change.mLocaleList))  {
		delta.mLocaleList = change.mLocaleList;
		delta.locale = change.locale;
	}*/

	if (base.touchscreen != change.touchscreen) {
		delta.touchscreen = change.touchscreen;
	}

	if (base.keyboard != change.keyboard) {
		delta.keyboard = change.keyboard;
	}

	if (base.keyboardHidden != change.keyboardHidden) {
		delta.keyboardHidden = change.keyboardHidden;
	}

	if (base.navigation != change.navigation) {
		delta.navigation = change.navigation;
	}

	if (base.navigationHidden != change.navigationHidden) {
		delta.navigationHidden = change.navigationHidden;
	}

	if (base.orientation != change.orientation) {
		delta.orientation = change.orientation;
	}

	if ((base.screenLayout & SCREENLAYOUT_SIZE_MASK) !=
			(change.screenLayout & SCREENLAYOUT_SIZE_MASK)) {
		delta.screenLayout |= change.screenLayout & SCREENLAYOUT_SIZE_MASK;
	}

	if ((base.screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK) !=
			(change.screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK)) {
		delta.screenLayout |= change.screenLayout & SCREENLAYOUT_LAYOUTDIR_MASK;
	}

	if ((base.screenLayout & SCREENLAYOUT_LONG_MASK) !=
			(change.screenLayout & SCREENLAYOUT_LONG_MASK)) {
		delta.screenLayout |= change.screenLayout & SCREENLAYOUT_LONG_MASK;
	}

	if ((base.screenLayout & SCREENLAYOUT_ROUND_MASK) !=
			(change.screenLayout & SCREENLAYOUT_ROUND_MASK)) {
		delta.screenLayout |= change.screenLayout & SCREENLAYOUT_ROUND_MASK;
	}

	if ((base.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK) !=
			(change.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK)) {
		delta.colorMode |= change.colorMode & COLOR_MODE_WIDE_COLOR_GAMUT_MASK;
	}

	if ((base.colorMode & COLOR_MODE_HDR_MASK) !=
			(change.colorMode & COLOR_MODE_HDR_MASK)) {
		delta.colorMode |= change.colorMode & COLOR_MODE_HDR_MASK;
	}

	if ((base.uiMode & UI_MODE_TYPE_MASK) != (change.uiMode & UI_MODE_TYPE_MASK)) {
		delta.uiMode |= change.uiMode & UI_MODE_TYPE_MASK;
	}

	if ((base.uiMode & UI_MODE_NIGHT_MASK) != (change.uiMode & UI_MODE_NIGHT_MASK)) {
		delta.uiMode |= change.uiMode & UI_MODE_NIGHT_MASK;
	}

	if (base.screenWidthDp != change.screenWidthDp) {
		delta.screenWidthDp = change.screenWidthDp;
	}

	if (base.screenHeightDp != change.screenHeightDp) {
		delta.screenHeightDp = change.screenHeightDp;
	}

	if (base.smallestScreenWidthDp != change.smallestScreenWidthDp) {
		delta.smallestScreenWidthDp = change.smallestScreenWidthDp;
	}

	if (base.densityDpi != change.densityDpi) {
		delta.densityDpi = change.densityDpi;
	}

	if (base.assetsSeq != change.assetsSeq) {
		delta.assetsSeq = change.assetsSeq;
	}

	/*if (!base.windowConfiguration.equals(change.windowConfiguration)) {
		delta.windowConfiguration.setTo(change.windowConfiguration);
	}*/
	return delta;
}

    static const char* XML_ATTR_FONT_SCALE = "fs";
    static const char* XML_ATTR_MCC = "mcc";
    static const char* XML_ATTR_MNC = "mnc";
    static const char* XML_ATTR_LOCALES = "locales";
    static const char* XML_ATTR_TOUCHSCREEN = "touch";
    static const char* XML_ATTR_KEYBOARD = "key";
    static const char* XML_ATTR_KEYBOARD_HIDDEN = "keyHid";
    static const char* XML_ATTR_HARD_KEYBOARD_HIDDEN = "hardKeyHid";
    static const char* XML_ATTR_NAVIGATION = "nav";
    static const char* XML_ATTR_NAVIGATION_HIDDEN = "navHid";
    static const char* XML_ATTR_ORIENTATION = "ori";
    static const char* XML_ATTR_ROTATION = "rot";
    static const char* XML_ATTR_SCREEN_LAYOUT = "scrLay";
    static const char* XML_ATTR_COLOR_MODE = "clrMod";
    static const char* XML_ATTR_UI_MODE = "ui";
    static const char* XML_ATTR_SCREEN_WIDTH = "width";
    static const char* XML_ATTR_SCREEN_HEIGHT = "height";
    static const char* XML_ATTR_SMALLEST_WIDTH = "sw";
    static const char* XML_ATTR_DENSITY = "density";
    static const char* XML_ATTR_APP_BOUNDS = "app_bounds";
/**
 * Reads the attributes corresponding to Configuration member fields from the Xml parser.
 * The parser is expected to be on a tag which has Configuration attributes.
 *
 * @param parser The Xml parser from which to read attributes.
 * @param configOut The Configuration to populate from the Xml attributes.
 * {@hide}
 */
#if 0
void Configuration::readXmlAttrs(XmlPullParser& parser, Configuration& configOut)
		throws XmlPullParserException, IOException {
	configOut.fontScale = Float.intBitsToFloat(
			XmlUtils.readIntAttribute(parser, XML_ATTR_FONT_SCALE, 0));
	configOut.mcc = XmlUtils.readIntAttribute(parser, XML_ATTR_MCC, 0);
	configOut.mnc = XmlUtils.readIntAttribute(parser, XML_ATTR_MNC, 0);

	String localesStr = XmlUtils.readStringAttribute(parser, XML_ATTR_LOCALES);
	configOut.mLocaleList = LocaleList.forLanguageTags(localesStr);
	configOut.locale = configOut.mLocaleList.get(0);

	configOut.touchscreen = XmlUtils.readIntAttribute(parser, XML_ATTR_TOUCHSCREEN,
			TOUCHSCREEN_UNDEFINED);
	configOut.keyboard = XmlUtils.readIntAttribute(parser, XML_ATTR_KEYBOARD,
			KEYBOARD_UNDEFINED);
	configOut.keyboardHidden = XmlUtils.readIntAttribute(parser, XML_ATTR_KEYBOARD_HIDDEN,
			KEYBOARDHIDDEN_UNDEFINED);
	configOut.hardKeyboardHidden =
			XmlUtils.readIntAttribute(parser, XML_ATTR_HARD_KEYBOARD_HIDDEN,
					HARDKEYBOARDHIDDEN_UNDEFINED);
	configOut.navigation = XmlUtils.readIntAttribute(parser, XML_ATTR_NAVIGATION,
			NAVIGATION_UNDEFINED);
	configOut.navigationHidden = XmlUtils.readIntAttribute(parser, XML_ATTR_NAVIGATION_HIDDEN,
			NAVIGATIONHIDDEN_UNDEFINED);
	configOut.orientation = XmlUtils.readIntAttribute(parser, XML_ATTR_ORIENTATION,
			ORIENTATION_UNDEFINED);
	configOut.screenLayout = XmlUtils.readIntAttribute(parser, XML_ATTR_SCREEN_LAYOUT,
			SCREENLAYOUT_UNDEFINED);
	configOut.colorMode = XmlUtils.readIntAttribute(parser, XML_ATTR_COLOR_MODE,
			COLOR_MODE_UNDEFINED);
	configOut.uiMode = XmlUtils.readIntAttribute(parser, XML_ATTR_UI_MODE, 0);
	configOut.screenWidthDp = XmlUtils.readIntAttribute(parser, XML_ATTR_SCREEN_WIDTH,
			SCREEN_WIDTH_DP_UNDEFINED);
	configOut.screenHeightDp = XmlUtils.readIntAttribute(parser, XML_ATTR_SCREEN_HEIGHT,
			SCREEN_HEIGHT_DP_UNDEFINED);
	configOut.smallestScreenWidthDp =
			XmlUtils.readIntAttribute(parser, XML_ATTR_SMALLEST_WIDTH,
					SMALLEST_SCREEN_WIDTH_DP_UNDEFINED);
	configOut.densityDpi = XmlUtils.readIntAttribute(parser, XML_ATTR_DENSITY,
			DENSITY_DPI_UNDEFINED);

	// For persistence, we don't care about assetsSeq and WindowConfiguration, so do not read it
	// out.
}


/**
 * Writes the Configuration's member fields as attributes into the XmlSerializer.
 * The serializer is expected to have already started a tag so that attributes can be
 * immediately written.
 *
 * @param xml The serializer to which to write the attributes.
 * @param config The Configuration whose member fields to write.
 * {@hide}
 */
void Configuration::writeXmlAttrs(XmlSerializer xml, Configuration config){
	XmlUtils.writeIntAttribute(xml, XML_ATTR_FONT_SCALE,
			Float.floatToIntBits(config.fontScale));
	if (config.mcc != 0) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_MCC, config.mcc);
	}
	if (config.mnc != 0) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_MNC, config.mnc);
	}
	config.fixUpLocaleList();
	if (!config.mLocaleList.isEmpty()) {
	   XmlUtils.writeStringAttribute(xml, XML_ATTR_LOCALES, config.mLocaleList.toLanguageTags());
	}
	if (config.touchscreen != TOUCHSCREEN_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_TOUCHSCREEN, config.touchscreen);
	}
	if (config.keyboard != KEYBOARD_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_KEYBOARD, config.keyboard);
	}
	if (config.keyboardHidden != KEYBOARDHIDDEN_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_KEYBOARD_HIDDEN, config.keyboardHidden);
	}
	if (config.hardKeyboardHidden != HARDKEYBOARDHIDDEN_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_HARD_KEYBOARD_HIDDEN,
				config.hardKeyboardHidden);
	}
	if (config.navigation != NAVIGATION_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_NAVIGATION, config.navigation);
	}
	if (config.navigationHidden != NAVIGATIONHIDDEN_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_NAVIGATION_HIDDEN, config.navigationHidden);
	}
	if (config.orientation != ORIENTATION_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_ORIENTATION, config.orientation);
	}
	if (config.screenLayout != SCREENLAYOUT_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_SCREEN_LAYOUT, config.screenLayout);
	}
	if (config.colorMode != COLOR_MODE_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_COLOR_MODE, config.colorMode);
	}
	if (config.uiMode != 0) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_UI_MODE, config.uiMode);
	}
	if (config.screenWidthDp != SCREEN_WIDTH_DP_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_SCREEN_WIDTH, config.screenWidthDp);
	}
	if (config.screenHeightDp != SCREEN_HEIGHT_DP_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_SCREEN_HEIGHT, config.screenHeightDp);
	}
	if (config.smallestScreenWidthDp != SMALLEST_SCREEN_WIDTH_DP_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_SMALLEST_WIDTH, config.smallestScreenWidthDp);
	}
	if (config.densityDpi != DENSITY_DPI_UNDEFINED) {
		XmlUtils.writeIntAttribute(xml, XML_ATTR_DENSITY, config.densityDpi);
	}

	// For persistence, we do not care about assetsSeq and window configuration, so do not write
	// it out.
}
#endif
}/*endof namespace*/
