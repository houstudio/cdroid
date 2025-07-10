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
#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <core/rect.h>
#include <core/displaymetrics.h>
namespace cdroid{
class WindowManager;
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
    friend WindowManager;
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
    static constexpr int STATE_UNKNOWN = 0;//ViewProtoEnums.DISPLAY_STATE_UNKNOWN;
    static constexpr int STATE_OFF = 1;//ViewProtoEnums.DISPLAY_STATE_OFF;
    static constexpr int STATE_ON  = 2;//ViewProtoEnums.DISPLAY_STATE_ON; 
    static constexpr int STATE_DOZE= 3;//ViewProtoEnums.DISPLAY_STATE_DOZE;
    static constexpr int STATE_SUSPEND= 4;//ViewProtoEnums.DISPLAY_STATE_SUSPEND;
    static constexpr int STATE_VR= 5;//ViewProtoEnums.DISPLAY_STATE_VR;
    static constexpr int STATE_ON_SUSPEND=6;//ViewProtoEnums.DISPLAY_STATE_ON_SUSPEND;
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
    int getState();
    void getMetrics(DisplayMetrics&outMetrics);
    void getRealMetrics(DisplayMetrics&outMetrics);
};
}
#endif//__DISPLAY_H__
