#ifndef __FRAMEWORK_STYLEABLE_H__
#define __FRAMEWORK_STYLEABLE_H__
#include <cstdint>
namespace cdroid {

// Framework android:attr resource IDs (from AOSP public.xml), grouped by class.
// Each class has its own namespace. The styleable namespace below defines the
// R.styleable.Class[] arrays (index → attr ID mapping) for TypedArray.

namespace fw_attr {
    namespace View {
        constexpr uint32_t background            = 0x010100d4;
        constexpr uint32_t padding               = 0x010100d5;
        constexpr uint32_t paddingLeft           = 0x010100d6;
        constexpr uint32_t paddingTop            = 0x010100d7;
        constexpr uint32_t paddingRight          = 0x010100d8;
        constexpr uint32_t paddingBottom         = 0x010100d9;
        constexpr uint32_t paddingStart          = 0x010103b3;
        constexpr uint32_t paddingEnd            = 0x010103b4;
        constexpr uint32_t minWidth              = 0x0101013f;
        constexpr uint32_t minHeight             = 0x01010140;
        constexpr uint32_t visibility            = 0x010100dc;
        constexpr uint32_t id                    = 0x010100d0;
        constexpr uint32_t tag                   = 0x010100d1;
        constexpr uint32_t scrollX               = 0x010100d2;
        constexpr uint32_t scrollY               = 0x010100d3;
        constexpr uint32_t fitsSystemWindows     = 0x010100dd;
        constexpr uint32_t scrollbars            = 0x010100de;
        constexpr uint32_t fadingEdge            = 0x010100df;
        constexpr uint32_t fadingEdgeLength      = 0x010100e0;
        constexpr uint32_t nextFocusLeft         = 0x010100e1;
        constexpr uint32_t nextFocusRight        = 0x010100e2;
        constexpr uint32_t nextFocusUp           = 0x010100e3;
        constexpr uint32_t nextFocusDown         = 0x010100e4;
        constexpr uint32_t clickable             = 0x010100e5;
        constexpr uint32_t longClickable         = 0x010100e6;
        constexpr uint32_t saveEnabled           = 0x010100e7;
        constexpr uint32_t drawingCacheQuality   = 0x010100e8;
        constexpr uint32_t duplicateParentState  = 0x010100e9;
        constexpr uint32_t scrollbarStyle        = 0x0101007f;
        constexpr uint32_t scrollbarSize         = 0x01010063;
        constexpr uint32_t scrollbarFadeDuration = 0x010102a8;
        constexpr uint32_t scrollbarDefaultDelayBeforeFade = 0x010102a9;
        constexpr uint32_t fadeScrollbars        = 0x010102aa;
        constexpr uint32_t focusable             = 0x010100da;
        constexpr uint32_t focusableInTouchMode  = 0x010100db;
        constexpr uint32_t soundEffectsEnabled   = 0x01010215;
        constexpr uint32_t hapticFeedbackEnabled = 0x0101025e;
        constexpr uint32_t onClick               = 0x0101026f;
        constexpr uint32_t contentDescription    = 0x01010273;
        constexpr uint32_t isScrollContainer     = 0x0101024e;
        constexpr uint32_t foregroundGravity     = 0x01010200;
        constexpr uint32_t overScrollMode        = 0x010102c1;
        constexpr uint32_t filterTouchesWhenObscured = 0x010102c4;
        constexpr uint32_t keepScreenOn          = 0x01010216;
        constexpr uint32_t layerType             = 0x01010354;
        constexpr uint32_t layoutDirection       = 0x010103b2;
        constexpr uint32_t textDirection         = 0x010103b0;
        constexpr uint32_t textAlignment         = 0x010103b1;
        constexpr uint32_t importantForAccessibility = 0x010103aa;
        constexpr uint32_t requiresFadingEdge    = 0x010103a5;
        constexpr uint32_t rotation              = 0x01010326;
        constexpr uint32_t rotationX             = 0x01010327;
        constexpr uint32_t rotationY             = 0x01010328;
        constexpr uint32_t scaleX                = 0x01010324;
        constexpr uint32_t scaleY                = 0x01010325;
        constexpr uint32_t transformPivotX       = 0x01010320;
        constexpr uint32_t transformPivotY       = 0x01010321;
        constexpr uint32_t translationX          = 0x01010322;
        constexpr uint32_t translationY          = 0x01010323;
        constexpr uint32_t translationZ          = 0x010103fa;
        constexpr uint32_t alpha                 = 0x0101031f;
        constexpr uint32_t elevation             = 0x01010440;
        constexpr uint32_t transitionName        = 0x01010400;
        constexpr uint32_t stateListAnimator     = 0x01010448;
        constexpr uint32_t nestedScrollingEnabled = 0x01010436;
        constexpr uint32_t backgroundTint        = 0x0101046b;
        constexpr uint32_t backgroundTintMode    = 0x0101046c;
        constexpr uint32_t foregroundTint        = 0x0101046d;
        constexpr uint32_t foregroundTintMode    = 0x0101046e;
        constexpr uint32_t outlineProvider       = 0x010104b8;
        constexpr uint32_t verticalScrollbarPosition = 0x01010334;
        constexpr uint32_t scrollIndicators      = 0x010104e6;
        constexpr uint32_t nextFocusForward      = 0x0101033c;
        constexpr uint32_t nextClusterForward    = 0x01010542;
        constexpr uint32_t keyboardNavigationCluster = 0x01010540;
        constexpr uint32_t focusedByDefault      = 0x01010544;
        constexpr uint32_t allowClickWhenDisabled = 0x01010618;
        constexpr uint32_t enabled               = 0x0101000e;
        constexpr uint32_t orientation           = 0x010100c4;
        constexpr uint32_t gravity               = 0x010100af;
        // Layout params
        constexpr uint32_t layout_width          = 0x010100f4;
        constexpr uint32_t layout_height         = 0x010100f5;
        constexpr uint32_t layout_margin         = 0x010100f6;
        constexpr uint32_t layout_marginLeft     = 0x010100f7;
        constexpr uint32_t layout_marginTop      = 0x010100f8;
        constexpr uint32_t layout_marginRight    = 0x010100f9;
        constexpr uint32_t layout_marginBottom   = 0x010100fa;
        constexpr uint32_t layout_marginStart    = 0x010103b5;
        constexpr uint32_t layout_marginEnd      = 0x010103b6;
        constexpr uint32_t layout_gravity        = 0x010100b3;
        constexpr uint32_t layout_weight         = 0x01010181;
        // TextView
        constexpr uint32_t text                  = 0x0101014f;
        constexpr uint32_t hint                  = 0x01010150;
        constexpr uint32_t textSize              = 0x01010095;
        constexpr uint32_t textColor             = 0x01010098;
        constexpr uint32_t textColorHint         = 0x0101009a;
        constexpr uint32_t textStyle             = 0x01010097;
        constexpr uint32_t typeface              = 0x01010096;
        constexpr uint32_t maxLines              = 0x01010153;
        constexpr uint32_t minLines              = 0x01010156;
        constexpr uint32_t singleLine            = 0x0101015d;
        constexpr uint32_t inputType             = 0x01010220;
        constexpr uint32_t ellipsize             = 0x010100ab;
        constexpr uint32_t fontFamily            = 0x010103ac;
        constexpr uint32_t drawableLeft          = 0x0101016f;
        constexpr uint32_t drawableRight         = 0x01010170;
        constexpr uint32_t drawableTop           = 0x0101016d;
        constexpr uint32_t drawableBottom        = 0x0101016e;
        constexpr uint32_t drawablePadding       = 0x01010171;
        constexpr uint32_t shadowColor           = 0x01010161;
        constexpr uint32_t shadowDx              = 0x01010162;
        constexpr uint32_t shadowDy              = 0x01010163;
        constexpr uint32_t shadowRadius          = 0x01010164;
        constexpr uint32_t letterSpacing         = 0x010104b6;
        constexpr uint32_t includeFontPadding    = 0x0101015f;
    }
    namespace ViewGroup {
        constexpr uint32_t clipChildren           = 0x010100ea;
        constexpr uint32_t clipToPadding          = 0x010100eb;
        constexpr uint32_t layoutAnimation        = 0x010100ec;
        constexpr uint32_t descendantFocusability = 0x010100f1;
        constexpr uint32_t animateLayoutChanges   = 0x010102f2;
        constexpr uint32_t layoutMode             = 0x010103da;
        constexpr uint32_t addStatesFromChildren  = 0x010100f0;
        constexpr uint32_t splitMotionEvents      = 0x010102ef;
        constexpr uint32_t alwaysDrawnWithCache   = 0x010100ef;
        constexpr uint32_t transitionGroup        = 0x01010401;
        constexpr uint32_t touchscreenBlocksFocus = 0x0101048f;
    }
    namespace LinearLayout {
        constexpr uint32_t baselineAligned          = 0x01010126;
        constexpr uint32_t baselineAlignedChildIndex= 0x01010127;
        constexpr uint32_t measureWithLargestChild  = 0x010102d4;
        constexpr uint32_t weightSum                = 0x01010128;
        constexpr uint32_t showDividers             = 0x01010329;
        constexpr uint32_t divider                  = 0x01010129;
        constexpr uint32_t dividerPadding           = 0x0101032a;
        constexpr uint32_t measureAllChildren       = 0x0101010a;
    }
    namespace ImageView {
        constexpr uint32_t src                = 0x01010119;
        constexpr uint32_t scaleType          = 0x0101011d;
        constexpr uint32_t adjustViewBounds   = 0x0101011e;
        constexpr uint32_t maxWidth           = 0x0101011f;
        constexpr uint32_t maxHeight          = 0x01010120;
        constexpr uint32_t tint               = 0x01010121;
        constexpr uint32_t cropToPadding      = 0x01010123;
        constexpr uint32_t drawablePadding    = 0x01010171;
        constexpr uint32_t baseline           = 0x01010122;
    }
    // Full <declare-styleable name="TextView"> from AOSP attrs.xml, in canonical
    // index order. Indices MUST stay aligned with styleable::TextView below so
    // the switch-loop in TextView::TextView references the same positions AOSP's
    // com.android.internal.R.styleable.TextView_* uses.
    namespace TextView {
        constexpr uint32_t bufferType        = 0x0101014e;
        constexpr uint32_t text              = 0x0101014f;
        constexpr uint32_t hint              = 0x01010150;
        constexpr uint32_t textColor         = 0x01010098;
        constexpr uint32_t textColorHighlight= 0x01010099;
        constexpr uint32_t searchResultHighlightColor = 0x01010682;
        constexpr uint32_t focusedSearchResultHighlightColor = 0x01010683;
        constexpr uint32_t textColorHint     = 0x0101009a;
        constexpr uint32_t textAppearance    = 0x01010034;
        constexpr uint32_t textSize          = 0x01010095;
        constexpr uint32_t textScaleX        = 0x01010151;
        constexpr uint32_t typeface          = 0x01010096;
        constexpr uint32_t textStyle         = 0x01010097;
        constexpr uint32_t textFontWeight    = 0x01010585;
        constexpr uint32_t fontFamily        = 0x010103ac;
        constexpr uint32_t textLocale        = 0x01010592;
        constexpr uint32_t textColorLink     = 0x0101009b;
        constexpr uint32_t cursorVisible     = 0x01010152;
        constexpr uint32_t maxLines          = 0x01010153;
        constexpr uint32_t maxHeight         = 0x01010120;
        constexpr uint32_t lines             = 0x01010154;
        constexpr uint32_t height            = 0x01010155;
        constexpr uint32_t minLines          = 0x01010156;
        constexpr uint32_t minHeight         = 0x01010140;
        constexpr uint32_t maxEms            = 0x01010157;
        constexpr uint32_t maxWidth          = 0x0101011f;
        constexpr uint32_t ems               = 0x01010158;
        constexpr uint32_t width             = 0x01010159;
        constexpr uint32_t minEms            = 0x0101015a;
        constexpr uint32_t minWidth          = 0x0101013f;
        constexpr uint32_t gravity           = 0x010100af;
        constexpr uint32_t scrollHorizontally= 0x0101015b;
        constexpr uint32_t password          = 0x0101015c;
        constexpr uint32_t singleLine        = 0x0101015d;
        constexpr uint32_t enabled           = 0x0101000e;
        constexpr uint32_t selectAllOnFocus  = 0x0101015e;
        constexpr uint32_t includeFontPadding= 0x0101015f;
        constexpr uint32_t maxLength         = 0x01010160;
        constexpr uint32_t shadowColor       = 0x01010161;
        constexpr uint32_t shadowDx          = 0x01010162;
        constexpr uint32_t shadowDy          = 0x01010163;
        constexpr uint32_t shadowRadius      = 0x01010164;
        constexpr uint32_t autoLink          = 0x010100b0;
        constexpr uint32_t linksClickable    = 0x010100b1;
        constexpr uint32_t numeric           = 0x01010165;
        constexpr uint32_t digits            = 0x01010166;
        constexpr uint32_t phoneNumber       = 0x01010167;
        constexpr uint32_t inputMethod       = 0x01010168;
        constexpr uint32_t capitalize        = 0x01010169;
        constexpr uint32_t autoText          = 0x0101016a;
        constexpr uint32_t editable          = 0x0101016b;
        constexpr uint32_t freezesText       = 0x0101016c;
        constexpr uint32_t ellipsize         = 0x010100ab;
        constexpr uint32_t drawableTop       = 0x0101016d;
        constexpr uint32_t drawableBottom    = 0x0101016e;
        constexpr uint32_t drawableLeft      = 0x0101016f;
        constexpr uint32_t drawableRight     = 0x01010170;
        // Relative drawables: resolved against layoutDirection (start<->left/right
        // in Drawables::resolveWithLayoutDirection). MUST stay separate from
        // drawableLeft/Right — do not collapse the two reads.
        constexpr uint32_t drawableStart     = 0x01010392;
        constexpr uint32_t drawableEnd       = 0x01010393;
        constexpr uint32_t drawablePadding   = 0x01010171;
        constexpr uint32_t drawableTint      = 0x010104d6;
        constexpr uint32_t drawableTintMode  = 0x010104d7;
        constexpr uint32_t lineSpacingExtra  = 0x01010217;
        constexpr uint32_t lineSpacingMultiplier = 0x01010218;
        constexpr uint32_t lineHeight        = 0x0101057f;
        constexpr uint32_t firstBaselineToTopHeight = 0x0101057d;
        constexpr uint32_t lastBaselineToBottomHeight = 0x0101057e;
        constexpr uint32_t marqueeRepeatLimit= 0x0101021d;
        constexpr uint32_t inputType         = 0x01010220;
        constexpr uint32_t allowUndo         = 0x010104df;
        constexpr uint32_t imeOptions        = 0x01010264;
        constexpr uint32_t privateImeOptions = 0x01010223;
        constexpr uint32_t imeActionLabel    = 0x01010265;
        constexpr uint32_t imeActionId       = 0x01010266;
        constexpr uint32_t editorExtras      = 0x01010224;
        constexpr uint32_t textSelectHandleLeft = 0x010102c5;
        constexpr uint32_t textSelectHandleRight= 0x010102c6;
        constexpr uint32_t textSelectHandle  = 0x010102c7;
        // textEdit* paste/suggestion-window layouts (IME): present for index parity
        // with AOSP; CDROID's IME/InputConnection is out of scope (DEFERRED).
        constexpr uint32_t textEditPasteWindowLayout = 0x01010314;
        constexpr uint32_t textEditNoPasteWindowLayout = 0x01010315;
        constexpr uint32_t textEditSidePasteWindowLayout = 0x0101035e;
        constexpr uint32_t textEditSideNoPasteWindowLayout = 0x0101035f;
        constexpr uint32_t textEditSuggestionItemLayout = 0x01010374;
        constexpr uint32_t textEditSuggestionContainerLayout = 0x01120109; // attr-private
        constexpr uint32_t textEditSuggestionHighlightStyle = 0x0112010a;  // attr-private
        constexpr uint32_t textCursorDrawable = 0x01010362;
        constexpr uint32_t textIsSelectable   = 0x01010316;
        constexpr uint32_t textAllCaps        = 0x0101038c;
        constexpr uint32_t elegantTextHeight  = 0x0101045d;
        constexpr uint32_t fallbackLineSpacing= 0x0101057b;
        constexpr uint32_t letterSpacing      = 0x010104b6;
        constexpr uint32_t fontFeatureSettings= 0x010104b7;
        constexpr uint32_t fontVariationSettings = 0x01010570;
        constexpr uint32_t breakStrategy      = 0x010104dd;
        constexpr uint32_t hyphenationFrequency = 0x010104de;
        constexpr uint32_t lineBreakStyle     = 0x0101066e;
        constexpr uint32_t lineBreakWordStyle = 0x0101066f;
        constexpr uint32_t autoSizeTextType   = 0x01010535;
        constexpr uint32_t autoSizeStepGranularity = 0x01010536;
        constexpr uint32_t autoSizePresetSizes= 0x01010537;
        constexpr uint32_t autoSizeMinTextSize= 0x01010538;
        constexpr uint32_t autoSizeMaxTextSize= 0x01010546;
        constexpr uint32_t justificationMode  = 0x01010567;
        constexpr uint32_t useBoundsForWidth  = 0x01010698;
        constexpr uint32_t shiftDrawingOffsetForStartOverhang = 0x010106a2;
        constexpr uint32_t useLocalePreferredLineHeightForMinimum = 0x0101069d;
    }
    // <declare-styleable name="ProgressBar"> from AOSP attrs.xml, canonical order.
    namespace ProgressBar {
        constexpr uint32_t min                     = 0x01010539;
        constexpr uint32_t max                     = 0x01010136;
        constexpr uint32_t progress                = 0x01010137;
        constexpr uint32_t secondaryProgress       = 0x01010138;
        constexpr uint32_t indeterminate           = 0x01010139;
        constexpr uint32_t indeterminateOnly       = 0x0101013a;
        constexpr uint32_t indeterminateDrawable   = 0x0101013b;
        constexpr uint32_t progressDrawable        = 0x0101013c;
        constexpr uint32_t indeterminateDuration   = 0x0101013d;
        constexpr uint32_t indeterminateBehavior   = 0x0101013e;
        constexpr uint32_t minWidth                = 0x0101013f;
        constexpr uint32_t maxWidth                = 0x0101011f;
        constexpr uint32_t minHeight               = 0x01010140;
        constexpr uint32_t maxHeight               = 0x01010120;
        constexpr uint32_t interpolator            = 0x01010141;
        constexpr uint32_t animationResolution     = 0x0101031a;
        constexpr uint32_t mirrorForRtl            = 0x010103ce;
        constexpr uint32_t progressTint            = 0x01010463;
        constexpr uint32_t progressTintMode        = 0x01010464;
        constexpr uint32_t progressBackgroundTint  = 0x01010465;
        constexpr uint32_t progressBackgroundTintMode = 0x01010466;
        constexpr uint32_t secondaryProgressTint   = 0x01010467;
        constexpr uint32_t secondaryProgressTintMode = 0x01010468;
        constexpr uint32_t indeterminateTint       = 0x01010469;
        constexpr uint32_t indeterminateTintMode   = 0x0101046a;
        constexpr uint32_t backgroundTint          = 0x0101046b;
        constexpr uint32_t backgroundTintMode      = 0x0101046c;
    }
    namespace CompoundButton {
        constexpr uint32_t checked            = 0x01010106;
        constexpr uint32_t button             = 0x01010107;
    }
    // <declare-styleable name="Switch"> from AOSP attrs.xml, canonical order.
    namespace Switch {
        constexpr uint32_t thumb                = 0x01010142;
        constexpr uint32_t thumbTint           = 0x01010471;
        constexpr uint32_t thumbTintMode       = 0x01010472;
        constexpr uint32_t track               = 0x0101036f;
        constexpr uint32_t trackTint           = 0x010104d9;
        constexpr uint32_t trackTintMode       = 0x010104da;
        constexpr uint32_t textOn              = 0x01010124;
        constexpr uint32_t textOff             = 0x01010125;
        constexpr uint32_t thumbTextPadding    = 0x01010372;
        constexpr uint32_t switchTextAppearance= 0x0101036e;
        constexpr uint32_t switchMinWidth      = 0x01010370;
        constexpr uint32_t switchPadding       = 0x01010371;
        constexpr uint32_t splitTrack          = 0x0101044c;
        constexpr uint32_t showText            = 0x010104ad;
    }
    // Attrs read by each widget's ctor (IDs from aapt2 dump resources). Several IDs
    // are shared across widgets (e.g. textOn/textOff, disabledAlpha, splitTrack) —
    // each widget keeps its own namespace for readability.
    namespace SeekBar {
        constexpr uint32_t thumb            = 0x01010142;
        constexpr uint32_t thumbOffset      = 0x01010143;
        constexpr uint32_t splitTrack       = 0x0101044c;
        constexpr uint32_t disabledAlpha    = 0x01010033;
        constexpr uint32_t thumbTint        = 0x01010471;
        constexpr uint32_t tickMark         = 0x0101050a;
        constexpr uint32_t tickMarkTint     = 0x0101050b;
    }
    namespace ToggleButton {
        constexpr uint32_t textOn           = 0x01010124;
        constexpr uint32_t textOff          = 0x01010125;
        constexpr uint32_t disabledAlpha    = 0x01010033;
    }
    namespace CheckedTextView {
        constexpr uint32_t checked          = 0x01010106;
        constexpr uint32_t checkMark        = 0x01010108;
        constexpr uint32_t checkMarkTint    = 0x010104a7;
        constexpr uint32_t checkMarkTintMode= 0x010104a8;
    }
    namespace ListView {
        constexpr uint32_t divider              = 0x01010129;
        constexpr uint32_t dividerHeight        = 0x0101012a;
        constexpr uint32_t headerDividersEnabled= 0x0101022e;
        constexpr uint32_t footerDividersEnabled= 0x0101022f;
        constexpr uint32_t overScrollHeader     = 0x010102c2;
        constexpr uint32_t overScrollFooter     = 0x010102c3;
    }
    namespace RadioGroup {
        constexpr uint32_t checkedButton        = 0x01010148;
    }
    namespace NestedScrollView {
        constexpr uint32_t fillViewport         = 0x0101017a;
    }
    namespace AdapterViewAnimator {
        constexpr uint32_t inAnimation          = 0x01010177;
        constexpr uint32_t outAnimation         = 0x01010178;
        constexpr uint32_t animateFirstView     = 0x010102d5;
        constexpr uint32_t loopViews            = 0x01010307;
    }
    namespace AdapterViewFlipper {
        constexpr uint32_t flipInterval         = 0x01010179;
        constexpr uint32_t autoStart            = 0x010102b5;
    }
    namespace TextClock {
        constexpr uint32_t format12Hour         = 0x010103ca;
        constexpr uint32_t format24Hour         = 0x010103cb;
        constexpr uint32_t timeZone             = 0x010103cc;
    }
    namespace AnalogClock {
        constexpr uint32_t dial                 = 0x01010102;
        constexpr uint32_t hand_hour            = 0x01010103;
        constexpr uint32_t hand_minute          = 0x01010104;
        constexpr uint32_t hand_second          = 0x01010623;
    }
    namespace GridView {
        constexpr uint32_t horizontalSpacing    = 0x01010114;
        constexpr uint32_t verticalSpacing      = 0x01010115;
        constexpr uint32_t stretchMode          = 0x01010116;
        constexpr uint32_t columnWidth          = 0x01010117;
        constexpr uint32_t numColumns           = 0x01010118;
    }
    namespace GridLayout {
        constexpr uint32_t rowCount             = 0x01010375;
        constexpr uint32_t columnCount          = 0x01010377;
        constexpr uint32_t useDefaultMargins    = 0x01010379;
        constexpr uint32_t alignmentMode        = 0x0101037a;
        constexpr uint32_t rowOrderPreserved    = 0x01010376;
        constexpr uint32_t columnOrderPreserved = 0x01010378;
    }
    namespace Spinner {
        constexpr uint32_t spinnerMode          = 0x010102f1;
        constexpr uint32_t prompt               = 0x0101017b;
        constexpr uint32_t popupBackground      = 0x01010176;
        constexpr uint32_t dropDownSelector     = 0x01010175;
        constexpr uint32_t dropDownWidth        = 0x01010262;
        constexpr uint32_t dropDownAnchor       = 0x01010263;
    }
    namespace RatingBar {
        constexpr uint32_t numStars             = 0x01010144;
        constexpr uint32_t rating               = 0x01010145;
        constexpr uint32_t stepSize             = 0x01010146;
        constexpr uint32_t isIndicator          = 0x01010147;
    }
    namespace Chronometer {
        constexpr uint32_t format               = 0x01010105;
        constexpr uint32_t countDown            = 0x0101051b;
    }
    namespace ScrollView {
        constexpr uint32_t fillViewport         = 0x0101017a;
    }
    namespace RelativeLayout {
        constexpr uint32_t ignoreGravity       = 0x010101ff;
    }
    // ViewGroup.LayoutParams / MarginLayoutParams styleables (read for every child
    // during inflation). layout_width/height are MATCH_PARENT(-1)/WRAP_CONTENT(-2).
    namespace Layout {
        constexpr uint32_t layout_width        = 0x010100f4;
        constexpr uint32_t layout_height       = 0x010100f5;
    }
    namespace MarginLayout {
        constexpr uint32_t layout_margin           = 0x010100f6;
        constexpr uint32_t layout_marginLeft       = 0x010100f7;
        constexpr uint32_t layout_marginTop        = 0x010100f8;
        constexpr uint32_t layout_marginRight      = 0x010100f9;
        constexpr uint32_t layout_marginBottom     = 0x010100fa;
        constexpr uint32_t layout_marginStart      = 0x010103b5;
        constexpr uint32_t layout_marginEnd        = 0x010103b6;
        constexpr uint32_t layout_marginHorizontal = 0x0101053b;
        constexpr uint32_t layout_marginVertical   = 0x0101053c;
    }
}

// R.styleable arrays: index → framework attr ID. Used by obtainStyledAttributesTyped.
namespace styleable {
    namespace View {
        enum {
            background, padding, paddingLeft, paddingTop, paddingRight, paddingBottom,
            paddingStart, paddingEnd, minWidth, minHeight, visibility, id, tag,
            scrollX, scrollY, fitsSystemWindows, scrollbars, fadingEdge, fadingEdgeLength,
            nextFocusLeft, nextFocusRight, nextFocusUp, nextFocusDown,
            clickable, longClickable, saveEnabled, drawingCacheQuality, duplicateParentState,
            focusable, focusableInTouchMode, soundEffectsEnabled, hapticFeedbackEnabled,
            onClick, contentDescription, isScrollContainer, foregroundGravity,
            scrollbarStyle, scrollbarSize, scrollbarFadeDuration, scrollbarDefaultDelayBeforeFade,
            fadeScrollbars, filterTouchesWhenObscured, keepScreenOn, layerType,
            layoutDirection, textDirection, textAlignment, importantForAccessibility,
            requiresFadingEdge, overScrollMode, verticalScrollbarPosition,
            rotation, rotationX, rotationY, scaleX, scaleY,
            transformPivotX, transformPivotY, translationX, translationY, translationZ,
            alpha, elevation, transitionName, stateListAnimator,
            nestedScrollingEnabled, backgroundTint, backgroundTintMode,
            foregroundTint, foregroundTintMode, outlineProvider,
            scrollIndicators, nextFocusForward, nextClusterForward,
            keyboardNavigationCluster, focusedByDefault, allowClickWhenDisabled,
            enabled,
            COUNT
        };
        extern const uint32_t IDS[];
    }
    namespace ViewGroup {
        enum {
            clipChildren, clipToPadding, layoutAnimation, descendantFocusability,
            animateLayoutChanges, layoutMode, addStatesFromChildren,
            splitMotionEvents, alwaysDrawnWithCache, transitionGroup,
            touchscreenBlocksFocus,
            COUNT
        };
        extern const uint32_t IDS[];
    }
    namespace LinearLayout {
        enum {
            orientation, gravity, baselineAligned, baselineAlignedChildIndex,
            weightSum, measureWithLargestChild, showDividers, divider,
            dividerPadding, measureAllChildren,
            COUNT
        };
        extern const uint32_t IDS[];
    }
    namespace ImageView {
        enum { src, scaleType, adjustViewBounds, maxWidth, maxHeight, tint,
               cropToPadding, drawablePadding, baseline, baselineAlignBottom, alpha,
               COUNT };
        extern const uint32_t IDS[];
    }
    namespace CompoundButton {
        enum { checked, button, COUNT };
        extern const uint32_t IDS[];
    }
    namespace FrameLayout {
        enum { measureAllChildren, foreground, foregroundInsidePadding, COUNT };
        extern const uint32_t IDS[];
    }
    // Mirrors AOSP <declare-styleable name="TextView"> index-for-index (106 attrs).
    // Enum names are positional — keep order identical to fw_attr::TextView above.
    namespace TextView {
        enum {
            bufferType, text, hint, textColor, textColorHighlight,
            searchResultHighlightColor, focusedSearchResultHighlightColor,
            textColorHint, textAppearance, textSize, textScaleX, typeface, textStyle,
            textFontWeight, fontFamily, textLocale, textColorLink, cursorVisible,
            maxLines, maxHeight, lines, height, minLines, minHeight,
            maxEms, maxWidth, ems, width, minEms, minWidth,
            gravity, scrollHorizontally, password, singleLine, enabled,
            selectAllOnFocus, includeFontPadding, maxLength,
            shadowColor, shadowDx, shadowDy, shadowRadius,
            autoLink, linksClickable, numeric, digits, phoneNumber, inputMethod,
            capitalize, autoText, editable, freezesText, ellipsize,
            drawableTop, drawableBottom, drawableLeft, drawableRight,
            drawableStart, drawableEnd, drawablePadding, drawableTint, drawableTintMode,
            lineSpacingExtra, lineSpacingMultiplier, lineHeight,
            firstBaselineToTopHeight, lastBaselineToBottomHeight, marqueeRepeatLimit,
            inputType, allowUndo, imeOptions, privateImeOptions,
            imeActionLabel, imeActionId, editorExtras,
            textSelectHandleLeft, textSelectHandleRight, textSelectHandle,
            textEditPasteWindowLayout, textEditNoPasteWindowLayout,
            textEditSidePasteWindowLayout, textEditSideNoPasteWindowLayout,
            textEditSuggestionItemLayout, textEditSuggestionContainerLayout,
            textEditSuggestionHighlightStyle,
            textCursorDrawable, textIsSelectable, textAllCaps,
            elegantTextHeight, fallbackLineSpacing, letterSpacing,
            fontFeatureSettings, fontVariationSettings,
            breakStrategy, hyphenationFrequency, lineBreakStyle, lineBreakWordStyle,
            autoSizeTextType, autoSizeStepGranularity, autoSizePresetSizes,
            autoSizeMinTextSize, autoSizeMaxTextSize,
            justificationMode, useBoundsForWidth,
            shiftDrawingOffsetForStartOverhang,
            useLocalePreferredLineHeightForMinimum,
            COUNT
        };
        extern const uint32_t IDS[];
    }
    // Mirrors AOSP <declare-styleable name="ProgressBar"> (27 attrs).
    namespace ProgressBar {
        enum {
            min, max, progress, secondaryProgress,
            indeterminate, indeterminateOnly,
            indeterminateDrawable, progressDrawable,
            indeterminateDuration, indeterminateBehavior,
            minWidth, maxWidth, minHeight, maxHeight,
            interpolator, animationResolution, mirrorForRtl,
            progressTint, progressTintMode,
            progressBackgroundTint, progressBackgroundTintMode,
            secondaryProgressTint, secondaryProgressTintMode,
            indeterminateTint, indeterminateTintMode,
            backgroundTint, backgroundTintMode,
            COUNT
        };
        extern const uint32_t IDS[];
    }
    // Mirrors AOSP <declare-styleable name="Switch"> (14 attrs).
    namespace Switch {
        enum {
            thumb, thumbTint, thumbTintMode,
            track, trackTint, trackTintMode,
            textOn, textOff,
            thumbTextPadding, switchTextAppearance,
            switchMinWidth, switchPadding,
            splitTrack, showText,
            COUNT
        };
        extern const uint32_t IDS[];
    }
    namespace SeekBar {
        enum { thumb, thumbOffset, splitTrack, disabledAlpha,
               thumbTint, tickMark, tickMarkTint, COUNT };
        extern const uint32_t IDS[];
    }
    namespace ToggleButton {
        enum { textOn, textOff, disabledAlpha, COUNT };
        extern const uint32_t IDS[];
    }
    namespace CheckedTextView {
        enum { checked, checkMark, checkMarkTint, checkMarkTintMode, COUNT };
        extern const uint32_t IDS[];
    }
    namespace ListView {
        enum { divider, dividerHeight, headerDividersEnabled,
               footerDividersEnabled, overScrollHeader, overScrollFooter, COUNT };
        extern const uint32_t IDS[];
    }
    namespace RadioGroup {
        enum { checkedButton, orientation, COUNT };
        extern const uint32_t IDS[];
    }
    namespace NestedScrollView {
        enum { fillViewport, COUNT };
        extern const uint32_t IDS[];
    }
    namespace AdapterViewAnimator {
        enum { inAnimation, outAnimation, animateFirstView, loopViews, COUNT };
        extern const uint32_t IDS[];
    }
    namespace AdapterViewFlipper {
        enum { flipInterval, autoStart, COUNT };
        extern const uint32_t IDS[];
    }
    namespace TextClock {
        enum { format12Hour, format24Hour, timeZone, COUNT };
        extern const uint32_t IDS[];
    }
    namespace AnalogClock {
        enum { dial, hand_hour, hand_minute, hand_second, COUNT };
        extern const uint32_t IDS[];
    }
    namespace GridView {
        enum { horizontalSpacing, verticalSpacing, stretchMode, columnWidth,
               numColumns, gravity, COUNT };
        extern const uint32_t IDS[];
    }
    namespace GridLayout {
        enum { orientation, rowCount, columnCount, useDefaultMargins,
               alignmentMode, rowOrderPreserved, columnOrderPreserved, COUNT };
        extern const uint32_t IDS[];
    }
    namespace Spinner {
        enum { spinnerMode, prompt, popupBackground, dropDownSelector,
               dropDownWidth, dropDownAnchor, gravity, COUNT };
        extern const uint32_t IDS[];
    }
    namespace RatingBar {
        enum { numStars, rating, stepSize, isIndicator, COUNT };
        extern const uint32_t IDS[];
    }
    namespace Chronometer {
        enum { format, countDown, COUNT };
        extern const uint32_t IDS[];
    }
    namespace ScrollView {
        enum { fillViewport, COUNT };
        extern const uint32_t IDS[];
    }
    namespace RelativeLayout {
        enum { ignoreGravity, gravity, COUNT };
        extern const uint32_t IDS[];
    }
    namespace Layout {
        enum { layout_width, layout_height, COUNT };
        extern const uint32_t IDS[];
    }
    namespace MarginLayout {
        enum { layout_margin, layout_marginLeft, layout_marginTop,
               layout_marginRight, layout_marginBottom, layout_marginStart,
               layout_marginEnd, layout_marginHorizontal, layout_marginVertical, COUNT };
        extern const uint32_t IDS[];
    }
}
} // namespace cdroid
#endif // __FRAMEWORK_STYLEABLE_H__
