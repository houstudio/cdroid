#ifndef __DIRECTFB_STRINGS_H__
#define __DIRECTFB_STRINGS_H__
#include <directfb.h>


struct DFBPixelFormatName {
     DFBSurfacePixelFormat format;
     const char *name;
};

#define DirectFBPixelFormatNames(Identifier) struct DFBPixelFormatName Identifier[] = { \
     { DSPF_ARGB1555, "ARGB1555" }, \
     { DSPF_RGB16, "RGB16" }, \
     { DSPF_RGB24, "RGB24" }, \
     { DSPF_RGB32, "RGB32" }, \
     { DSPF_ARGB, "ARGB" }, \
     { DSPF_A8, "A8" }, \
     { DSPF_YUY2, "YUY2" }, \
     { DSPF_RGB332, "RGB332" }, \
     { DSPF_UYVY, "UYVY" }, \
     { DSPF_I420, "I420" }, \
     { DSPF_YV12, "YV12" }, \
     { DSPF_LUT8, "LUT8" }, \
     { DSPF_ALUT44, "ALUT44" }, \
     { DSPF_AiRGB, "AiRGB" }, \
     { DSPF_A1, "A1" }, \
     { DSPF_NV12, "NV12" }, \
     { DSPF_NV16, "NV16" }, \
     { DSPF_ARGB2554, "ARGB2554" }, \
     { DSPF_ARGB4444, "ARGB4444" }, \
     { DSPF_RGBA4444, "RGBA4444" }, \
     { DSPF_NV21, "NV21" }, \
     { DSPF_AYUV, "AYUV" }, \
     { DSPF_A4, "A4" }, \
     { DSPF_ARGB1666, "ARGB1666" }, \
     { DSPF_ARGB6666, "ARGB6666" }, \
     { DSPF_RGB18, "RGB18" }, \
     { DSPF_LUT2, "LUT2" }, \
     { DSPF_RGB444, "RGB444" }, \
     { DSPF_RGB555, "RGB555" }, \
     { DSPF_BGR555, "BGR555" }, \
     { DSPF_RGBA5551, "RGBA5551" }, \
     { DSPF_YUV444P, "YUV444P" }, \
     { DSPF_ARGB8565, "ARGB8565" }, \
     { DSPF_AVYU, "AVYU" }, \
     { DSPF_VYU, "VYU" }, \
     { DSPF_A1_LSB, "A1_LSB" }, \
     { DSPF_YV16, "YV16" }, \
     { DSPF_ABGR, "ABGR" }, \
     { DSPF_RGBAF88871, "RGBAF88871" }, \
     { DSPF_LUT4, "LUT4" }, \
     { DSPF_ALUT8, "ALUT8" }, \
     { DSPF_LUT1, "LUT1" }, \
     { (DFBSurfacePixelFormat) DSPF_UNKNOWN, "UNKNOWN" } \
};


struct DFBPorterDuffRuleName {
     DFBSurfacePorterDuffRule rule;
     const char *name;
};

#define DirectFBPorterDuffRuleNames(Identifier) struct DFBPorterDuffRuleName Identifier[] = { \
     { DSPD_CLEAR, "CLEAR" }, \
     { DSPD_SRC, "SRC" }, \
     { DSPD_SRC_OVER, "SRC_OVER" }, \
     { DSPD_DST_OVER, "DST_OVER" }, \
     { DSPD_SRC_IN, "SRC_IN" }, \
     { DSPD_DST_IN, "DST_IN" }, \
     { DSPD_SRC_OUT, "SRC_OUT" }, \
     { DSPD_DST_OUT, "DST_OUT" }, \
     { DSPD_SRC_ATOP, "SRC_ATOP" }, \
     { DSPD_DST_ATOP, "DST_ATOP" }, \
     { DSPD_ADD, "ADD" }, \
     { DSPD_XOR, "XOR" }, \
     { DSPD_DST, "DST" }, \
     { (DFBSurfacePorterDuffRule) DSPD_NONE, "NONE" } \
};


struct DFBSurfaceCapabilitiesName {
     DFBSurfaceCapabilities capability;
     const char *name;
};

#define DirectFBSurfaceCapabilitiesNames(Identifier) struct DFBSurfaceCapabilitiesName Identifier[] = { \
     { DSCAPS_PRIMARY, "PRIMARY" }, \
     { DSCAPS_SYSTEMONLY, "SYSTEMONLY" }, \
     { DSCAPS_VIDEOONLY, "VIDEOONLY" }, \
     { DSCAPS_GL, "GL" }, \
     { DSCAPS_DOUBLE, "DOUBLE" }, \
     { DSCAPS_SUBSURFACE, "SUBSURFACE" }, \
     { DSCAPS_INTERLACED, "INTERLACED" }, \
     { DSCAPS_SEPARATED, "SEPARATED" }, \
     { DSCAPS_SEPARATED, "SEPARATED" }, \
     { DSCAPS_STATIC_ALLOC, "STATIC_ALLOC" }, \
     { DSCAPS_TRIPLE, "TRIPLE" }, \
     { DSCAPS_PREMULTIPLIED, "PREMULTIPLIED" }, \
     { DSCAPS_DEPTH, "DEPTH" }, \
     { DSCAPS_STEREO, "STEREO" }, \
     { DSCAPS_SHARED, "SHARED" }, \
     { DSCAPS_ROTATED, "ROTATED" }, \
     { (DFBSurfaceCapabilities) DSCAPS_NONE, "NONE" } \
};


struct DFBColorSpaceName {
     DFBSurfaceColorSpace colorspace;
     const char *name;
};

#define DirectFBColorSpaceNames(Identifier) struct DFBColorSpaceName Identifier[] = { \
     { DSCS_RGB, "RGB" }, \
     { DSCS_BT601, "BT601" }, \
     { DSCS_BT601_FULLRANGE, "BT601_FULLRANGE" }, \
     { DSCS_BT709, "BT709" }, \
     { (DFBSurfaceColorSpace) DSCS_UNKNOWN, "UNKNOWN" } \
};


struct DFBInputDeviceTypeFlagsName {
     DFBInputDeviceTypeFlags type;
     const char *name;
};

#define DirectFBInputDeviceTypeFlagsNames(Identifier) struct DFBInputDeviceTypeFlagsName Identifier[] = { \
     { DIDTF_KEYBOARD, "KEYBOARD" }, \
     { DIDTF_MOUSE, "MOUSE" }, \
     { DIDTF_JOYSTICK, "JOYSTICK" }, \
     { DIDTF_REMOTE, "REMOTE" }, \
     { DIDTF_VIRTUAL, "VIRTUAL" }, \
     { (DFBInputDeviceTypeFlags) DIDTF_NONE, "NONE" } \
};


struct DFBSurfaceDrawingFlagsName {
     DFBSurfaceDrawingFlags flag;
     const char *name;
};

#define DirectFBSurfaceDrawingFlagsNames(Identifier) struct DFBSurfaceDrawingFlagsName Identifier[] = { \
     { DSDRAW_BLEND, "BLEND" }, \
     { DSDRAW_DST_COLORKEY, "DST_COLORKEY" }, \
     { DSDRAW_SRC_PREMULTIPLY, "SRC_PREMULTIPLY" }, \
     { DSDRAW_DST_PREMULTIPLY, "DST_PREMULTIPLY" }, \
     { DSDRAW_DEMULTIPLY, "DEMULTIPLY" }, \
     { DSDRAW_XOR, "XOR" }, \
     { (DFBSurfaceDrawingFlags) DSDRAW_NOFX, "NOFX" } \
};


struct DFBSurfaceBlittingFlagsName {
     DFBSurfaceBlittingFlags flag;
     const char *name;
};

#define DirectFBSurfaceBlittingFlagsNames(Identifier) struct DFBSurfaceBlittingFlagsName Identifier[] = { \
     { DSBLIT_BLEND_ALPHACHANNEL, "BLEND_ALPHACHANNEL" }, \
     { DSBLIT_BLEND_COLORALPHA, "BLEND_COLORALPHA" }, \
     { DSBLIT_COLORIZE, "COLORIZE" }, \
     { DSBLIT_SRC_COLORKEY, "SRC_COLORKEY" }, \
     { DSBLIT_DST_COLORKEY, "DST_COLORKEY" }, \
     { DSBLIT_SRC_PREMULTIPLY, "SRC_PREMULTIPLY" }, \
     { DSBLIT_DST_PREMULTIPLY, "DST_PREMULTIPLY" }, \
     { DSBLIT_DEMULTIPLY, "DEMULTIPLY" }, \
     { DSBLIT_DEINTERLACE, "DEINTERLACE" }, \
     { DSBLIT_SRC_PREMULTCOLOR, "SRC_PREMULTCOLOR" }, \
     { DSBLIT_XOR, "XOR" }, \
     { DSBLIT_INDEX_TRANSLATION, "INDEX_TRANSLATION" }, \
     { DSBLIT_ROTATE90, "ROTATE90" }, \
     { DSBLIT_ROTATE180, "ROTATE180" }, \
     { DSBLIT_ROTATE270, "ROTATE270" }, \
     { DSBLIT_COLORKEY_PROTECT, "COLORKEY_PROTECT" }, \
     { DSBLIT_SRC_COLORKEY_EXTENDED, "SRC_COLORKEY_EXTENDED" }, \
     { DSBLIT_DST_COLORKEY_EXTENDED, "DST_COLORKEY_EXTENDED" }, \
     { DSBLIT_SRC_MASK_ALPHA, "SRC_MASK_ALPHA" }, \
     { DSBLIT_SRC_MASK_COLOR, "SRC_MASK_COLOR" }, \
     { DSBLIT_FLIP_HORIZONTAL, "FLIP_HORIZONTAL" }, \
     { DSBLIT_FLIP_VERTICAL, "FLIP_VERTICAL" }, \
     { DSBLIT_ROP, "ROP" }, \
     { DSBLIT_SRC_COLORMATRIX, "SRC_COLORMATRIX" }, \
     { DSBLIT_SRC_CONVOLUTION, "SRC_CONVOLUTION" }, \
     { (DFBSurfaceBlittingFlags) DSBLIT_NOFX, "NOFX" } \
};


struct DFBSurfaceFlipFlagsName {
     DFBSurfaceFlipFlags flag;
     const char *name;
};

#define DirectFBSurfaceFlipFlagsNames(Identifier) struct DFBSurfaceFlipFlagsName Identifier[] = { \
     { DSFLIP_WAIT, "WAIT" }, \
     { DSFLIP_BLIT, "BLIT" }, \
     { DSFLIP_ONSYNC, "ONSYNC" }, \
     { DSFLIP_WAIT, "WAIT" }, \
     { DSFLIP_PIPELINE, "PIPELINE" }, \
     { DSFLIP_ONCE, "ONCE" }, \
     { DSFLIP_QUEUE, "QUEUE" }, \
     { DSFLIP_FLUSH, "FLUSH" }, \
     { DSFLIP_SWAP, "SWAP" }, \
     { DSFLIP_UPDATE, "UPDATE" }, \
     { DSFLIP_WAITFORSYNC, "WAITFORSYNC" }, \
     { (DFBSurfaceFlipFlags) DSFLIP_NONE, "NONE" } \
};


struct DFBSurfaceBlendFunctionName {
     DFBSurfaceBlendFunction function;
     const char *name;
};

#define DirectFBSurfaceBlendFunctionNames(Identifier) struct DFBSurfaceBlendFunctionName Identifier[] = { \
     { DSBF_ZERO, "ZERO" }, \
     { DSBF_ONE, "ONE" }, \
     { DSBF_SRCCOLOR, "SRCCOLOR" }, \
     { DSBF_INVSRCCOLOR, "INVSRCCOLOR" }, \
     { DSBF_SRCALPHA, "SRCALPHA" }, \
     { DSBF_INVSRCALPHA, "INVSRCALPHA" }, \
     { DSBF_DESTALPHA, "DESTALPHA" }, \
     { DSBF_INVDESTALPHA, "INVDESTALPHA" }, \
     { DSBF_DESTCOLOR, "DESTCOLOR" }, \
     { DSBF_INVDESTCOLOR, "INVDESTCOLOR" }, \
     { DSBF_SRCALPHASAT, "SRCALPHASAT" }, \
     { (DFBSurfaceBlendFunction) DSBF_UNKNOWN, "UNKNOWN" } \
};


struct DFBInputDeviceCapabilitiesName {
     DFBInputDeviceCapabilities capability;
     const char *name;
};

#define DirectFBInputDeviceCapabilitiesNames(Identifier) struct DFBInputDeviceCapabilitiesName Identifier[] = { \
     { DICAPS_KEYS, "KEYS" }, \
     { DICAPS_AXES, "AXES" }, \
     { DICAPS_BUTTONS, "BUTTONS" }, \
     { (DFBInputDeviceCapabilities) DICAPS_NONE, "NONE" } \
};


struct DFBDisplayLayerTypeFlagsName {
     DFBDisplayLayerTypeFlags type;
     const char *name;
};

#define DirectFBDisplayLayerTypeFlagsNames(Identifier) struct DFBDisplayLayerTypeFlagsName Identifier[] = { \
     { DLTF_GRAPHICS, "GRAPHICS" }, \
     { DLTF_VIDEO, "VIDEO" }, \
     { DLTF_STILL_PICTURE, "STILL_PICTURE" }, \
     { DLTF_BACKGROUND, "BACKGROUND" }, \
     { (DFBDisplayLayerTypeFlags) DLTF_NONE, "NONE" } \
};


struct DFBDisplayLayerCapabilitiesName {
     DFBDisplayLayerCapabilities capability;
     const char *name;
};

#define DirectFBDisplayLayerCapabilitiesNames(Identifier) struct DFBDisplayLayerCapabilitiesName Identifier[] = { \
     { DLCAPS_SURFACE, "SURFACE" }, \
     { DLCAPS_OPACITY, "OPACITY" }, \
     { DLCAPS_ALPHACHANNEL, "ALPHACHANNEL" }, \
     { DLCAPS_SCREEN_LOCATION, "SCREEN_LOCATION" }, \
     { DLCAPS_FLICKER_FILTERING, "FLICKER_FILTERING" }, \
     { DLCAPS_DEINTERLACING, "DEINTERLACING" }, \
     { DLCAPS_SRC_COLORKEY, "SRC_COLORKEY" }, \
     { DLCAPS_DST_COLORKEY, "DST_COLORKEY" }, \
     { DLCAPS_BRIGHTNESS, "BRIGHTNESS" }, \
     { DLCAPS_CONTRAST, "CONTRAST" }, \
     { DLCAPS_HUE, "HUE" }, \
     { DLCAPS_SATURATION, "SATURATION" }, \
     { DLCAPS_LEVELS, "LEVELS" }, \
     { DLCAPS_FIELD_PARITY, "FIELD_PARITY" }, \
     { DLCAPS_WINDOWS, "WINDOWS" }, \
     { DLCAPS_SOURCES, "SOURCES" }, \
     { DLCAPS_ALPHA_RAMP, "ALPHA_RAMP" }, \
     { DLCAPS_PREMULTIPLIED, "PREMULTIPLIED" }, \
     { DLCAPS_SCREEN_POSITION, "SCREEN_POSITION" }, \
     { DLCAPS_SCREEN_SIZE, "SCREEN_SIZE" }, \
     { DLCAPS_CLIP_REGIONS, "CLIP_REGIONS" }, \
     { DLCAPS_LR_MONO, "LR_MONO" }, \
     { DLCAPS_STEREO, "STEREO" }, \
     { (DFBDisplayLayerCapabilities) DLCAPS_NONE, "NONE" } \
};


struct DFBDisplayLayerBufferModeName {
     DFBDisplayLayerBufferMode mode;
     const char *name;
};

#define DirectFBDisplayLayerBufferModeNames(Identifier) struct DFBDisplayLayerBufferModeName Identifier[] = { \
     { DLBM_FRONTONLY, "FRONTONLY" }, \
     { DLBM_BACKVIDEO, "BACKVIDEO" }, \
     { DLBM_BACKSYSTEM, "BACKSYSTEM" }, \
     { DLBM_TRIPLE, "TRIPLE" }, \
     { DLBM_WINDOWS, "WINDOWS" }, \
     { (DFBDisplayLayerBufferMode) DLBM_UNKNOWN, "UNKNOWN" } \
};


struct DFBWindowCapabilitiesName {
     DFBWindowCapabilities capability;
     const char *name;
};

#define DirectFBWindowCapabilitiesNames(Identifier) struct DFBWindowCapabilitiesName Identifier[] = { \
     { DWCAPS_LR_MONO, "LR_MONO" }, \
     { DWCAPS_ALPHACHANNEL, "ALPHACHANNEL" }, \
     { DWCAPS_DOUBLEBUFFER, "DOUBLEBUFFER" }, \
     { DWCAPS_INPUTONLY, "INPUTONLY" }, \
     { DWCAPS_NODECORATION, "NODECORATION" }, \
     { DWCAPS_SUBWINDOW, "SUBWINDOW" }, \
     { DWCAPS_COLOR, "COLOR" }, \
     { DWCAPS_NOFOCUS, "NOFOCUS" }, \
     { DWCAPS_LR_MONO, "LR_MONO" }, \
     { DWCAPS_STEREO, "STEREO" }, \
     { (DFBWindowCapabilities) DWCAPS_NONE, "NONE" } \
};


struct DFBDisplayLayerOptionsName {
     DFBDisplayLayerOptions option;
     const char *name;
};

#define DirectFBDisplayLayerOptionsNames(Identifier) struct DFBDisplayLayerOptionsName Identifier[] = { \
     { DLOP_ALPHACHANNEL, "ALPHACHANNEL" }, \
     { DLOP_FLICKER_FILTERING, "FLICKER_FILTERING" }, \
     { DLOP_DEINTERLACING, "DEINTERLACING" }, \
     { DLOP_SRC_COLORKEY, "SRC_COLORKEY" }, \
     { DLOP_DST_COLORKEY, "DST_COLORKEY" }, \
     { DLOP_OPACITY, "OPACITY" }, \
     { DLOP_FIELD_PARITY, "FIELD_PARITY" }, \
     { DLOP_LR_MONO, "LR_MONO" }, \
     { DLOP_STEREO, "STEREO" }, \
     { DLOP_STEREO, "STEREO" }, \
     { (DFBDisplayLayerOptions) DLOP_NONE, "NONE" } \
};


struct DFBWindowOptionsName {
     DFBWindowOptions option;
     const char *name;
};

#define DirectFBWindowOptionsNames(Identifier) struct DFBWindowOptionsName Identifier[] = { \
     { DWOP_COLORKEYING, "COLORKEYING" }, \
     { DWOP_ALPHACHANNEL, "ALPHACHANNEL" }, \
     { DWOP_OPAQUE_REGION, "OPAQUE_REGION" }, \
     { DWOP_SHAPED, "SHAPED" }, \
     { DWOP_ALPHACHANNEL, "ALPHACHANNEL" }, \
     { DWOP_KEEP_POSITION, "KEEP_POSITION" }, \
     { DWOP_KEEP_SIZE, "KEEP_SIZE" }, \
     { DWOP_KEEP_STACKING, "KEEP_STACKING" }, \
     { DWOP_GHOST, "GHOST" }, \
     { DWOP_INDESTRUCTIBLE, "INDESTRUCTIBLE" }, \
     { DWOP_INPUTONLY, "INPUTONLY" }, \
     { DWOP_STEREO_SIDE_BY_SIDE_HALF, "STEREO_SIDE_BY_SIDE_HALF" }, \
     { DWOP_SCALE, "SCALE" }, \
     { DWOP_KEEP_ABOVE, "KEEP_ABOVE" }, \
     { DWOP_KEEP_UNDER, "KEEP_UNDER" }, \
     { DWOP_FOLLOW_BOUNDS, "FOLLOW_BOUNDS" }, \
     { (DFBWindowOptions) DWOP_NONE, "NONE" } \
};


struct DFBScreenCapabilitiesName {
     DFBScreenCapabilities capability;
     const char *name;
};

#define DirectFBScreenCapabilitiesNames(Identifier) struct DFBScreenCapabilitiesName Identifier[] = { \
     { DSCCAPS_VSYNC, "VSYNC" }, \
     { DSCCAPS_POWER_MANAGEMENT, "POWER_MANAGEMENT" }, \
     { DSCCAPS_MIXERS, "MIXERS" }, \
     { DSCCAPS_ENCODERS, "ENCODERS" }, \
     { DSCCAPS_OUTPUTS, "OUTPUTS" }, \
     { (DFBScreenCapabilities) DSCCAPS_NONE, "NONE" } \
};


struct DFBScreenEncoderCapabilitiesName {
     DFBScreenEncoderCapabilities capability;
     const char *name;
};

#define DirectFBScreenEncoderCapabilitiesNames(Identifier) struct DFBScreenEncoderCapabilitiesName Identifier[] = { \
     { DSECAPS_TV_STANDARDS, "TV_STANDARDS" }, \
     { DSECAPS_TEST_PICTURE, "TEST_PICTURE" }, \
     { DSECAPS_MIXER_SEL, "MIXER_SEL" }, \
     { DSECAPS_OUT_SIGNALS, "OUT_SIGNALS" }, \
     { DSECAPS_SCANMODE, "SCANMODE" }, \
     { DSECAPS_FREQUENCY, "FREQUENCY" }, \
     { DSECAPS_BRIGHTNESS, "BRIGHTNESS" }, \
     { DSECAPS_CONTRAST, "CONTRAST" }, \
     { DSECAPS_HUE, "HUE" }, \
     { DSECAPS_SATURATION, "SATURATION" }, \
     { DSECAPS_CONNECTORS, "CONNECTORS" }, \
     { DSECAPS_SLOW_BLANKING, "SLOW_BLANKING" }, \
     { DSECAPS_RESOLUTION, "RESOLUTION" }, \
     { DSECAPS_FRAMING, "FRAMING" }, \
     { DSECAPS_ASPECT_RATIO, "ASPECT_RATIO" }, \
     { (DFBScreenEncoderCapabilities) DSECAPS_NONE, "NONE" } \
};


struct DFBScreenEncoderTypeName {
     DFBScreenEncoderType type;
     const char *name;
};

#define DirectFBScreenEncoderTypeNames(Identifier) struct DFBScreenEncoderTypeName Identifier[] = { \
     { DSET_CRTC, "CRTC" }, \
     { DSET_TV, "TV" }, \
     { DSET_DIGITAL, "DIGITAL" }, \
     { (DFBScreenEncoderType) DSET_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenEncoderTVStandardsName {
     DFBScreenEncoderTVStandards standard;
     const char *name;
};

#define DirectFBScreenEncoderTVStandardsNames(Identifier) struct DFBScreenEncoderTVStandardsName Identifier[] = { \
     { DSETV_PAL, "PAL" }, \
     { DSETV_NTSC, "NTSC" }, \
     { DSETV_SECAM, "SECAM" }, \
     { DSETV_PAL_60, "PAL_60" }, \
     { DSETV_PAL_BG, "PAL_BG" }, \
     { DSETV_PAL_I, "PAL_I" }, \
     { DSETV_PAL_M, "PAL_M" }, \
     { DSETV_PAL_N, "PAL_N" }, \
     { DSETV_PAL_NC, "PAL_NC" }, \
     { DSETV_NTSC_M_JPN, "NTSC_M_JPN" }, \
     { DSETV_DIGITAL, "DIGITAL" }, \
     { DSETV_NTSC_443, "NTSC_443" }, \
     { (DFBScreenEncoderTVStandards) DSETV_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenOutputCapabilitiesName {
     DFBScreenOutputCapabilities capability;
     const char *name;
};

#define DirectFBScreenOutputCapabilitiesNames(Identifier) struct DFBScreenOutputCapabilitiesName Identifier[] = { \
     { DSOCAPS_CONNECTORS, "CONNECTORS" }, \
     { DSOCAPS_ENCODER_SEL, "ENCODER_SEL" }, \
     { DSOCAPS_SIGNAL_SEL, "SIGNAL_SEL" }, \
     { DSOCAPS_CONNECTOR_SEL, "CONNECTOR_SEL" }, \
     { DSOCAPS_SLOW_BLANKING, "SLOW_BLANKING" }, \
     { DSOCAPS_RESOLUTION, "RESOLUTION" }, \
     { (DFBScreenOutputCapabilities) DSOCAPS_NONE, "NONE" } \
};


struct DFBScreenOutputConnectorsName {
     DFBScreenOutputConnectors connector;
     const char *name;
};

#define DirectFBScreenOutputConnectorsNames(Identifier) struct DFBScreenOutputConnectorsName Identifier[] = { \
     { DSOC_VGA, "VGA" }, \
     { DSOC_SCART, "SCART" }, \
     { DSOC_YC, "YC" }, \
     { DSOC_CVBS, "CVBS" }, \
     { DSOC_SCART2, "SCART2" }, \
     { DSOC_COMPONENT, "COMPONENT" }, \
     { DSOC_HDMI, "HDMI" }, \
     { DSOC_656, "656" }, \
     { (DFBScreenOutputConnectors) DSOC_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenOutputSignalsName {
     DFBScreenOutputSignals signal;
     const char *name;
};

#define DirectFBScreenOutputSignalsNames(Identifier) struct DFBScreenOutputSignalsName Identifier[] = { \
     { DSOS_VGA, "VGA" }, \
     { DSOS_YC, "YC" }, \
     { DSOS_CVBS, "CVBS" }, \
     { DSOS_RGB, "RGB" }, \
     { DSOS_YCBCR, "YCBCR" }, \
     { DSOS_HDMI, "HDMI" }, \
     { DSOS_656, "656" }, \
     { (DFBScreenOutputSignals) DSOS_NONE, "NONE" } \
};


struct DFBScreenOutputSlowBlankingSignalsName {
     DFBScreenOutputSlowBlankingSignals slow_signal;
     const char *name;
};

#define DirectFBScreenOutputSlowBlankingSignalsNames(Identifier) struct DFBScreenOutputSlowBlankingSignalsName Identifier[] = { \
     { DSOSB_16x9, "16x9" }, \
     { DSOSB_4x3, "4x3" }, \
     { DSOSB_FOLLOW, "FOLLOW" }, \
     { DSOSB_MONITOR, "MONITOR" }, \
     { (DFBScreenOutputSlowBlankingSignals) DSOSB_OFF, "OFF" } \
};


struct DFBScreenOutputResolutionName {
     DFBScreenOutputResolution resolution;
     const char *name;
};

#define DirectFBScreenOutputResolutionNames(Identifier) struct DFBScreenOutputResolutionName Identifier[] = { \
     { DSOR_640_480, "640_480" }, \
     { DSOR_720_480, "720_480" }, \
     { DSOR_720_576, "720_576" }, \
     { DSOR_800_600, "800_600" }, \
     { DSOR_1024_768, "1024_768" }, \
     { DSOR_1152_864, "1152_864" }, \
     { DSOR_1280_720, "1280_720" }, \
     { DSOR_1280_768, "1280_768" }, \
     { DSOR_1280_960, "1280_960" }, \
     { DSOR_1280_1024, "1280_1024" }, \
     { DSOR_1400_1050, "1400_1050" }, \
     { DSOR_1600_1200, "1600_1200" }, \
     { DSOR_1920_1080, "1920_1080" }, \
     { DSOR_960_540, "960_540" }, \
     { DSOR_1440_540, "1440_540" }, \
     { DSOR_800_480, "800_480" }, \
     { DSOR_1024_600, "1024_600" }, \
     { DSOR_1366_768, "1366_768" }, \
     { DSOR_1920_1200, "1920_1200" }, \
     { DSOR_2560_1440, "2560_1440" }, \
     { DSOR_2560_1600, "2560_1600" }, \
     { DSOR_3840_2160, "3840_2160" }, \
     { DSOR_4096_2160, "4096_2160" }, \
     { (DFBScreenOutputResolution) DSOR_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenMixerCapabilitiesName {
     DFBScreenMixerCapabilities capability;
     const char *name;
};

#define DirectFBScreenMixerCapabilitiesNames(Identifier) struct DFBScreenMixerCapabilitiesName Identifier[] = { \
     { DSMCAPS_FULL, "FULL" }, \
     { DSMCAPS_SUB_LEVEL, "SUB_LEVEL" }, \
     { DSMCAPS_SUB_LAYERS, "SUB_LAYERS" }, \
     { DSMCAPS_BACKGROUND, "BACKGROUND" }, \
     { (DFBScreenMixerCapabilities) DSMCAPS_NONE, "NONE" } \
};


struct DFBScreenMixerTreeName {
     DFBScreenMixerTree tree;
     const char *name;
};

#define DirectFBScreenMixerTreeNames(Identifier) struct DFBScreenMixerTreeName Identifier[] = { \
     { DSMT_FULL, "FULL" }, \
     { DSMT_SUB_LEVEL, "SUB_LEVEL" }, \
     { DSMT_SUB_LAYERS, "SUB_LAYERS" }, \
     { (DFBScreenMixerTree) DSMT_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenEncoderTestPictureName {
     DFBScreenEncoderTestPicture test_picture;
     const char *name;
};

#define DirectFBScreenEncoderTestPictureNames(Identifier) struct DFBScreenEncoderTestPictureName Identifier[] = { \
     { DSETP_MULTI, "MULTI" }, \
     { DSETP_SINGLE, "SINGLE" }, \
     { DSETP_WHITE, "WHITE" }, \
     { DSETP_YELLOW, "YELLOW" }, \
     { DSETP_CYAN, "CYAN" }, \
     { DSETP_GREEN, "GREEN" }, \
     { DSETP_MAGENTA, "MAGENTA" }, \
     { DSETP_RED, "RED" }, \
     { DSETP_BLUE, "BLUE" }, \
     { DSETP_BLACK, "BLACK" }, \
     { (DFBScreenEncoderTestPicture) DSETP_OFF, "OFF" } \
};


struct DFBScreenEncoderScanModeName {
     DFBScreenEncoderScanMode scan_mode;
     const char *name;
};

#define DirectFBScreenEncoderScanModeNames(Identifier) struct DFBScreenEncoderScanModeName Identifier[] = { \
     { DSESM_INTERLACED, "INTERLACED" }, \
     { DSESM_PROGRESSIVE, "PROGRESSIVE" }, \
     { (DFBScreenEncoderScanMode) DSESM_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenEncoderConfigFlagsName {
     DFBScreenEncoderConfigFlags config_flags;
     const char *name;
};

#define DirectFBScreenEncoderConfigFlagsNames(Identifier) struct DFBScreenEncoderConfigFlagsName Identifier[] = { \
     { DSECONF_NONE, "NONE" }, \
     { DSECONF_TV_STANDARD, "TV_STANDARD" }, \
     { DSECONF_TEST_PICTURE, "TEST_PICTURE" }, \
     { DSECONF_MIXER, "MIXER" }, \
     { DSECONF_OUT_SIGNALS, "OUT_SIGNALS" }, \
     { DSECONF_SCANMODE, "SCANMODE" }, \
     { DSECONF_TEST_COLOR, "TEST_COLOR" }, \
     { DSECONF_ADJUSTMENT, "ADJUSTMENT" }, \
     { DSECONF_FREQUENCY, "FREQUENCY" }, \
     { DSECONF_CONNECTORS, "CONNECTORS" }, \
     { DSECONF_SLOW_BLANKING, "SLOW_BLANKING" }, \
     { DSECONF_RESOLUTION, "RESOLUTION" }, \
     { DSECONF_FRAMING, "FRAMING" }, \
     { DSECONF_ASPECT_RATIO, "ASPECT_RATIO" }, \
     { (DFBScreenEncoderConfigFlags) DSECONF_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenEncoderFrequencyName {
     DFBScreenEncoderFrequency frequency;
     const char *name;
};

#define DirectFBScreenEncoderFrequencyNames(Identifier) struct DFBScreenEncoderFrequencyName Identifier[] = { \
     { DSEF_25HZ, "25HZ" }, \
     { DSEF_29_97HZ, "29_97HZ" }, \
     { DSEF_50HZ, "50HZ" }, \
     { DSEF_59_94HZ, "59_94HZ" }, \
     { DSEF_60HZ, "60HZ" }, \
     { DSEF_75HZ, "75HZ" }, \
     { DSEF_30HZ, "30HZ" }, \
     { DSEF_24HZ, "24HZ" }, \
     { DSEF_23_976HZ, "23_976HZ" }, \
     { (DFBScreenEncoderFrequency) DSEF_UNKNOWN, "UNKNOWN" } \
};


struct DFBScreenEncoderPictureFramingName {
     DFBScreenEncoderPictureFraming framing;
     const char *name;
};

#define DirectFBScreenEncoderPictureFramingNames(Identifier) struct DFBScreenEncoderPictureFramingName Identifier[] = { \
     { DSEPF_MONO, "MONO" }, \
     { DSEPF_STEREO_SIDE_BY_SIDE_HALF, "STEREO_SIDE_BY_SIDE_HALF" }, \
     { DSEPF_STEREO_TOP_AND_BOTTOM, "STEREO_TOP_AND_BOTTOM" }, \
     { DSEPF_STEREO_FRAME_PACKING, "STEREO_FRAME_PACKING" }, \
     { DSEPF_STEREO_SIDE_BY_SIDE_FULL, "STEREO_SIDE_BY_SIDE_FULL" }, \
     { (DFBScreenEncoderPictureFraming) DSEPF_UNKNOWN, "UNKNOWN" } \
};


struct DFBAccelerationMaskName {
     DFBAccelerationMask mask;
     const char *name;
};

#define DirectFBAccelerationMaskNames(Identifier) struct DFBAccelerationMaskName Identifier[] = { \
     { DFXL_FILLRECTANGLE, "FILLRECTANGLE" }, \
     { DFXL_DRAWRECTANGLE, "DRAWRECTANGLE" }, \
     { DFXL_DRAWLINE, "DRAWLINE" }, \
     { DFXL_FILLTRIANGLE, "FILLTRIANGLE" }, \
     { DFXL_FILLTRAPEZOID, "FILLTRAPEZOID" }, \
     { DFXL_FILLQUADRANGLE, "FILLQUADRANGLE" }, \
     { DFXL_FILLSPAN, "FILLSPAN" }, \
     { DFXL_DRAWMONOGLYPH, "DRAWMONOGLYPH" }, \
     { DFXL_BLIT, "BLIT" }, \
     { DFXL_STRETCHBLIT, "STRETCHBLIT" }, \
     { DFXL_TEXTRIANGLES, "TEXTRIANGLES" }, \
     { DFXL_BLIT2, "BLIT2" }, \
     { DFXL_TILEBLIT, "TILEBLIT" }, \
     { DFXL_DRAWSTRING, "DRAWSTRING" }, \
     { (DFBAccelerationMask) DFXL_NONE, "NONE" } \
};

#endif
