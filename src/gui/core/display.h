#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <core/rect.h>
#include <core/displaymetrics.h>
namespace cdroid{

struct DisplayInfo{
    int type;
    /**
     * The width/height of the portion of the display that is available to applications, in pixels.
     * Represents the size of the display minus any system decorations.
     */
    int appWidth;
    int appHeight;
    /**
     * The logical width of the display, in pixels.
     * Represents the usable size of the display which may be smaller than the
     * physical size when the system is emulating a smaller display.
     */
    int logicalWidth;
    int logicalHeight;
    /**Number of overscan pixels on the left side of the display.*/
    int overscanLeft;
    /** Number of overscan pixels on the top side of the display.*/
    int overscanTop;
    /** Number of overscan pixels on the right side of the display.*/
    int overscanRight;

    /** Number of overscan pixels on the bottom side of the display. */
    int overscanBottom;
    int rotation;/*Surface's Rotation in anticlockwise*/
    int logicalDensityDpi;
    float physicalXDpi;
    float physicalYDpi;
    int state;/*Display.STATE*/
};

class Display{
private:
    int mDisplayId;
    int mFlags;
    int mType;
    bool mIsValid;
    DisplayInfo mDisplayInfo;
public:
    static constexpr int DEFAULT_DISPLAY = 0;
    static constexpr int INVALID_DISPLAY = -1;
    enum DisplayType{
        TYPE_UNKNOWN,
        TYPE_BUILT_IN,
        TYPE_HDMI,
        TYPE_WIFI,
        TYPE_OVERLAY,
        TYPE_VIRTUAL
    };
    enum Rotation{
        ROTATION_0,
        ROTATION_90,
        ROTATION_180,
        ROTATION_270
    };
private:
    void updateDisplayInfoLocked();   
public:
    Display(int id,DisplayInfo&displayInfo);
    int  getDisplayId();
    bool isValid();
    bool getDisplayInfo(DisplayInfo&);
    int  getType();
    void getSize(Point&outSize);
    void getRealSize(Point&outSize);
    int  getRotation();
    void getMetrics(DisplayMetrics&outMetrics);
    void getRealMetrics(DisplayMetrics&outMetrics);
};
}
#endif//__DISPLAY_H__
