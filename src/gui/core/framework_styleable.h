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
    namespace TextView {
        constexpr uint32_t textAppearance     = 0x01010034;
        constexpr uint32_t textColorHighlight = 0x01010099;
        constexpr uint32_t textColorLink      = 0x0101009b;
        constexpr uint32_t textAllCaps        = 0x0101038c;
        constexpr uint32_t textScaleX         = 0x01010151;
        constexpr uint32_t lineSpacingExtra   = 0x01010217;
        constexpr uint32_t lineSpacingMultiplier = 0x01010218;
        constexpr uint32_t capitalize         = 0x01010169;
        constexpr uint32_t bufferType         = 0x0101014e;
        constexpr uint32_t selectAllOnFocus   = 0x0101015e;
        constexpr uint32_t textIsSelectable   = 0x01010316;
        constexpr uint32_t editable           = 0x0101016b;
        constexpr uint32_t password           = 0x0101015c;
        constexpr uint32_t imeOptions         = 0x01010264;
        constexpr uint32_t editorExtras       = 0x01010224;
        constexpr uint32_t autoLink           = 0x010100b0;
        constexpr uint32_t linksClickable     = 0x010100b1;
    }
    namespace ProgressBar {
        constexpr uint32_t max                = 0x01010136;
        constexpr uint32_t progress           = 0x01010137;
        constexpr uint32_t secondaryProgress  = 0x01010138;
        constexpr uint32_t indeterminate      = 0x01010139;
        constexpr uint32_t indeterminateOnly  = 0x0101013a;
        constexpr uint32_t indeterminateDrawable = 0x0101013b;
        constexpr uint32_t progressDrawable   = 0x0101013c;
        constexpr uint32_t indeterminateDuration = 0x0101013d;
        constexpr uint32_t indeterminateBehavior = 0x0101013e;
        constexpr uint32_t progressTint       = 0x01010463;
        constexpr uint32_t progressTintMode   = 0x01010464;
        constexpr uint32_t indeterminateTint  = 0x01010469;
        constexpr uint32_t indeterminateTintMode = 0x0101046a;
        constexpr uint32_t progressBarStyle   = 0x01010077;
        constexpr uint32_t progressBarPadding = 0x01010319;
    }
    namespace CompoundButton {
        constexpr uint32_t checked            = 0x01010106;
        constexpr uint32_t button             = 0x01010107;
    }
    namespace Switch {
        constexpr uint32_t thumb              = 0x01010142;
        constexpr uint32_t switchTextAppearance = 0x0101036e;
        constexpr uint32_t switchTextOn       = 0x0101036b;
        constexpr uint32_t switchTextOff      = 0x0101036c;
        constexpr uint32_t showText           = 0x010104ad;
        constexpr uint32_t splitTrack         = 0x0101044c;
        constexpr uint32_t thumbTint          = 0x01010471;
        constexpr uint32_t thumbTintMode      = 0x01010472;
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
}
} // namespace cdroid
#endif // __FRAMEWORK_STYLEABLE_H__
