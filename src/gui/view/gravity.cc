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
#include <view/view.h>
#include <iostream>
#include <sstream>
#include <cdlog.h>


namespace cdroid{

void Gravity::apply(int gravity, int w, int h,const Rect& container,Rect& outRect){
    return apply(gravity, w, h, container, 0, 0, outRect);
}

void Gravity::apply(int gravity, int w, int h,const Rect& container,Rect& outRect, int layoutDirection){
    const int absGravity = getAbsoluteGravity(gravity, layoutDirection);
    apply(absGravity, w, h, container, 0, 0, outRect);
}

void Gravity::apply(int gravity, int w, int h,const Rect& container,int xAdj, int yAdj, Rect& outRect){
    int tmp=0;
    switch (gravity&((AXIS_PULL_BEFORE|AXIS_PULL_AFTER)<<AXIS_X_SHIFT)) {
    case 0:
            outRect.left = container.left + ((container.width - w)/2) + xAdj;
            outRect.width= w;//outRect.right = outRect.left + w;
            if ((gravity&(AXIS_CLIP<<AXIS_X_SHIFT)) == (AXIS_CLIP<<AXIS_X_SHIFT)) {
                if (outRect.left < container.left) {
                    outRect.left = container.left;//outRect.left = container.left;
                }
                if (outRect.right() > container.right()) {
                    outRect.width = container.right()-outRect.left;//outRect.right = container.right;
                }
            }
            break;
    case AXIS_PULL_BEFORE<<AXIS_X_SHIFT:
            outRect.left = container.left + xAdj;
            outRect.width= w;//outRect.right = outRect.left + w;
            if ((gravity&(AXIS_CLIP<<AXIS_X_SHIFT)) == (AXIS_CLIP<<AXIS_X_SHIFT)) {
                if (outRect.right() > container.right()) {
                    outRect.width=container.right()-outRect.left;//outRect.right = container.right;
                }
            }
            break;
    case AXIS_PULL_AFTER<<AXIS_X_SHIFT:
            outRect.left  = container.right()-w-xAdj;
            outRect.width = w;
            if ((gravity&(AXIS_CLIP<<AXIS_X_SHIFT))== (AXIS_CLIP<<AXIS_X_SHIFT)) {
                if (outRect.left < container.left) {
                    outRect.left = container.left;
                }
            }
            break;
    default:
            outRect.left = container.left + xAdj;
            outRect.width=container.width+xAdj;//outRect.right = container.right + xAdj;
            break;
    }
        
    switch (gravity&((AXIS_PULL_BEFORE|AXIS_PULL_AFTER)<<AXIS_Y_SHIFT)) {
    case 0:
            outRect.top = container.top + ((container.height - h)/2) + yAdj;
            outRect.height = h;
            if ((gravity&(AXIS_CLIP<<AXIS_Y_SHIFT)) == (AXIS_CLIP<<AXIS_Y_SHIFT)) {
                if (outRect.top < container.top) {
                    outRect.top = container.top;
                }
                if (outRect.bottom() > container.bottom()) {
                    outRect.height = container.bottom() - outRect.top;
                }
            }
            break;
    case AXIS_PULL_BEFORE<<AXIS_Y_SHIFT:
            outRect.top = container.top + yAdj;
            outRect.height = h;
            if ((gravity&(AXIS_CLIP<<AXIS_Y_SHIFT)) == (AXIS_CLIP<<AXIS_Y_SHIFT)) {
                if (outRect.bottom() > container.bottom()) {
                    outRect.height = container.bottom() - outRect.top;
                }
            }
            break;
    case AXIS_PULL_AFTER<<AXIS_Y_SHIFT:
            outRect.height = h;
            outRect.top = container.bottom() - yAdj - h;
            if ((gravity&(AXIS_CLIP<<AXIS_Y_SHIFT)) == (AXIS_CLIP<<AXIS_Y_SHIFT)) {
                if (outRect.top < container.top) {
                    outRect.top = container.top;
                }
            }
            break;
    default:
            outRect.top = container.top + yAdj;
            outRect.height = container.height;
            break;
    }
}

void Gravity::apply(int gravity, int w, int h,const Rect& container,int xAdj, int yAdj, Rect& outRect, int layoutDirection){
    const int absGravity = getAbsoluteGravity(gravity, layoutDirection);
    apply(absGravity, w, h, container, xAdj, yAdj, outRect);
}

void Gravity::applyDisplay(int gravity,const Rect& display,Rect& inoutObj){
    if ((gravity&DISPLAY_CLIP_VERTICAL) != 0) {
        if (inoutObj.top < display.top) inoutObj.top = display.top;
        if (inoutObj.bottom() > display.bottom()) inoutObj.width=display.bottom()-inoutObj.top;//inoutObj.bottom = display.bottom;
    } else {
        int off = 0;
        if (inoutObj.top < display.top) off = display.top-inoutObj.top;
        else if (inoutObj.bottom() > display.bottom()) off = display.bottom()-inoutObj.bottom();
        if (off != 0) {
            if (inoutObj.height > display.height) {
                inoutObj.top = display.top;
                inoutObj.height = display.height;//bottom;
            } else {
                inoutObj.top+=off;//inoutObj.top += off;inoutObj.bottom += off;
            }
        }
    }
        
    if ((gravity&DISPLAY_CLIP_HORIZONTAL) != 0) {
        if (inoutObj.left < display.left) inoutObj.left = display.left;
        if (inoutObj.right() > display.right()) inoutObj.width=display.right()-inoutObj.left;//inoutObj.right = display.right;
    } else {
        int off = 0;
        if (inoutObj.left < display.left) off = display.left-inoutObj.left;
        else if (inoutObj.right() > display.right()) off = display.right()-inoutObj.right();
        if (off != 0) {
            if (inoutObj.width > display.width) {
                inoutObj.left = display.left;
                inoutObj.width=display.width;//inoutObj.right = display.right;
            } else {
                //inoutObj.left += off;  inoutObj.right += off;
                inoutObj.left+=off;
            }
        }
    }
}

void Gravity::applyDisplay(int gravity,const Rect& display,Rect& inoutObj, int layoutDirection){
    int absGravity = getAbsoluteGravity(gravity, layoutDirection);
    applyDisplay(absGravity, display, inoutObj);
}

bool Gravity::isVertical(int gravity){
    return gravity > 0 && (gravity & VERTICAL_GRAVITY_MASK) != 0;
}

bool Gravity::isHorizontal(int gravity){
    return gravity > 0 && (gravity & RELATIVE_HORIZONTAL_GRAVITY_MASK) != 0;
}

int Gravity::getAbsoluteGravity(int gravity, int layoutDirection){
    int result = gravity;
    // If layout is script specific and gravity is horizontal relative (START or END)
    if ((result & RELATIVE_LAYOUT_DIRECTION) > 0) {
        if ((result & Gravity::START) == Gravity::START) {
            // Remove the START bit
            result &= ~START;
            if (layoutDirection == View::LAYOUT_DIRECTION_RTL) {
                // Set the RIGHT bit
                result |= RIGHT;
            } else { // Set the LEFT bit
                result |= LEFT;
            }
        } else if ((result & Gravity::END) == Gravity::END) {
            // Remove the END bit
            result &= ~END;
            if (layoutDirection == View::LAYOUT_DIRECTION_RTL) {
                // Set the LEFT bit
                result |= LEFT;
            } else { // Set the RIGHT bit
                result |= RIGHT;
            }
        }
        // Don't need the script specific bit any more, so remove it as we are converting to
        // absolute values (LEFT or RIGHT)
        result &= ~RELATIVE_LAYOUT_DIRECTION;
    }
    return result;
}

const std::string  Gravity::toString(int gravity){
    std::ostringstream result;
    if ((gravity & FILL) == FILL) {
        result<<"FILL ";
    } else {
        if ((gravity & FILL_VERTICAL) == FILL_VERTICAL) {
            result<<"FILL_VERTICAL ";
        } else {
            if ((gravity & TOP) == TOP) {
                result<<"TOP ";
            }
            if ((gravity & BOTTOM) == BOTTOM) {
                result<<"BOTTOM ";
            }
        }
        if ((gravity & FILL_HORIZONTAL) == FILL_HORIZONTAL) {
            result<<"FILL_HORIZONTAL ";
        } else {
            if ((gravity & START) == START) {
                result<<"START ";
            } else if ((gravity & LEFT) == LEFT) {
                result<<"LEFT ";
            }
            if ((gravity & END) == END) {
                result<<"END ";
            } else if ((gravity & RIGHT) == RIGHT) {
                result<<"RIGHT ";
            }
        }
    }
    if ((gravity & CENTER) == CENTER) {
        result<<"CENTER ";
    } else {
        if ((gravity & CENTER_VERTICAL) == CENTER_VERTICAL) {
            result<<"CENTER_VERTICAL ";
        }
        if ((gravity & CENTER_HORIZONTAL) == CENTER_HORIZONTAL) {
            result<<"CENTER_HORIZONTAL ";
        }
    }
    if (result.str().length() == 0) {
        result<<"NO GRAVITY ";
    }
    if ((gravity & DISPLAY_CLIP_VERTICAL) == DISPLAY_CLIP_VERTICAL) {
        result<<"DISPLAY_CLIP_VERTICAL ";
    }
    if ((gravity & DISPLAY_CLIP_HORIZONTAL) == DISPLAY_CLIP_HORIZONTAL) {
        result<<"DISPLAY_CLIP_HORIZONTAL ";
    }
    //result.deleteCharAt(result.length() - 1);
    return result.str();
}


}
