#ifndef __FRAMEWORK_STYLEABLE_H__
#define __FRAMEWORK_STYLEABLE_H__
#include <cstdint>
namespace cdroid {
// Framework android:attr resource IDs (from AOSP public.xml, stable across API levels).
// These define the styleable arrays for TypedArray-based attribute reading,
// replacing the string-keyed AttributeSet approach for binary AXML.
namespace fw_attr {
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
    constexpr uint32_t clickable             = 0x010100e5;
    constexpr uint32_t longClickable         = 0x010100e6;
    constexpr uint32_t focusable             = 0x010100da;
    constexpr uint32_t enabled               = 0x0101000e;
    constexpr uint32_t onClick               = 0x0101026f;
    constexpr uint32_t contentDescription    = 0x01010273;
    constexpr uint32_t scrollX               = 0x010100d2;
    constexpr uint32_t scrollY               = 0x010100d3;
    constexpr uint32_t alpha                 = 0x0101031f;
    constexpr uint32_t elevation             = 0x01010440;
    constexpr uint32_t translationX          = 0x01010322;
    constexpr uint32_t translationY          = 0x01010323;
    constexpr uint32_t scaleX                = 0x01010324;
    constexpr uint32_t scaleY                = 0x01010325;
    constexpr uint32_t rotation              = 0x01010326;
    constexpr uint32_t fitsSystemWindows     = 0x010100dd;
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
    constexpr uint32_t gravity               = 0x010100af;
    constexpr uint32_t orientation           = 0x010100c4;
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
    constexpr uint32_t textAlignment         = 0x010103b1;
    constexpr uint32_t includeFontPadding    = 0x0101015f;
}

// R.styleable.View — the attr IDs View reads, in index order.
namespace styleable {
    namespace View {
        enum {
            background, padding, paddingLeft, paddingTop, paddingRight, paddingBottom,
            paddingStart, paddingEnd, minWidth, minHeight, visibility, id, tag,
            clickable, longClickable, focusable, enabled, onClick, contentDescription,
            scrollX, scrollY, alpha, elevation, translationX, translationY,
            scaleX, scaleY, rotation, fitsSystemWindows,
            COUNT
        };
        extern const uint32_t IDS[];
    }
}
} // namespace cdroid
#endif // __FRAMEWORK_STYLEABLE_H__
