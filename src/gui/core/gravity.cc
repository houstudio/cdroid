#include <gravity.h>
#include <widget/view.h>
#include <iostream>
#include <sstream>
#include <cdlog.h>


namespace cdroid{

void Gravity::apply(int gravity, int w, int h,const RECT& container,RECT& outRect){
    return apply(gravity, w, h, container, 0, 0, outRect);
}

void Gravity::apply(int gravity, int w, int h,const RECT& container,RECT& outRect, int layoutDirection){
    int absGravity = getAbsoluteGravity(gravity, layoutDirection);
    apply(absGravity, w, h, container, 0, 0, outRect);
}

void Gravity::apply(int gravity, int w, int h,const RECT& container,int xAdj, int yAdj, RECT& outRect){
    int tmp=0;
    switch (gravity&((AXIS_PULL_BEFORE|AXIS_PULL_AFTER)<<AXIS_X_SHIFT)) {
    case 0:
            outRect.x = container.x + ((container.width - w)/2) + xAdj;
            outRect.width=w;//outRect.right = outRect.left + w;
            if ((gravity&(AXIS_CLIP<<AXIS_X_SHIFT)) == (AXIS_CLIP<<AXIS_X_SHIFT)) {
                if (outRect.x < container.x) {
                    outRect.x = container.x;//outRect.left = container.left;
                }
                if (outRect.right() > container.right()) {
                    outRect.width = container.right()-outRect.x;//outRect.right = container.right;
                }
            }
            break;
    case AXIS_PULL_BEFORE<<AXIS_X_SHIFT:
            outRect.x = container.x + xAdj;
            outRect.width=w;//outRect.right = outRect.left + w;
            if ((gravity&(AXIS_CLIP<<AXIS_X_SHIFT)) == (AXIS_CLIP<<AXIS_X_SHIFT)) {
                if (outRect.right() > container.right()) {
                    outRect.width=container.right()-outRect.x;//outRect.right = container.right;
                }
            }
            break;
    case AXIS_PULL_AFTER<<AXIS_X_SHIFT:
            outRect.width=container.width-xAdj;//outRect.right = container.right - xAdj;
            outRect.x=outRect.right()-w;//outRect.left = outRect.right - w;
            if ((gravity&(AXIS_CLIP<<AXIS_X_SHIFT))== (AXIS_CLIP<<AXIS_X_SHIFT)) {
                if (outRect.x < container.x) {
                    outRect.x = container.x;
                }
            }
            break;
    default:
            outRect.x = container.x + xAdj;
            outRect.width=container.width+xAdj;//outRect.right = container.right + xAdj;
            break;
    }
        
    switch (gravity&((AXIS_PULL_BEFORE|AXIS_PULL_AFTER)<<AXIS_Y_SHIFT)) {
    case 0:
            outRect.y = container.y + ((container.height - h)/2) + yAdj;
            outRect.height=h;//outRect.bottom = outRect.top + h;
            if ((gravity&(AXIS_CLIP<<AXIS_Y_SHIFT)) == (AXIS_CLIP<<AXIS_Y_SHIFT)) {
                if (outRect.y < container.y) {
                    outRect.y = container.y;
                }
                if (outRect.bottom() > container.bottom()) {
                    outRect.height=container.bottom()-outRect.y;//outRect.bottom = container.bottom;
                }
            }
            break;
    case AXIS_PULL_BEFORE<<AXIS_Y_SHIFT:
            outRect.y = container.y + yAdj;
            outRect.width=h;//outRect.bottom = outRect.top + h;
            if ((gravity&(AXIS_CLIP<<AXIS_Y_SHIFT)) == (AXIS_CLIP<<AXIS_Y_SHIFT)) {
                if (outRect.bottom() > container.bottom()) {
                    outRect.width=container.bottom()-outRect.y;//outRect.bottom = container.bottom;
                }
            }
            break;
    case AXIS_PULL_AFTER<<AXIS_Y_SHIFT:
            outRect.height=container.bottom() - yAdj;//outRect.bottom = container.bottom - yAdj;
            outRect.y=outRect.height-h;//outRect.top = outRect.bottom - h;
            if ((gravity&(AXIS_CLIP<<AXIS_Y_SHIFT)) == (AXIS_CLIP<<AXIS_Y_SHIFT)) {
                if (outRect.y < container.y) {
                    outRect.y = container.y;
                }
            }
            break;
    default:
            outRect.y = container.y + yAdj;
            outRect.height=container.bottom() + yAdj-outRect.y;//outRect.bottom = container.bottom + yAdj;
            break;
    }
}

void Gravity::apply(int gravity, int w, int h,const RECT& container,int xAdj, int yAdj, RECT& outRect, int layoutDirection){
    int absGravity = getAbsoluteGravity(gravity, layoutDirection);
    apply(absGravity, w, h, container, xAdj, yAdj, outRect);
}

void Gravity::applyDisplay(int gravity,const RECT& display,RECT& inoutObj){
    if ((gravity&DISPLAY_CLIP_VERTICAL) != 0) {
        if (inoutObj.y < display.y) inoutObj.y = display.y;
        if (inoutObj.bottom() > display.bottom()) inoutObj.width=display.bottom()-inoutObj.y;//inoutObj.bottom = display.bottom;
    } else {
        int off = 0;
        if (inoutObj.y < display.y) off = display.y-inoutObj.y;
        else if (inoutObj.bottom() > display.bottom()) off = display.bottom()-inoutObj.bottom();
        if (off != 0) {
            if (inoutObj.height > display.height) {
                inoutObj.y = display.y;
                inoutObj.height = display.height;//bottom;
            } else {
                inoutObj.y+=off;//inoutObj.top += off;inoutObj.bottom += off;
            }
        }
    }
        
    if ((gravity&DISPLAY_CLIP_HORIZONTAL) != 0) {
        if (inoutObj.x < display.x) inoutObj.x = display.x;
        if (inoutObj.right() > display.right()) inoutObj.width=display.right()-inoutObj.x;//inoutObj.right = display.right;
    } else {
        int off = 0;
        if (inoutObj.x < display.x) off = display.x-inoutObj.x;
        else if (inoutObj.right() > display.right()) off = display.right()-inoutObj.right();
        if (off != 0) {
            if (inoutObj.width > display.width) {
                inoutObj.x = display.x;
                inoutObj.width=display.width;//inoutObj.right = display.right;
            } else {
                //inoutObj.left += off;  inoutObj.right += off;
                inoutObj.x+=off;
            }
        }
    }
}

void Gravity::applyDisplay(int gravity,const RECT& display,RECT& inoutObj, int layoutDirection){
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
