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
#ifndef __CDROID_VIEW_H__
#define __CDROID_VIEW_H__
#include <memory>
#include <vector>
#include <functional>
#include <gui_features.h>
#include <core/inputdevice.h>
#include <core/canvas.h>
#include <core/insets.h>
#include <core/systemclock.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <core/intent.h>
#include <core/display.h>
#include <core/parcel.h>
#include <core/parcelable.h>
#include <core/predicate.h>
#include <cairomm/pattern.h>
#include <view/dragevent.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <view/abssavedstate.h>
#include <menu/menu.h>
#include <menu/contextmenu.h>
#include <view/gravity.h>
#include <view/pointericon.h>
#include <view/rendernode.h>
#include <view/layoutparams.h>
#include <view/windowinsets.h>
#include <view/touchdelegate.h>
#include <view/velocitytracker.h>
#include <view/layoutinflater.h>
#include <view/configuration.h>
#include <view/viewpropertyanimator.h>
#include <view/viewconfiguration.h>
#include <view/viewtreeobserver.h>
#include <view/soundeffectconstants.h>
#include <view/hapticfeedbackconstants.h>
#include <view/scrollfeedbackprovider.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilitynodeprovider.h>
#include <view/inputeventconsistencyverifier.h>
#include <view/viewoutlineprovider.h>
#include <animation/animation.h>
#include <animation/statelistanimator.h>
#include <animation/animatorinflater.h>
#include <core/rect.h>
#include <drawables.h>
#include <cdlog.h>

#define DECLARE_UIEVENT(type, name, ...) using name = std::function<type(__VA_ARGS__)>

namespace cdroid{
class DragEvent;
class ViewGroup;
class ViewOverlay;
class Window;
class FocusFinder;
class UIEventSource;
class HandlerActionQueue;
class LayoutInflater;
class ScrollBarDrawable;
class HapticScrollFeedbackProvider;
class View:public Drawable::Callback,public KeyEvent::Callback{
private:
    static constexpr int POPULATING_ACCESSIBILITY_EVENT_TYPES=
        AccessibilityEvent::TYPE_VIEW_CLICKED
        | AccessibilityEvent::TYPE_VIEW_LONG_CLICKED
        | AccessibilityEvent::TYPE_VIEW_SELECTED
        | AccessibilityEvent::TYPE_VIEW_FOCUSED
        | AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED
        | AccessibilityEvent::TYPE_VIEW_HOVER_ENTER
        | AccessibilityEvent::TYPE_VIEW_HOVER_EXIT
        | AccessibilityEvent::TYPE_VIEW_TEXT_CHANGED
        | AccessibilityEvent::TYPE_VIEW_TEXT_SELECTION_CHANGED
        | AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED
        | AccessibilityEvent::TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY;
    
    static constexpr int FOCUSABLE_MASK  = 0x00000011;
    static constexpr int FITS_SYSTEM_WINDOWS = 0x00000002;

    static constexpr int PROVIDER_BACKGROUND = 0;
    static constexpr int PROVIDER_NONE = 1;
    static constexpr int PROVIDER_BOUNDS = 2;
    static constexpr int PROVIDER_PADDED_BOUNDS = 3;
protected:
    static constexpr int VISIBILITY_MASK = 0x0000000C;
    static constexpr int ENABLED_MASK    = 0x00000020;
    static constexpr int FILTER_TOUCHES_WHEN_OBSCURED = 0x400;//CLIPCHILDREN = 0x400 ,TRANSPARENT  = 0x800 ,
    static constexpr int OPTIONAL_FITS_SYSTEM_WINDOWS = 0x800;
    static constexpr int FADING_EDGE_NONE = 0x00000000;
    static constexpr int FADING_EDGE_HORIZONTAL = 0x00001000;
    static constexpr int FADING_EDGE_VERTICAL = 0x00002000;
    static constexpr int FADING_EDGE_MASK = 0x00003000;
        
    static constexpr int CLICKABLE = 0x00004000;
    static constexpr int DRAWING_CACHE_ENABLED = 0x00008000;

    static constexpr int SAVE_DISABLED = 0x000010000;
    static constexpr int SAVE_DISABLED_MASK = 0x000010000;

    static constexpr int WILL_NOT_CACHE_DRAWING = 0x000020000;
    static constexpr int FOCUSABLE_IN_TOUCH_MODE= 0x000040000;

    static constexpr int TOOLTIP = 0x40000000;

    /*PFLAGS in mPrivateFlags*/
    static constexpr int PFLAG_WANTS_FOCUS      = 0x00000001;
    static constexpr int PFLAG_FOCUSED          = 0x00000002;
    static constexpr int PFLAG_SELECTED         = 0x00000004;
    static constexpr int PFLAG_IS_ROOT_NAMESPACE= 0x00000008;
    static constexpr int PFLAG_HAS_BOUNDS       = 0x00000010;
    static constexpr int PFLAG_DRAWN            = 0x00000020;
    static constexpr int PFLAG_DRAW_ANIMATION   = 0x00000040;
    static constexpr int PFLAG_SKIP_DRAW        = 0x00000080;
    static constexpr int PFLAG_REQUEST_TRANSPARENT_REGIONS=0x200;
    static constexpr int PFLAG_DRAWABLE_STATE_DIRTY  =0x400;
    static constexpr int PFLAG_MEASURED_DIMENSION_SET=0x800;
    static constexpr int PFLAG_FORCE_LAYOUT     =0x00001000;
    static constexpr int PFLAG_LAYOUT_REQUIRED  =0x00002000;

    static constexpr int PFLAG_PRESSED          = 0x00004000;
    static constexpr int PFLAG_DRAWING_CACHE_VALID= 0x00008000;
    static constexpr int PFLAG_ANIMATION_STARTED= 0x00010000;
    static constexpr int PFLAG_SAVE_STATE_CALLED= 0x00020000;
    static constexpr int PFLAG_ALPHA_SET        = 0x00040000;
    static constexpr int PFLAG_SCROLL_CONTAINER = 0x00080000;
    static constexpr int PFLAG_SCROLL_CONTAINER_ADDED= 0x00100000;
    static constexpr int PFLAG_DIRTY            = 0x00200000;
    static constexpr int PFLAG_DIRTY_OPAQUE     = 0x00400000;
    static constexpr int PFLAG_DIRTY_MASK       = 0x00600000;
    static constexpr int PFLAG_OPAQUE_BACKGROUND= 0x00800000;
    static constexpr int PFLAG_OPAQUE_SCROLLBARS= 0x01000000;
    static constexpr int PFLAG_OPAQUE_MASK      = 0x01800000;
    static constexpr int PFLAG_PREPRESSED       = 0x02000000;
    static constexpr int PFLAG_CANCEL_NEXT_UP_EVENT=0x04000000;
    static constexpr int PFLAG_HOVERED    = 0x10000000;
    static constexpr int PFLAG_ACTIVATED  = 0x40000000;
    static constexpr int PFLAG_INVALIDATED= 0x80000000;

    /*PFLAGS2*/
    static constexpr int PFLAG2_DRAG_CAN_ACCEPT = 0x00000001;
    static constexpr int PFLAG2_DRAG_HOVERED    = 0x00000002;
    static constexpr int PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT   = 0x02;
    static constexpr int PFLAG2_LAYOUT_DIRECTION_MASK         = 0x00000003 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    static constexpr int PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL = 4 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    static constexpr int PFLAG2_LAYOUT_DIRECTION_RESOLVED     = 8 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    static constexpr int PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK= 0x0000000C<< PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
        
    static constexpr int PFLAG2_TEXT_DIRECTION_MASK_SHIFT=6;
    static constexpr int PFLAG2_TEXT_DIRECTION_MASK      = 0x00000007<< PFLAG2_TEXT_DIRECTION_MASK_SHIFT;
    static constexpr int PFLAG2_TEXT_DIRECTION_RESOLVED  = 0x00000008 << PFLAG2_TEXT_DIRECTION_MASK_SHIFT;
    static constexpr int PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT=10;
    static constexpr int PFLAG2_TEXT_DIRECTION_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT;
    static constexpr int PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT = /*TEXT_DIRECTION_RESOLVED_DEFAULT*/1 << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT;
        
    static constexpr int PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT = 13;
    static constexpr int PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT = 17;
    static constexpr int PFLAG2_TEXT_ALIGNMENT_MASK          = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT;
    static constexpr int PFLAG2_TEXT_ALIGNMENT_RESOLVED      = 0x00000008 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT;
    static constexpr int PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT;
    static constexpr int PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT = /*TEXT_ALIGNMENT_RESOLVED_DEFAULT*/1 << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT;
    
    static constexpr int PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_SHIFT =20;
    static constexpr int PFLAG2_ACCESSIBILITY_LIVE_REGION_SHIFT = 23;
        
    static constexpr int PFLAG2_ACCESSIBILITY_FOCUSED   = 0x04000000;
    static constexpr int PFLAG2_SUBTREE_ACCESSIBILITY_STATE_CHANGED =0x8000000;
    static constexpr int PFLAG2_VIEW_QUICK_REJECTED     = 0x10000000;
    static constexpr int PFLAG2_PADDING_RESOLVED        = 0x20000000;
    static constexpr int PFLAG2_DRAWABLE_RESOLVED       = 0x40000000;
    static constexpr int PFLAG2_HAS_TRANSIENT_STATE     = 0x80000000;
        
    //FLAGS in mPrivateFlags3
    static constexpr int PFLAG3_VIEW_IS_ANIMATING_TRANSFORM = 0x0001;
    static constexpr int PFLAG3_VIEW_IS_ANIMATING_ALPHA     = 0x0002;
    static constexpr int PFLAG3_IS_LAID_OUT                 = 0x0004;
    static constexpr int PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT= 0x0008;
    static constexpr int PFLAG3_CALLED_SUPER                = 0x0010;
    static constexpr int PFLAG3_APPLYING_INSETS             = 0x0020;
    static constexpr int PFLAG3_FITTING_SYSTEM_WINDOWS      = 0x0040;
    static constexpr int PFLAG3_NESTED_SCROLLING_ENABLED    = 0x0080;
    static constexpr int PFLAG3_SCROLL_INDICATOR_TOP        = 0x0100;
    static constexpr int PFLAG3_SCROLL_INDICATOR_BOTTOM     = 0x0200;
    static constexpr int PFLAG3_SCROLL_INDICATOR_LEFT       = 0x0400;
    static constexpr int PFLAG3_SCROLL_INDICATOR_RIGHT      = 0x0800;
    static constexpr int PFLAG3_SCROLL_INDICATOR_START      = 0x1000;
    static constexpr int PFLAG3_SCROLL_INDICATOR_END        = 0x2000;
    static constexpr int DRAG_MASK = PFLAG2_DRAG_CAN_ACCEPT | PFLAG2_DRAG_HOVERED;
        
    static constexpr int PFLAG3_ASSIST_BLOCKED          = 0x04000;
    static constexpr int PFLAG3_CLUSTER                 = 0x08000;
    static constexpr int PFLAG3_IS_AUTOFILLED           = 0x10000;
    static constexpr int PFLAG3_FINGER_DOWN             = 0x20000;
    static constexpr int PFLAG3_FOCUSED_BY_DEFAULT      = 0x40000;
    static constexpr int PFLAG3_IMPORTANT_FOR_AUTOFILL_SHIFT = 19;
    
    static constexpr int PFLAG3_OVERLAPPING_RENDERING_FORCED_VALUE = 0x800000;
    static constexpr int PFLAG3_HAS_OVERLAPPING_RENDERING_FORCED = 0x1000000;
        
    static constexpr int PFLAG3_TEMPORARY_DETACH        = 0x2000000;
    static constexpr int PFLAG3_NO_REVEAL_ON_FOCUS      = 0x4000000;
    static constexpr int PFLAG3_NOTIFY_AUTOFILL_ENTER_ON_LAYOUT = 0x8000000;
    static constexpr int PFLAG3_SCREEN_READER_FOCUSABLE = 0x10000000;
    static constexpr int PFLAG3_AGGREGATED_VISIBLE      = 0x20000000;
    static constexpr int PFLAG3_AUTOFILLID_EXPLICITLY_SET = 0x40000000;
    static constexpr int PFLAG3_ACCESSIBILITY_HEADING   = 0x80000000;

    /** Indicates if rotary scroll haptics support for the view has been determined. */
    static constexpr int PFLAG4_ROTARY_HAPTICS_DETERMINED = 0x100000;
    static constexpr int PFLAG4_ROTARY_HAPTICS_ENABLED = 0x200000;
    static constexpr int PFLAG4_ROTARY_HAPTICS_SCROLL_SINCE_LAST_ROTARY_INPUT = 0x400000;
    static constexpr int PFLAG4_ROTARY_HAPTICS_WAITING_FOR_SCROLL_EVENT = 0x800000;
    static constexpr int PFLAG4_CONTENT_SENSITIVITY_SHIFT = 24;
public:
    static bool VIEW_DEBUG;
    static int mViewCount;
    class MeasureSpec;
    class DragShadowBuilder;
    class BaseSavedState;
    static constexpr int DEBUG_CORNERS_COLOR    = 0xFF3f7fff;
    static constexpr int DEBUG_CORNERS_SIZE_DIP = 8;
    static constexpr int NO_ID =-1;
    static constexpr int LAST_APP_AUTOFILL_ID = INT_MAX/2;

    /* ResolvedLayoutDir*/
    static constexpr int LAYOUT_DIRECTION_UNDEFINED = LayoutDirection::UNDEFINED;
    static constexpr int LAYOUT_DIRECTION_LTR = LayoutDirection::LTR;
    static constexpr int LAYOUT_DIRECTION_RTL = LayoutDirection::RTL;
    static constexpr int LAYOUT_DIRECTION_INHERIT = LayoutDirection::INHERIT;
    static constexpr int LAYOUT_DIRECTION_LOCALE = LayoutDirection::LOCAL;
    static constexpr int LAYOUT_DIRECTION_DEFAULT = LAYOUT_DIRECTION_INHERIT;
    static constexpr int LAYOUT_DIRECTION_RESOLVED_DEFAULT = LAYOUT_DIRECTION_LTR;

    /*Focusable*/
    static constexpr int NOT_FOCUSABLE = 0x00;
    static constexpr int FOCUSABLE = 0x01;
    static constexpr int FOCUSABLE_AUTO = 0x10;

    /*Visibility*/
    static constexpr int VISIBLE   = 0x00000000;
    static constexpr int INVISIBLE = 0x00000004;
    static constexpr int GONE      = 0x00000008;

    /*AutofillTye*/
    static constexpr int AUTOFILL_TYPE_NONE = 0;
    static constexpr int AUTOFILL_TYPE_TEXT = 1;
    static constexpr int AUTOFILL_TYPE_TOGGLE = 2;
    static constexpr int AUTOFILL_TYPE_LIST = 3;
    static constexpr int AUTOFILL_TYPE_DATE = 4;

    static constexpr int ENABLED  = 0x00000000;
    static constexpr int DISABLED = 0x00000020;
    static constexpr int WILL_NOT_DRAW = 0x00000080;
    static constexpr int DRAW_MASK = 0x00000080;

    static constexpr int SCROLLBARS_NONE = 0x00000000;
    static constexpr int SCROLLBARS_HORIZONTAL = 0x00000100;
    static constexpr int SCROLLBARS_VERTICAL = 0x00000200;
    static constexpr int SCROLLBARS_MASK = 0x00000300;

    static constexpr int IMPORTANT_FOR_AUTOFILL_AUTO= 0x0;
    static constexpr int IMPORTANT_FOR_AUTOFILL_YES = 0x1;
    static constexpr int IMPORTANT_FOR_AUTOFILL_NO  = 0x2;
    static constexpr int IMPORTANT_FOR_AUTOFILL_YES_EXCLUDE_DESCENDANTS= 0x4;
    static constexpr int IMPORTANT_FOR_AUTOFILL_NO_EXCLUDE_DESCENDANTS = 0x8;

    /*TextAlignment*/
    static constexpr int TEXT_ALIGNMENT_INHERIT   = 0;
    static constexpr int TEXT_ALIGNMENT_GRAVITY   = 1;
    static constexpr int TEXT_ALIGNMENT_TEXT_START= 2;
    static constexpr int TEXT_ALIGNMENT_TEXT_END  = 3;
    static constexpr int TEXT_ALIGNMENT_CENTER    = 4;
    static constexpr int TEXT_ALIGNMENT_VIEW_START= 5;
    static constexpr int TEXT_ALIGNMENT_VIEW_END  = 6;
    static constexpr int TEXT_ALIGNMENT_DEFAULT   = TEXT_ALIGNMENT_GRAVITY;
    static constexpr int TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_GRAVITY;

    /*TextDirection*/
    static constexpr int TEXT_DIRECTION_INHERIT = 0;
    static constexpr int TEXT_DIRECTION_FIRST_STRONG=1;
    static constexpr int TEXT_DIRECTION_ANY_RTL = 2;
    static constexpr int TEXT_DIRECTION_LTR     = 3;
    static constexpr int TEXT_DIRECTION_RTL     = 4;
    static constexpr int TEXT_DIRECTION_LOCALE  = 5;
    static constexpr int TEXT_DIRECTION_FIRST_STRONG_LTR = 6;
    static constexpr int TEXT_DIRECTION_FIRST_STRONG_RTL = 7;
    static constexpr int TEXT_DIRECTION_DEFAULT = TEXT_DIRECTION_INHERIT;
    static constexpr int TEXT_DIRECTION_RESOLVED_DEFAULT = TEXT_DIRECTION_FIRST_STRONG;

    static constexpr int DRAWING_CACHE_QUALITY_LOW  = 0x00080000;
    static constexpr int DRAWING_CACHE_QUALITY_HIGH = 0x00100000;
    static constexpr int DRAWING_CACHE_QUALITY_AUTO = 0x00000000;
    static constexpr int DRAWING_CACHE_QUALITY_MASK = 0x00180000;
        
    static constexpr int LONG_CLICKABLE = 0x200000;
    static constexpr int DUPLICATE_PARENT_STATE = 0x10000;
    static constexpr int CONTEXT_CLICKABLE = 0x20000;
        
    /*The scrollbar style to display the scrollbars at the edge of the view,
     * increasing the padding of the view. The scrollbars will only overlap the
     * background, if any*/
    static constexpr int SCROLLBARS_INSIDE_OVERLAY = 0x00000000;
    static constexpr int SCROLLBARS_INSIDE_INSET   = 0x01000000;
    static constexpr int SCROLLBARS_OUTSIDE_OVERLAY= 0x02000000;
    static constexpr int SCROLLBARS_OUTSIDE_INSET  = 0x03000000;
    static constexpr int SCROLLBARS_INSET_MASK     = 0x01000000;
    static constexpr int SCROLLBARS_OUTSIDE_MASK   = 0x02000000;
    static constexpr int SCROLLBARS_STYLE_MASK     = 0x03000000;
    /**
     * View flag indicating that the screen should remain on while the
     * window containing this view is visible to the user.  This effectively
     * takes care of automatically setting the WindowManager's
     * {@link WindowManager.LayoutParams#FLAG_KEEP_SCREEN_ON}.
     */
    static constexpr int KEEP_SCREEN_ON = 0x04000000;
    static constexpr int SOUND_EFFECTS_ENABLED  = 0x08000000;
    static constexpr int HAPTIC_FEEDBACK_ENABLED= 0x10000000;
    static constexpr int PARENT_SAVE_DISABLED   = 0x20000000;
    static constexpr int PARENT_SAVE_DISABLED_MASK = 0x20000000;
    /*FocusableMode*/
    static constexpr int FOCUSABLES_ALL = 0x00000000;
    static constexpr int FOCUSABLES_TOUCH_MODE = 0x00000001;

    /*Indicates that the input type for the gesture is from a user touching the screen.*/
    static constexpr int TYPE_TOUCH = 0;
    static constexpr int TYPE_NON_TOUCH = 1;

    //FocusDirection*/
    static constexpr int FOCUS_BACKWARD = 0x01;
    static constexpr int FOCUS_FORWARD  = 0x02;
    static constexpr int FOCUS_LEFT = 0x11;
    static constexpr int FOCUS_UP   = 0x21;
    static constexpr int FOCUS_RIGHT= 0x42;
    static constexpr int FOCUS_DOWN = 0x82;
        
    static constexpr int MEASURED_SIZE_MASK  = 0x00ffffff;
    static constexpr int MEASURED_STATE_MASK = 0xff000000;
    static constexpr int MEASURED_STATE_TOO_SMALL = 0x1000000;
    static constexpr int MEASURED_HEIGHT_STATE_SHIFT = 16;

    // Accessiblity constants for mPrivateFlags2
    static constexpr int IMPORTANT_FOR_ACCESSIBILITY_AUTO = 0x00000000;
    static constexpr int IMPORTANT_FOR_ACCESSIBILITY_YES  = 0x00000001;
    static constexpr int IMPORTANT_FOR_ACCESSIBILITY_NO   = 0x00000002;
    static constexpr int IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS = 0x00000004;
    static constexpr int IMPORTANT_FOR_ACCESSIBILITY_DEFAULT = IMPORTANT_FOR_ACCESSIBILITY_AUTO;
    
    static constexpr int ACCESSIBILITY_LIVE_REGION_NONE = 0x00000000;
    static constexpr int ACCESSIBILITY_LIVE_REGION_POLITE = 0x00000001;
    static constexpr int ACCESSIBILITY_LIVE_REGION_ASSERTIVE = 0x00000002;
    static constexpr int ACCESSIBILITY_LIVE_REGION_DEFAULT = ACCESSIBILITY_LIVE_REGION_NONE;
    
    /*ScrollIndicators*/
    static constexpr int SCROLL_INDICATORS_NONE = 0x0000;
    static constexpr int SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT =8;/*protected*/
    static constexpr int SCROLL_INDICATOR_TOP = PFLAG3_SCROLL_INDICATOR_TOP >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    static constexpr int SCROLL_INDICATOR_BOTTOM = PFLAG3_SCROLL_INDICATOR_BOTTOM >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    static constexpr int SCROLL_INDICATOR_LEFT  = PFLAG3_SCROLL_INDICATOR_LEFT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    static constexpr int SCROLL_INDICATOR_RIGHT = PFLAG3_SCROLL_INDICATOR_RIGHT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    static constexpr int SCROLL_INDICATOR_START = PFLAG3_SCROLL_INDICATOR_START >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    static constexpr int SCROLL_INDICATOR_END   = PFLAG3_SCROLL_INDICATOR_END >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;

    //OverScrollMode of view
    static constexpr int OVER_SCROLL_ALWAYS = 0;
    static constexpr int OVER_SCROLL_IF_CONTENT_SCROLLS = 1;
    static constexpr int OVER_SCROLL_NEVER = 2;
        
    //SystemUI FLAGS
    static constexpr int SYSTEM_UI_FLAG_VISIBLE = 0;
    static constexpr int SYSTEM_UI_FLAG_LOW_PROFILE = 0x00000001;
    static constexpr int SYSTEM_UI_FLAG_HIDE_NAVIGATION = 0x00000002;
    static constexpr int SYSTEM_UI_FLAG_FULLSCREEN = 0x00000004;
    static constexpr int SYSTEM_UI_FLAG_LAYOUT_STABLE = 0x00000100;
    static constexpr int SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = 0x00000200;
    static constexpr int SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = 0x00000400;
    static constexpr int SYSTEM_UI_FLAG_IMMERSIVE = 0x00000800;
    static constexpr int SYSTEM_UI_FLAG_IMMERSIVE_STICKY = 0x00001000;
    static constexpr int SYSTEM_UI_FLAG_LIGHT_STATUS_BAR = 0x00002000;
    static constexpr int SYSTEM_UI_RESERVED_LEGACY1 = 0x00004000;
    static constexpr int SYSTEM_UI_RESERVED_LEGACY2 = 0x00010000;
    static constexpr int SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR = 0x00000010;
    static constexpr int STATUS_BAR_HIDDEN = SYSTEM_UI_FLAG_LOW_PROFILE;
    static constexpr int STATUS_BAR_VISIBLE = SYSTEM_UI_FLAG_VISIBLE;
    static constexpr int STATUS_BAR_DISABLE_EXPAND = 0x00010000;
    static constexpr int STATUS_BAR_DISABLE_NOTIFICATION_ICONS  = 0x00020000;
    static constexpr int STATUS_BAR_DISABLE_NOTIFICATION_ALERTS = 0x00040000;
    static constexpr int STATUS_BAR_DISABLE_NOTIFICATION_TICKER = 0x00080000;
    static constexpr int STATUS_BAR_DISABLE_SYSTEM_INFO = 0x00100000;
    static constexpr int STATUS_BAR_DISABLE_HOME  = 0x00200000;
    static constexpr int STATUS_BAR_DISABLE_BACK  = 0x00400000;
    static constexpr int STATUS_BAR_DISABLE_CLOCK = 0x00800000;
    static constexpr int STATUS_BAR_DISABLE_RECENT= 0x01000000;
    static constexpr int STATUS_BAR_DISABLE_SEARCH= 0x02000000;
    static constexpr int STATUS_BAR_TRANSIENT = 0x04000000;
    static constexpr int NAVIGATION_BAR_TRANSIENT = 0x08000000;
    static constexpr int STATUS_BAR_UNHIDE = 0x10000000;
    static constexpr int NAVIGATION_BAR_UNHIDE  = 0x20000000;
    static constexpr int STATUS_BAR_TRANSLUCENT = 0x40000000;
    static constexpr int NAVIGATION_BAR_TRANSLUCENT = 0x80000000;
    static constexpr int NAVIGATION_BAR_TRANSPARENT = 0x00008000;
    static constexpr int STATUS_BAR_TRANSPARENT = 0x00000008;
    static constexpr int SYSTEM_UI_TRANSPARENT = NAVIGATION_BAR_TRANSPARENT | STATUS_BAR_TRANSPARENT;
    static constexpr int PUBLIC_STATUS_BAR_VISIBILITY_MASK = 0x00003FF7;
    static constexpr int SYSTEM_UI_CLEARABLE_FLAGS = SYSTEM_UI_FLAG_LOW_PROFILE |
              SYSTEM_UI_FLAG_HIDE_NAVIGATION | SYSTEM_UI_FLAG_FULLSCREEN;
    static constexpr int SYSTEM_UI_LAYOUT_FLAGS = SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;

    /*FindViewFlags*/
    /**
     * Find views that render the specified text.
     *
     * @see #findViewsWithText(ArrayList, CharSequence, int)
     */
    static constexpr int FIND_VIEWS_WITH_TEXT =0x00000001;
    static constexpr int FIND_VIEWS_WITH_CONTENT_DESCRIPTION = 0x00000002;
    static constexpr int FIND_VIEWS_WITH_ACCESSIBILITY_NODE_PROVIDERS = 0x00000004;
    
    /*The undefined cursor position*/
    static constexpr int ACCESSIBILITY_CURSOR_POSITION_UNDEFINED = -1;
    static constexpr int SCREEN_STATE_OFF = 0x0;
    static constexpr int SCREEN_STATE_ON  = 0x1;
        
    //Indicates no axis of view scrolling.
    static constexpr int SCROLL_AXIS_NONE = 0;
    static constexpr int SCROLL_AXIS_HORIZONTAL = 1;
    static constexpr int SCROLL_AXIS_VERTICAL = 2;

    static constexpr int DRAG_FLAG_GLOBAL = 1 << 8;
    //static constexpr int DRAG_FLAG_GLOBAL_URI_WRITE = Intent.FLAG_GRANT_WRITE_URI_PERMISSION;
    //static constexpr int DRAG_FLAG_GLOBAL_PERSISTABLE_URI_PERMISSION = Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION;
    static constexpr int DRAG_FLAG_OPAQUE = 1 << 9;
        
    //ScrollBarPosition
    static constexpr int SCROLLBAR_POSITION_DEFAULT = 0;
    static constexpr int SCROLLBAR_POSITION_LEFT = 1;
    static constexpr int SCROLLBAR_POSITION_RIGHT = 2;

    /*enum LayerType*/
    static constexpr int LAYER_TYPE_NONE    =0;
    static constexpr int LAYER_TYPE_SOFTWARE=1;
    static constexpr int LAYER_TYPE_HARDWARE=2;
protected:
    /**
     * Mask for obtaining the bits which specify how to determine
     * whether a view is important for autofill.
     */
    static constexpr int PFLAG3_IMPORTANT_FOR_AUTOFILL_MASK = (IMPORTANT_FOR_AUTOFILL_AUTO  | IMPORTANT_FOR_AUTOFILL_YES | IMPORTANT_FOR_AUTOFILL_NO
           | IMPORTANT_FOR_AUTOFILL_YES_EXCLUDE_DESCENDANTS | IMPORTANT_FOR_AUTOFILL_NO_EXCLUDE_DESCENDANTS) << PFLAG3_IMPORTANT_FOR_AUTOFILL_SHIFT;

    static constexpr int SCROLL_INDICATORS_PFLAG3_MASK = PFLAG3_SCROLL_INDICATOR_TOP | PFLAG3_SCROLL_INDICATOR_BOTTOM | PFLAG3_SCROLL_INDICATOR_LEFT
           | PFLAG3_SCROLL_INDICATOR_RIGHT | PFLAG3_SCROLL_INDICATOR_START | PFLAG3_SCROLL_INDICATOR_END;

    //PFLAG2 in mPrivateFlag2
    static constexpr int ALL_RTL_PROPERTIES_RESOLVED = PFLAG2_LAYOUT_DIRECTION_RESOLVED |  PFLAG2_TEXT_DIRECTION_RESOLVED 
            | PFLAG2_TEXT_ALIGNMENT_RESOLVED | PFLAG2_PADDING_RESOLVED | PFLAG2_DRAWABLE_RESOLVED;
    static constexpr int PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_MASK = (IMPORTANT_FOR_ACCESSIBILITY_AUTO | IMPORTANT_FOR_ACCESSIBILITY_YES
            | IMPORTANT_FOR_ACCESSIBILITY_NO | IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS) << PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_SHIFT;

    static constexpr int PFLAG2_ACCESSIBILITY_LIVE_REGION_MASK = (ACCESSIBILITY_LIVE_REGION_NONE
            | ACCESSIBILITY_LIVE_REGION_POLITE | ACCESSIBILITY_LIVE_REGION_ASSERTIVE)<< PFLAG2_ACCESSIBILITY_LIVE_REGION_SHIFT;
public:
    class AttachInfo;
    class TransformationInfo;
    class TintInfo;
    class ForegroundInfo;
    class ListenerInfo;
public:
    DECLARE_UIEVENT(bool,OnKeyListener,View& v, int keyCode, KeyEvent&);
    DECLARE_UIEVENT(bool,OnTouchListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(bool,OnHoverListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(bool,OnGenericMotionListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(bool,OnCapturedPointerListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(void,OnClickListener,View&);
    DECLARE_UIEVENT(bool,OnLongClickListener,View&);
    DECLARE_UIEVENT(bool,OnDragListener,View&,DragEvent&);
    DECLARE_UIEVENT(bool,OnContextClickListener,View&);
    DECLARE_UIEVENT(void,OnCreateContextMenuListener,ContextMenu&,View&,ContextMenuInfo*);
    DECLARE_UIEVENT(void,OnFocusChangeListener,View&,bool);
    DECLARE_UIEVENT(void,OnScrollChangeListener,View& v, int, int, int, int);
    DECLARE_UIEVENT(void,OnSystemUiVisibilityChangeListener,int);
    DECLARE_UIEVENT(WindowInsets, OnApplyWindowInsetsListener,View&,WindowInsets&);
    typedef CallbackBase<void,View&,int,int,int,int,int,int,int,int>OnLayoutChangeListener;
    typedef CallbackBase<bool,View&,KeyEvent&>OnUnhandledKeyEventListener;
    typedef struct{
        CallbackBase<void,View&>onViewAttachedToWindow;
        CallbackBase<void,View&>onViewDetachedFromWindow;
    }OnAttachStateChangeListener;
    class AccessibilityDelegate;
private:
    friend ViewGroup;
    friend Window;
    friend FocusFinder;
    friend LayoutInflater;
    friend ViewPropertyAnimator;
    class TooltipInfo;
    class CheckForTap;
    class CheckForLongPress;
    class ScrollabilityCache;
    class SendViewScrolledAccessibilityEvent;
    int mMinWidth;
    int mMinHeight;
    int mDrawingCacheBackgroundColor;
    int mOldWidthMeasureSpec;
    int mOldHeightMeasureSpec;
    int mVerticalScrollbarPosition;
    float mLongClickX ,mLongClickY;
    int mNextFocusLeftId;
    int mNextFocusRightId;
    int mNextFocusUpId;
    int mNextFocusDownId;
    int mNextFocusForwardId;
    int mNextClusterForwardId;
    int mTouchSlop;
    Insets mLayoutInsets;
 
    bool mInContextButtonPress;
    bool mHasPerformedLongPress;
    bool mIgnoreNextUpEvent;
	
    bool mBackgroundSizeChanged;
    bool mDefaultFocusHighlightSizeChanged;
    bool mDefaultFocusHighlightEnabled;
    bool mBoundsChangedmDefaultFocusHighlightSizeChanged;

    ViewOverlay* mOverlay;
    HandlerActionQueue*mRunQueue;
    PointerIcon* mPointerIcon;
    InputEventConsistencyVerifier* mInputEventConsistencyVerifier;
    ViewTreeObserver* mFloatingTreeObserver;
    StateListAnimator* mStateListAnimator;
    TouchDelegate*mTouchDelegate;
    ViewPropertyAnimator* mAnimator;
    ViewGroup* mNestedScrollingParent;
    std::unordered_map<uint64_t,uint64_t>mMeasureCache;
    std::string mStartActivityRequestWho;
    ScrollabilityCache*mScrollCache;

    Drawable* mBackground;
    Drawable* mDefaultFocusHighlight;
    Drawable* mDefaultFocusHighlightCache;
    Drawable* mScrollIndicatorDrawable;
	
    CheckForTap* mPendingCheckForTap;
    Runnable mPerformClick;
    SendViewScrolledAccessibilityEvent* mSendViewScrolledAccessibilityEvent;
    CheckForLongPress* mPendingCheckForLongPress;
    Runnable mUnsetPressedState;
    class RoundScrollbarRenderer* mRoundScrollbarRenderer;
    class TintInfo* mBackgroundTint;
    class ForegroundInfo* mForegroundInfo;
private:	
    View(const View&) = delete;
    View&operator=(const View&) = delete;
    //Temporary values used to hold (x,y) coordinates when delegating from the
    // two-arg performLongClick() method to the legacy no-arg version
    void setKeyedTag(int key,void* tag);
    void removeTapCallback();
    void removeLongPressCallback();
    void removePerformClickCallback();
    void removeUnsetPressCallback();
    HandlerActionQueue* getRunQueue();

    void checkForLongClick(int delayOffset,int x,int y);
    bool performClickInternal();
    bool performLongClickInternal(float x,float y);
    void setPressed(bool pressed,float x,float y);
    void resetPressedState();
    void hideTooltip();
    bool showHoverTooltip();
    void handleTooltipUp();
    bool showTooltip(int x, int y, bool fromLongClick);
    bool showLongClickTooltip(int x, int y);
    void initView();
    void drawBackground(Canvas&canvas);
    void applyBackgroundTint();
    void applyForegroundTint();
    View* findViewInsideOutShouldExist(View* root, int id)const;
    void sendAccessibilityHoverEvent(int eventType);
    bool requestFocusNoSearch(int direction,Rect*previouslyFocusedRect);
    bool hasAncestorThatBlocksDescendantFocus()const;
    void setAlphaInternal(float);
    float getFinalAlpha()const;
    void debugDrawFocus(Canvas&canvas);
    Drawable* getDefaultFocusHighlightDrawable();
    void setDefaultFocusHighlight(Drawable* highlight);
    void switchDefaultFocusHighlight();
    void drawDefaultFocusHighlight(Canvas& canvas);

    void sizeChange(int newWidth,int newHeight,int oldWidth,int oldHeight);
    void setMeasuredDimensionRaw(int measuredWidth, int measuredHeight);
    bool isPerformHapticFeedbackSuppressed(int feedbackConstant, int flags);
    int  computeHapticFeedbackPrivateFlags();
    void initializeScrollbarsInternal(const AttributeSet&attrs);
    void initializeScrollBarDrawable();
    void initScrollCache();
    ScrollabilityCache* getScrollCache();
    bool initialAwakenScrollBars();
    Drawable* getAutofilledDrawable();
    void drawAutofilledHighlight(Canvas& canvas);
    bool isOnVerticalScrollbarThumb(int x,int y);
    bool isOnHorizontalScrollbarThumb(int x,int y);
    bool isHoverable()const;
    bool hasSize()const;
    bool canTakeFocus()const;
    void getRoundVerticalScrollBarBounds(Rect* bounds);
    void getStraightVerticalScrollBarBounds(Rect*drawBounds, Rect*touchBounds=nullptr);
    void getVerticalScrollBarBounds(Rect*bounds, Rect*touchBounds=nullptr);
    void getHorizontalScrollBarBounds(Rect*drawBounds, Rect*touchBounds);
    void initializeScrollIndicatorsInternal();
    void setFocusedInCluster(View* cluster);
    void updateFocusedInCluster(View* oldFocus,int direction);
    void updatePflags3AndNotifyA11yIfChanged(int mask, bool newValue);
    void updatePreferKeepClearForFocus();
    void updatePositionUpdateListener();
    bool dispatchGenericMotionEventInternal(MotionEvent& event);
    void populateAccessibilityNodeInfoDrawingOrderInParent(AccessibilityNodeInfo& info);
    void notifySubtreeAccessibilityStateChangedByParentIfNeeded();
    static int numViewsForAccessibility(View* view);
    static float sanitizeFloatPropertyValue(float,const std::string&);
    static float sanitizeFloatPropertyValue(float,const std::string&,float,float);
    View* findLabelForView(View* view, int labeledId);
    bool applyLegacyAnimation(ViewGroup* parent, int64_t drawingTime, Animation* a, bool scalingRequired);
    bool needRtlPropertiesResolution()const;
    bool skipInvalidate()const;
    void buildDrawingCache(bool autoScale);
    void buildDrawingCacheImpl(bool autoScale);
    bool hasParentWantsFocus()const;
    void cleanupDraw();
    void invalidateInternal(int l, int t, int r, int b, bool invalidateCache,bool fullInvalidate);
    View* findAccessibilityFocusHost(bool searchDescendants);
    bool hasListenersForAccessibility() const;
    bool isAccessibilityPane()const;
    bool isAutofillable();
    void postSendViewScrolledAccessibilityEventCallback(int dx, int dy);
    void cancel(SendViewScrolledAccessibilityEvent* callback);
    void setOutlineProviderFromAttribute(int providerInt);
    void rebuildOutline();
    HapticScrollFeedbackProvider*getScrollFeedbackProvider();
    void doRotaryProgressForScrollHaptics(MotionEvent& rotaryEvent);
    void doRotaryLimitForScrollHaptics(MotionEvent& rotaryEvent);
    void processScrollEventForRotaryEncoderHaptics();
protected:
    static bool sIgnoreMeasureCache;
    static bool sAlwaysRemeasureExactly;
    static bool sPreserveMarginParamsInLayoutParamConversion;
    int mID;
    int mLayerType;
    int mAutofillViewId;
    int mAccessibilityViewId;
    int mAccessibilityCursorPosition;
    int mScrollX;
    int mScrollY;
    int mOverScrollMode;
    int mViewFlags;
    int mPrivateFlags;
    int mPrivateFlags2;
    int mPrivateFlags3;
    int mPrivateFlags4;
    int mPaddingLeft;
    int mPaddingRight;
    int mPaddingTop;
    int mPaddingBottom;
    int mMeasuredWidth;
    int mMeasuredHeight;

    int mUserPaddingLeft; //set by the user to append to the scrollbar's size.
    int mUserPaddingRight;
    int mUserPaddingTop;
    int mUserPaddingBottom;
    /* Cache the paddingStart set by the user to append to the scrollbar's size. */
    int mUserPaddingStart;
    /* Cache the paddingEnd set by the user to append to the scrollbar's size.*/
    int mUserPaddingEnd;
    /* Cache initial left padding*/
    int mUserPaddingLeftInitial;
    /* Cache initial right padding*/
    int mUserPaddingRightInitial;
    int mSystemUiVisibility;
    int mTransientStateCount;
    int mWindowAttachCount;
    int mLabelForId;
    int mAccessibilityTraversalBeforeId;
    int mAccessibilityTraversalAfterId;
    bool mLeftPaddingDefined;
    bool mRightPaddingDefined;
    bool mCachingFailed;
    bool mLastIsOpaque;
    bool mSendingHoverAccessibilityEvents;
    AccessibilityDelegate* mAccessibilityDelegate;
    Rect mClipBounds;
    std::string mHint;
    std::string mContentDescription;
    std::string mStateDescription;
    std::string mAccessibilityPaneTitle;
    Cairo::RefPtr<Cairo::ImageSurface> mDrawingCache;
    Cairo::RefPtr<Cairo::ImageSurface> mUnscaledDrawingCache;
    void * mTag;
    Context* mContext;
    LayoutParams* mLayoutParams;
    TransformationInfo* mTransformationInfo;
    SparseArray<void*>* mKeyedTags;
    Animation* mCurrentAnimation;
    std::vector<int> mDrawableState;
    ViewOutlineProvider mOutlineProvider;
    HapticScrollFeedbackProvider* mScrollFeedbackProvider;
    int mTop,mLeft,mRight,mBottom;
    ViewGroup * mParent;
    AttachInfo* mAttachInfo;
    RenderNode* mRenderNode;
    ListenerInfo* mListenerInfo;
    class TooltipInfo* mTooltipInfo;
    ListenerInfo*getListenerInfo();
protected:
    virtual void internalSetPadding(int left, int top, int right, int bottom);
    void assignParent(ViewGroup*p);
    bool debugDraw()const;
    void setBackgroundBounds();
    int dipsToPixels(int dips)const;
    void computeOpaqueFlags();
    bool hasOpaqueScrollbars()const;
    virtual void resolveDrawables();
    bool areDrawablesResolved()const;
    void setDuplicateParentStateEnabled(bool);
    bool isDuplicateParentStateEnabled()const;

    int getWindowAttachCount()const;
    void recomputePadding();
    virtual bool isPaddingOffsetRequired()const;
    virtual int getLeftPaddingOffset();
    virtual int getRightPaddingOffset();
    virtual int getTopPaddingOffset();
    virtual int getBottomPaddingOffset();
    int getFadeTop(bool offsetRequired);
    int getFadeHeight(bool offsetRequired);
    bool isHardwareAccelerated()const;
    void setClipBounds(const Rect*clipBounds);
    bool getClipBounds(Rect&outRect);

    void invalidateParentIfNeededAndWasQuickRejected();
    virtual void invalidateInheritedLayoutMode(int);
    void destroyDrawingCache();
    void resetResolvedDrawablesInternal();
    Cairo::RefPtr<Cairo::ImageSurface>getDrawingCache(bool autoScale);
    virtual bool hasWindowFocus()const;

    void setLeftTopRightBottom(int left, int top, int right, int bottom);
    virtual bool setFrame(int x,int y,int w,int h);
    virtual void resetResolvedDrawables();
    virtual bool verifyDrawable(Drawable*)const;
    virtual void drawableStateChanged();
    virtual std::vector<int> onCreateDrawableState(int);
    virtual void setFlags(int flag,int mask);
    virtual bool hasFlag(int flag) const;
    bool fitSystemWindows(Rect& insets);
    void applyInsets(const Rect& insets);
    void postUpdate(Runnable r);
    virtual void dispatchSetSelected(bool selected);
    virtual void dispatchSetPressed(bool pressed);
    virtual void dispatchVisibilityChanged(View& changedView,int visiblity);
    virtual bool dispatchVisibilityAggregated(bool isVisible);
    virtual void dispatchWindowFocusChanged(bool);
    virtual bool dispatchTooltipHoverEvent(MotionEvent& event);
    void handleTooltipKey(KeyEvent& event);
    virtual void onWindowFocusChanged(bool hasWindowFocus);
    virtual void onVisibilityChanged(View& changedView,int visibility);
    virtual void onDisplayHint(int hint/*Visibility*/);
    virtual void onAttachedToWindow();
    virtual void onDetachedFromWindow();
    virtual void onDetachedFromWindowInternal();
    virtual void  onMeasure(int widthMeasureSpec, int heightMeasureSpec);
    virtual void dispatchDraw(Canvas&);
    virtual void onFocusChanged(bool,int,Rect*);
    virtual void onFocusLost();
    void updateSystemGestureExclusionRects();
    void updateKeepClearRects();
    std::vector<Rect>collectPreferKeepClearRects();
    bool computeFitSystemWindows(Rect& inoutInsets, Rect& outLocalInsets);
    virtual void clearParentsWantFocus();
    virtual void clearFocusInternal(View* focused, bool propagate, bool refocus);
    virtual void handleFocusGainInternal(int direction,Rect*previouslyFocusedRect);
    bool awakenScrollBars();
    bool awakenScrollBars(int startDelay, bool invalidate);

    virtual void setSystemGestureExclusionRects(const std::vector<Rect>&rects);
    std::vector<Rect> getSystemGestureExclusionRects();
    void setPreferKeepClear(bool preferKeepClear);
    bool isPreferKeepClear()const;
    void setPreferKeepClearRects(const std::vector<Rect>& rects);
    std::vector<Rect> getPreferKeepClearRects()const;
    void setUnrestrictedPreferKeepClearRects(const std::vector<Rect>& rects);
    std::vector<Rect>getUnrestrictedPreferKeepClearRects()const;

    static int combineVisibility(int vis1, int vis2);
    WindowInsets* getRootWindowInsets();
    virtual void onSizeChanged(int w,int h,int oldw,int oldh);
    virtual void onScrollChanged(int l, int t, int oldl, int oldt);
    virtual void onLayout(bool ,int,int,int,int);
    virtual void onDraw(Canvas& ctx);
    virtual void onDrawForeground(Canvas& canvas);
    virtual void onFinishInflate();
    virtual void dispatchSetActivated(bool activated);
    virtual void dispatchAttachedToWindow(AttachInfo*info,int visibility);
    virtual void dispatchDetachedFromWindow();
    virtual void dispatchCancelPendingInputEvents();
    virtual void onCancelPendingInputEvents();
    bool canReceivePointerEvents()const;
    bool getFilterTouchesWhenObscured()const;
    void setFilterTouchesWhenObscured(bool enabled);
    bool onFilterTouchEventForSecurity(MotionEvent& event);
    virtual bool dispatchHoverEvent(MotionEvent&event);
    virtual bool dispatchTrackballEvent(MotionEvent& event);
    virtual bool dispatchCapturedPointerEvent(MotionEvent& event);
    virtual bool dispatchGenericPointerEvent(MotionEvent& event);
    virtual bool dispatchGenericFocusedEvent(MotionEvent& event);
    virtual bool hasHoveredChild()const;
    virtual bool pointInHoveredChild(MotionEvent& event);

    virtual void saveHierarchyState(SparseArray<Parcelable*>& container);
    virtual void dispatchSaveInstanceState(SparseArray<Parcelable*>& container);
    virtual Parcelable* onSaveInstanceState();
    virtual void restoreHierarchyState(SparseArray<Parcelable*>& container);
    virtual void dispatchRestoreInstanceState(SparseArray<Parcelable*>& container);
    virtual void onRestoreInstanceState(Parcelable& state);

    static int combineMeasuredStates(int curState, int newState);
    static std::vector<int>& mergeDrawableStates(std::vector<int>&baseState,const std::vector<int>&additionalState);
    static int resolveSize(int size, int measureSpec);
    static int resolveSizeAndState(int size, int measureSpec, int childMeasuredState);
    static int getDefaultSize(int size, int measureSpec);
    void damageInParent();
    void transformRect(Rect&rect);
    virtual bool hasDefaultFocus()const;
    virtual int getSuggestedMinimumWidth();
    virtual int getSuggestedMinimumHeight();
    void setMeasuredDimension(int measuredWidth, int measuredHeight);
    virtual ContextMenuInfo* getContextMenuInfo();
    virtual void onCreateContextMenu(ContextMenu& menu);
    virtual bool handleScrollBarDragging(MotionEvent& event);
    bool performButtonActionOnTouchDown(MotionEvent&);
    virtual bool updateLocalSystemUiVisibility(int localValue, int localChanges);

    void onAnimationStart();
    void onAnimationEnd();
    virtual bool onSetAlpha(int alpha);

    bool isVerticalScrollBarHidden()const;
    bool shouldDrawRoundScrollbar()const;

    bool isOnScrollbar(int x,int y);
    bool isOnScrollbarThumb(int x,int y);
    bool isDraggingScrollBar()const;
    bool isVisibleToUser(Rect* boundInView);
    void dispatchCollectViewAttributes(AttachInfo* attachInfo, int visibility);
    void performCollectViewAttributes(AttachInfo* attachInfo, int visibility);
    void needGlobalAttributesUpdate(bool force);
    virtual void onConfigurationChanged(Configuration& newConfig);
    virtual bool overScrollBy(int deltaX, int deltaY, int scrollX, int scrollY, int scrollRangeX,
              int  scrollRangeY, int maxOverScrollX, int maxOverScrollY, bool isTouchEvent);
    virtual void onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY);
    virtual float getTopFadingEdgeStrength();
    virtual float getBottomFadingEdgeStrength();
    virtual float getLeftFadingEdgeStrength();
    virtual float getRightFadingEdgeStrength();
    virtual void getScrollIndicatorBounds(Rect&);
    virtual void onDrawScrollIndicators(Canvas& canvas);
    virtual void onDrawScrollBars(Canvas& canvas);
    void onDrawHorizontalScrollBar(Canvas& canvas, Drawable* scrollBar,const Rect&);
    void onDrawVerticalScrollBar (Canvas& canvas , Drawable* scrollBar,const Rect&);
    virtual void resetSubtreeAccessibilityStateChanged();
    bool traverseAtGranularity(int granularity, bool forward,  bool extendSelection);
    void ensureTransformationInfo();
public:
    View(Context*ctx,const AttributeSet&attrs);
    View(int w,int h);
    virtual ~View();
    bool isShowingLayoutBounds()const;
    void setShowingLayoutBounds(bool debugLayout);
    virtual void draw(Canvas&canvas);
    bool draw(Canvas&canvas,ViewGroup*parent,int64_t drawingTime);

    virtual void invalidateParentCaches();
    virtual void invalidateParentIfNeeded();
    virtual void invalidateViewProperty(bool invalidateParent, bool forceRedraw);
    virtual void invalidate(const Rect&dirty);
    virtual void invalidate(int l,int t,int w,int h);
    virtual void invalidate(bool invalidateCache=true);

    bool isDirty()const;
    void postInvalidate();
    void postInvalidate(int left, int top, int width, int height);
    void postInvalidateDelayed(long delayMilliseconds);
    void postInvalidateDelayed(long delayMilliseconds, int left, int top,int width, int height);
    void postInvalidateOnAnimation();
    void postInvalidateOnAnimation(int left, int top, int width, int height);
    void invalidateDrawable(Drawable& who)override;
    int  getLayerType()const;
    void setLayerType(int);
    int  getDrawingCacheBackgroundColor()const;
    void setDrawingCacheBackgroundColor(int);
    void scheduleDrawable(Drawable& who,Runnable& what, int64_t when)override;
    void unscheduleDrawable(Drawable& who,Runnable& what)override;
    virtual void unscheduleDrawable(Drawable& who);

    const Rect getBound()const;
    void getHitRect(Rect&);
    bool pointInView(int localX,int localY,int slop);
    const Rect getDrawingRect()const;
    int64_t getDrawingTime()const;
    virtual void getFocusedRect(Rect&r);
    void getDrawingRect(Rect& outRect)const;
    bool getGlobalVisibleRect(Rect& r,Point*globalOffet);
    bool getLocalVisibleRect(Rect&r);

    void offsetTopAndBottom(int offset);
    void offsetLeftAndRight(int offset);
    void setLeft(int left);
    void setTop(int top);
    void setRight(int right);
    void setBottom(int bottom);
    int getLeft()const;
    int getTop()const;
    int getRight()const;
    int getBottom()const;
    int getWidth()const;
    int getHeight()const;
    int getPaddingTop();
    int getPaddingBottom();
    int getPaddingLeft();
    int getPaddingStart();
    int getPaddingRight();
    int getPaddingEnd();
    bool isPaddingRelative()const;
    Insets computeOpticalInsets();
    Insets getOpticalInsets();
    void resetPaddingToInitialValues();
    void setOpticalInsets(const Insets& insets);
    virtual void setPadding(int left, int top, int right, int bottom);
    virtual void setPaddingRelative(int start,int top,int end,int bottom);
    bool isPaddingResolved()const;
    void setTooltipText(const std::string& tooltipText);
    std::string getTooltipText()const;
    virtual void resolvePadding();
    virtual void onRtlPropertiesChanged(int layoutDirection);
    virtual bool resolveLayoutDirection();
    bool canResolveTextDirection()const;
    bool canResolveLayoutDirection()const;
    int getMinimumHeight();
    void setMinimumHeight(int minHeight);
    int getMinimumWidth();
    void setMinimumWidth(int minWidth);
    Animation* getAnimation()const;
    void startAnimation(Animation* animation);
    void clearAnimation();
    void setAnimation(Animation* animation);

    void applyDrawableToTransparentRegion(Drawable* dr,const Cairo::RefPtr<Cairo::Region>& region);
    virtual bool gatherTransparentRegion(const Cairo::RefPtr<Cairo::Region>& region);
    void setSoundEffectsEnabled(bool soundEffectsEnabled);
    bool isSoundEffectsEnabled()const;
    void playSoundEffect(int soundConstant);
    void setHapticFeedbackEnabled(bool hapticFeedbackEnabled);
    bool isHapticFeedbackEnabled()const;
    bool performHapticFeedback(int feedbackConstant, int flags=0);
    void performHapticFeedbackForInputDevice(int feedbackConstant, int inputDeviceId,int inputSource, int flags);

    void setSystemUiVisibility(int visibility);
    int getSystemUiVisibility()const;
    int getWindowSystemUiVisibility()const;
    virtual void onWindowSystemUiVisibilityChanged(int visible);
    virtual void dispatchWindowSystemUiVisiblityChanged(int visible);
    void setOnSystemUiVisibilityChangeListener(const OnSystemUiVisibilityChangeListener& l);
    virtual void dispatchSystemUiVisibilityChanged(int visibility);
    void setDisabledSystemUiVisibility(int flags);

    bool startDragAndDrop(ClipData*,DragShadowBuilder*,void*myLocalState,int flags);
    void cancelDragDrop();
    void updateDragShadow(DragShadowBuilder*);
    virtual bool onDragEvent(DragEvent&);
    virtual bool dispatchDragEvent(DragEvent&);
    virtual bool canAcceptDrag();
    bool callDragEventHandler(DragEvent& event);

    void setDrawingCacheEnabled(bool);
    bool isDrawingCacheEnabled()const;
    
    void setDefaultFocusHighlightEnabled(bool defaultFocusHighlightEnabled);
    bool getDefaultFocusHighlightEnabled()const;
    bool isLayoutDirectionResolved()const;
    int getLayoutDirection()const;
    virtual bool isOpaque()const;
    virtual void setLayoutDirection(int layoutDirection);
    bool isLayoutRtl()const;
    bool isSaveFromParentEnable()const;
    void setSaveFromParentEnabled(bool);
    bool isFocusableInTouchMode()const;
    virtual void setFocusable(int focusable);
    virtual void setFocusableInTouchMode(bool focusableInTouchMode);
    bool isScreenReaderFocusable()const;
    void setScreenReaderFocusable(bool screenReaderFocusable);
    bool isAccessibilityHeading() const;
    void setAccessibilityHeading(bool isHeading);
    virtual void drawableHotspotChanged(float x, float y);
    virtual void dispatchDrawableHotspotChanged(float x,float y);
    void refreshDrawableState();
    bool isDefaultFocusHighlightNeeded(const Drawable* background,const Drawable* foreground)const;
    virtual const std::vector<int>getDrawableState();
    bool getKeepScreenOn()const;
    void setKeepScreenOn(bool);

    int getNextFocusLeftId()const;
    void setNextFocusLeftId(int id);
    int getNextFocusRightId()const;
    void setNextFocusRightId(int id);
    int getNextFocusUpId()const;
    void setNextFocusUpId(int id);
    int getNextFocusDownId()const;
    void setNextFocusDownId(int id);
    int getNextFocusForwardId()const;
    void setNextFocusForwardId(int id);
    int getNextClusterForwardId()const;
    void setNextClusterForwardId(int);

    int getScrollBarSize()const;
    void setScrollBarSize(int scrollBarSize);
    virtual void setScrollBarStyle(int style);
    int  getScrollBarStyle()const;
    bool isHorizontalScrollBarEnabled()const;
    void setHorizontalScrollBarEnabled(bool);
    bool isVerticalScrollBarEnabled()const;
    void setVerticalScrollBarEnabled(bool);
    virtual int getHorizontalScrollbarHeight()const;
    virtual int getVerticalScrollbarWidth()const;
    virtual int getVerticalScrollbarPosition()const;
    bool isScrollContainer()const;
    void setScrollContainer(bool isScrollContainer);
    bool isHorizontalFadingEdgeEnabled()const;
    void setHorizontalFadingEdgeEnabled(bool horizontalFadingEdgeEnabled);
    bool isVerticalFadingEdgeEnabled()const;
    void setVerticalFadingEdgeEnabled(bool verticalFadingEdgeEnabled);

    virtual void setVerticalScrollbarPosition(int position);
    void setVerticalScrollbarThumbDrawable(Drawable* drawable);
    void setVerticalScrollbarTrackDrawable(Drawable* drawable);
    void setHorizontalScrollbarThumbDrawable(Drawable* drawable);
    void setHorizontalScrollbarTrackDrawable(Drawable* drawable);
    Drawable* getVerticalScrollbarThumbDrawable()const;
    Drawable* getVerticalScrollbarTrackDrawable()const;
    Drawable* getHorizontalScrollbarThumbDrawable()const;
    Drawable* getHorizontalScrollbarTrackDrawable()const;
    void setScrollbarFadingEnabled(bool fadeScrollbars);
    bool isScrollbarFadingEnabled()const;
    int  getScrollBarDefaultDelayBeforeFade()const;
    void setScrollBarDefaultDelayBeforeFade(int scrollBarDefaultDelayBeforeFade);
    int  getScrollBarFadeDuration()const;
    void setScrollBarFadeDuration(int scrollBarFadeDuration);

    int getScrollIndicators()const;
    virtual void setScrollIndicators(int indicators,int mask=SCROLL_INDICATORS_PFLAG3_MASK >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT);
    virtual void computeScroll();
    virtual int  computeHorizontalScrollRange();
    virtual int  computeHorizontalScrollOffset();
    virtual int  computeHorizontalScrollExtent();
    virtual int  computeVerticalScrollRange();
    virtual int  computeVerticalScrollOffset();
    virtual int  computeVerticalScrollExtent();
    virtual bool canScrollHorizontally(int direction);
    virtual bool canScrollVertically(int direction);

    void  setRevealOnFocusHint(bool revealOnFocus);
    bool  getRevealOnFocusHint()const;
    virtual bool isAttachedToWindow()const;
    bool  isLaidOut()const;
    bool  willNotDraw()const;
    void  setWillNotDraw(bool willNotDraw);
    const Rect getClientRect()const;
    bool  hasOnClickListener()const;
    virtual void setOnClickListener(const OnClickListener& l);
    virtual void setOnLongClickListener(const OnLongClickListener& l);
    void setOnContextClickListener(const OnContextClickListener& l);
    void setOnCreateContextMenuListener(const OnCreateContextMenuListener& l);
    virtual void setOnFocusChangeListener(const OnFocusChangeListener& listtener); 
    virtual void setOnScrollChangeListener(const OnScrollChangeListener& l);
    void  addOnLayoutChangeListener(const OnLayoutChangeListener& listener);
    void  removeOnLayoutChangeListener(const OnLayoutChangeListener& listener);
    void  addOnAttachStateChangeListener(const OnAttachStateChangeListener& listener);
    void  removeOnAttachStateChangeListener(const OnAttachStateChangeListener& listener);
    virtual bool performClick();
    virtual bool performLongClick();
    virtual bool performLongClick(float x,float y);
    bool callOnClick();
    void getHotspotBounds(Rect& outRect);
    void getBoundsOnScreen(Rect& outRect, bool clipToParent=false);
    void cancelPendingInputEvents();
    void cancelLongPress();
    void setTouchDelegate(TouchDelegate* delegate);
    TouchDelegate* getTouchDelegate()const;
    bool  performContextClick(float x, float y);
    bool  performContextClick();
    virtual bool showContextMenu();
    virtual bool showContextMenu(float x, float y);
    void startActivityForResult(Intent intent, int requestCode);
    virtual bool dispatchActivityResult(const std::string& who, int requestCode, int resultCode, Intent data);
    virtual void onActivityResult(int requestCode, int resultCode, Intent data);
    void setOnKeyListener(const OnKeyListener& l);
    void setOnTouchListener(const OnTouchListener& l);
    void setOnGenericMotionListener(const OnGenericMotionListener& l);
    void setOnHoverListener(const OnHoverListener& l);
    void setOnDragListener(const OnDragListener& l);
    // Foreground color

    //foreground/background
    Drawable* getForeground()const;
    void setForeground(Drawable* foreground);
    bool isForegroundInsidePadding()const;
    int getForegroundGravity()const;
    void setForegroundGravity(int gravity);
    void setForegroundTintList(const ColorStateList* tint);
    void setForegroundTintMode(int tintMode);
    void setForegroundTintBlendMode(int blendMode);
    const ColorStateList* getForegroundTintList();
    virtual void onResolveDrawables(int layoutDirection);

    virtual void jumpDrawablesToCurrentState();
    Drawable*getBackground()const;
    virtual void setBackground(Drawable*background);
    void setBackgroundColor(int color);
    void setBackgroundResource(const std::string&resid);
    void setBackgroundTintList(const ColorStateList* tint);
    void setBackgroundTintMode(int tintMode);
    int getBackgroundTintMode() const;
    const ColorStateList* getBackgroundTintList()const;
    virtual int getSolidColor()const;

    AccessibilityDelegate* getAccessibilityDelegate()const;
    void setAccessibilityDelegate(AccessibilityDelegate* delegate);
    virtual AccessibilityNodeProvider* getAccessibilityNodeProvider();
    bool isActionableForAccessibility()const;
    void notifyViewAccessibilityStateChangedIfNeeded(int changeType);
    virtual void notifySubtreeAccessibilityStateChangedIfNeeded();
    bool dispatchNestedPrePerformAccessibilityAction(int action, Bundle* arguments);
    virtual bool performAccessibilityAction(int action, Bundle* arguments);
    virtual bool performAccessibilityActionInternal(int action, Bundle* arguments);
    std::string getIterableTextForAccessibility();
    bool isAccessibilitySelectionExtendable()const;
    int getAccessibilitySelectionStart()const;
    int getAccessibilitySelectionEnd()const;
    void setAccessibilitySelection(int start, int end);
    void setTransitionVisibility(int visibility);

    bool isTemporarilyDetached()const;
    virtual void dispatchStartTemporaryDetach();
    virtual void dispatchFinishTemporaryDetach();
    virtual void onFinishTemporaryDetach();
    virtual void onStartTemporaryDetach();
    virtual bool hasTransientState();
    void setHasTransientState(bool hasTransientState);

    static int generateViewId();
    static bool isViewIdGenerated(int id);
    void setId(int id);
    int  getId()const;
    void setLabelFor(int);
    int  getLabelFor()const;
    int  getAccessibilityViewId();
    int getAutofillViewId();
    int getAccessibilityWindowId()const;
    void setAccessibilityTraversalBefore(int beforeId);
    int getAccessibilityTraversalBefore()const;
    void setAccessibilityTraversalAfter(int afterId);
    int getAccessibilityTraversalAfter()const;
    int  getAutoFillViewId();
    void setTag(void*);
    void*getTag()const;
    void setTag(int key,void*tag);
    void*getTag(int key)const;
    void setTagInternal(int key, void* tag);
    virtual void setHint(const std::string&hint);
    const std::string&getHint()const;
    void setContentDescription(const std::string&);
    virtual std::string getContentDescription()const;
    void setStateDescription(const std::string& stateDescription);
    void setIsRootNamespace(bool);
    bool isRootNamespace()const;
    cdroid::Context*getContext()const;
    virtual void scrollTo(int x,int y);
    virtual void scrollBy(int dx,int dy);
    void setScrollX(int x);
    void setScrollY(int y);
    int getScrollX()const;
    int getScrollY()const;
    int getOverScrollMode()const;
    virtual void setOverScrollMode(int overScrollMode);
    int getVerticalFadingEdgeLength()const;
    int getHorizontalFadingEdgeLength()const;
    void setFadingEdgeLength(int length);
    int getFadingEdgeLength() const;
    void transformFromViewToWindowSpace(int*);
    void mapRectFromViewToScreenCoords(RectF& rect, bool clipToParent);
    void getLocationOnScreen(int*);
    void getLocationInWindow(int*);
    bool startNestedScroll(int axes);
    void stopNestedScroll();
    void setNestedScrollingEnabled(bool benabled);
    bool isNestedScrollingEnabled()const;
    bool hasNestedScrollingParent()const;
    virtual bool dispatchNestedScroll(int dxConsumed, int dyConsumed,
            int dxUnconsumed, int dyUnconsumed,int* offsetInWindow);
    virtual bool dispatchNestedPreScroll(int dx, int dy,int* consumed,int* offsetInWindow);
    virtual bool dispatchNestedFling(float velocityX, float velocityY, bool consumed);
    virtual bool dispatchNestedPreFling(float velocityX, float velocityY);
    int  getRawTextDirection()const;
    void setTextDirection(int textDirection);
    int  getTextDirection()const;
    bool canResolveTextAlignment()const;
    virtual void resetResolvedTextAlignment();
    bool isTextAlignmentInherited()const;
    bool isTextAlignmentResolved()const;
    virtual bool resolveTextAlignment();
    int  getRawTextAlignment()const;
    void setTextAlignment(int textAlignment);
    int  getTextAlignment()const;

    //Pointer
    virtual bool hasPointerCapture()const;
    void requestPointerCapture();
    void releasePointerCapture();
    virtual void onPointerCaptureChange(bool hasCapture);
    virtual void dispatchPointerCaptureChanged(bool hasCapture);
    virtual bool onCapturedPointerEvent(MotionEvent& event);
    void setOnCapturedPointerListener(const OnCapturedPointerListener& l);
    virtual PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex);
    void setPointerIcon(PointerIcon* pointerIcon);
    PointerIcon*getPointerIcon();
    int getImportantForAutofill()const;
    void setImportantForAutofill(int mode);
    bool isImportantForAutofill()const;
    bool canNotifyAutofillEnterExitEvent();

    // Attribute
    virtual void clearFlag(int flag);
    bool isAccessibilityFocused()const;
    void notifyEnterOrExitForAutoFillIfNeeded(bool enter);
    void setAccessibilityPaneTitle(const std::string& accessibilityPaneTitle);
    std::string getAccessibilityPaneTitle() const;
    virtual std::string getAccessibilityClassName() const;
    virtual void sendAccessibilityEvent(int eventType);
    virtual void sendAccessibilityEventInternal(int eventType);
    virtual void sendAccessibilityEventUnchecked(AccessibilityEvent& event);
    virtual void sendAccessibilityEventUncheckedInternal(AccessibilityEvent& event);
    virtual bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event);
    virtual bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event);
    virtual void onPopulateAccessibilityEvent(AccessibilityEvent& event);
    virtual void onPopulateAccessibilityEventInternal(AccessibilityEvent& event);
    virtual void onInitializeAccessibilityEvent(AccessibilityEvent& event);
    virtual void onInitializeAccessibilityEventInternal(AccessibilityEvent& event);
    virtual AccessibilityNodeInfo* createAccessibilityNodeInfo();
    AccessibilityNodeInfo* createAccessibilityNodeInfoInternal();
    virtual void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info);
    virtual void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info);
    void addExtraDataToAccessibilityNodeInfo(AccessibilityNodeInfo& info,const std::string& extraDataKey,Bundle* arguments);
    bool isVisibleToUserForAutofill(int virtualId)const;
    bool isVisibleToUser();
    bool requestAccessibilityFocus();
    void clearAccessibilityFocus();
    void clearAccessibilityFocusNoCallbacks(int action);
    bool isAccessibilityFocusedViewOrHost();
    virtual bool isFocused()const;
    virtual bool isInEditMode()const;
    bool isFocusedByDefault()const;
    void setFocusedByDefault(bool isFocusedByDefault);
    // Enable & Visible
    virtual void setVisibility(int visable);
    virtual int getVisibility() const;
    bool isAggregatedVisible()const;
    int  getWindowVisibility()const;
    void getWindowVisibleDisplayFrame(Rect& outRect);
    void getWindowDisplayFrame(Rect& outRect);
    virtual void dispatchConfigurationChanged(Configuration& newConfig);
    bool isShown()const;
    virtual WindowInsets onApplyWindowInsets(WindowInsets& insets);
    void setOnApplyWindowInsetsListener(const OnApplyWindowInsetsListener& listener);
    WindowInsets dispatchApplyWindowInsets(WindowInsets& insets);
    WindowInsets computeSystemWindowInsets(WindowInsets& in, Rect& outLocalInsets);
    void setFitsSystemWindows(bool fitSystemWindows);
    bool getFitsSystemWindows()const;
    bool fitsSystemWindows();
    void requestFitSystemWindows();
    void requestApplyInsets();
    void makeOptionalFitsSystemWindows();
    virtual void setEnabled(bool enable);
    virtual bool isEnabled() const;
    virtual void setSelected(bool);
    bool isSelected()const;
    bool fitSystemWindowsInt(Rect& insets);
    void setPressed(bool);
    bool isPressed()const;
    void setActivated(bool activated);
    bool isActivated()const;
    bool isHovered()const;
    void setHovered(bool hovered);
    bool isAutofilled()const;
    void setAutofilled(bool);
    //void setAutofillId(AutoillId);
    int getAutofillType()const;

    bool isClickable()const;
    void setClickable(bool clickable);
    bool isLongClickable()const;
    void setLongClickable(bool longClickable);
    bool isContextClickable()const;
    void setContextClickable(bool contextClickable);

    bool isInScrollingContainer()const;
    virtual bool isInTouchMode()const;
    bool isFocusable()const;
    void setFocusable(bool);
    int  getFocusable()const;
    void notifyGlobalFocusCleared(View* oldFocus);
    bool rootViewRequestFocus();
    virtual void unFocus(View*);
    virtual bool hasFocus()const;
    virtual bool restoreFocusInCluster(int direction);
    virtual bool restoreFocusNotInCluster();
    virtual bool restoreDefaultFocus();
    bool requestRectangleOnScreen(Rect& rectangle, bool immediate=false);
    void clearFocus();
    virtual View*findFocus();
    bool requestFocus(int direction=FOCUS_DOWN);
    bool requestFocusFromTouch();
    int getImportantForAccessibility()const;
    void setImportantForAccessibility(int mode);
    bool isImportantForAccessibility();
    ViewGroup* getParentForAccessibility();
    View*getSelfOrParentImportantForA11y();
    virtual void addChildrenForAccessibility(std::vector<View*>& outChildren);
    bool includeForAccessibility();
    void setAccessibilityLiveRegion(int mode);
    int  getAccessibilityLiveRegion() const;
    virtual bool requestFocus(int direction,Rect* previouslyFocusedRect);
    bool hasFocusable()const{ return hasFocusable(true, false); }
    virtual bool hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const;
    bool hasExplicitFocusable()const;
    virtual View*keyboardNavigationClusterSearch(View* currentCluster,int direction);
    // Parent and children views
    virtual ViewGroup*getParent()const;
    ViewTreeObserver* getViewTreeObserver();
    ViewGroup* getRootView()const;
    bool toGlobalMotionEvent(MotionEvent& ev);
    bool toLocalMotionEvent(MotionEvent& ev);
    void transformMatrixToGlobal(Matrix& matrix);
    void transformMatrixToLocal(Matrix& matrix);
    void bringToFront();

    virtual View* findViewById(int id);
    virtual View* findViewWithTag(void*);
    virtual View* findViewTraversal(int);
    virtual View* findViewByAccessibilityId(int accessibilityId);
    virtual View* findViewByAccessibilityIdTraversal(int accessibilityId);
    virtual View* findViewByPredicateTraversal(const Predicate<View*>&predicate,View* childToSkip);
    virtual View* findViewWithTagTraversal(void* tag);
    View* findViewByPredicate(const Predicate<View*>&predicate);
    View* findViewByPredicateInsideOut(View*start,const Predicate<View*>&predicate);

    virtual View*focusSearch(int direction)const;
    View*findUserSetNextFocus(View*root,int direction)const;
    View*findUserSetNextKeyboardNavigationCluster(View*root,int direction)const;
    View*findKeyboardNavigationCluster()const;
    virtual void addTouchables(std::vector<View*>& views);
    virtual void addFocusables(std::vector<View*>& views,int direction);
    virtual void addFocusables(std::vector<View*>& views,int direction,int focusableMode);

    std::vector<View*>getFocusables(int direction);
    void setKeyboardNavigationCluster(bool);
    bool isKeyboardNavigationCluster()const;
    virtual void addKeyboardNavigationClusters(std::vector<View*>&views,int drection);
    virtual bool dispatchTouchEvent(MotionEvent& event);
    virtual bool dispatchGenericMotionEvent(MotionEvent& event);
    bool dispatchPointerEvent(MotionEvent& event);

    KeyEvent::DispatcherState* getKeyDispatcherState()const;
    virtual bool dispatchKeyEvent(KeyEvent&event);
    virtual bool dispatchKeyShortcutEvent(KeyEvent&);
    virtual View* dispatchUnhandledKeyEvent(KeyEvent& evt);
    virtual bool dispatchUnhandledMove(View* focused,int direction);
    virtual bool onKeyPreIme(int keyCode, KeyEvent& event);
    virtual bool onKeyShortcut(int keyCode, KeyEvent& event);
    bool onKeyUp(int keycode,KeyEvent& evt)override;
    bool onKeyDown(int keycode,KeyEvent& evt)override;
    bool onKeyLongPress(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int count, KeyEvent& event)override;
    virtual bool onCheckIsTextEditor();
    virtual bool onUnhandledKeyEvent(KeyEvent& event);
    virtual bool hasUnhandledKeyListener()const;
    void addOnUnhandledKeyEventListener(const OnUnhandledKeyEventListener& listener);
    void removeOnUnhandledKeyEventListener(const OnUnhandledKeyEventListener& listener);

    virtual void createContextMenu(ContextMenu& menu);
    virtual int  commitText(const std::wstring&);
    virtual void dispatchDisplayHint(/*Visibility*/int hint);
    virtual void dispatchWindowVisibilityChanged(int visibility);
    virtual void onWindowVisibilityChanged(int);
    virtual void onVisibilityAggregated(bool isVisible);
    virtual bool onInterceptTouchEvent(MotionEvent& evt);
    virtual bool onTouchEvent(MotionEvent& evt);
    virtual bool onHoverEvent(MotionEvent& evt);
    virtual bool onTrackballEvent(MotionEvent& event);
    virtual bool onGenericMotionEvent(MotionEvent& event);
    virtual void onHoverChanged(bool hovered);
	
    void postOnAnimation(Runnable& action);
    void postOnAnimationDelayed(Runnable& action, long delayMillis);
    bool post(Runnable& what);
    bool post(const std::function<void()>&what);
    bool postDelayed(const std::function<void()>&what,long delay=0);
    virtual bool postDelayed(Runnable& what,long delay=0);
    virtual bool removeCallbacks(const Runnable& what);

    virtual int getBaseline();
    static bool isLayoutModeOptical(View*);
    virtual bool resolveRtlPropertiesIfNeeded();
    void resetRtlProperties();
    
    virtual void dispatchScreenStateChanged(int screenState);
    virtual void onScreenStateChanged(int screenState);
    virtual void dispatchMovedToDisplay(Display& display, Configuration& config);
    virtual void onMovedToDisplay(int displayId, Configuration& config);

    virtual void resetResolvedTextDirection();
    virtual void resetResolvedLayoutDirection();
    virtual void resetResolvedPaddingInternal();
    virtual void resetResolvedPadding();
    const Display* getDisplay()const;
    void measure(int widthMeasureSpec, int heightMeasureSpec);
    int  getMeasuredWidth()const;
    int  getMeasuredWidthAndState()const;
    int  getMeasuredHeight()const;
    int  getMeasuredState()const;
    int  getMeasuredHeightAndState()const;

    Matrix& getMatrix();
    Matrix& getInverseMatrix();
    bool hasIdentityMatrix()const;

    void setX(float);
    void setY(float);
    void setZ(float);
    float getX()const;//x pos to screen
    float getY()const;//y pos to screen
    float getZ()const;
    float getElevation()const;
    void setElevation(float elevation);
    float getTranslationX()const;
    float getTranslationY()const;
    float getTranslationZ()const;
    void setTranslationX(float x);
    void setTranslationY(float y);
    void setTranslationZ(float z);

    float getScaleX()const;
    void  setScaleX(float);
    float getScaleY()const;
    void  setScaleY(float);
    float getPivotX()const;
    void  setPivotX(float);
    float getPivotY()const;
    void  setPivotY(float);
    bool  isPivotSet()const;
    void  resetPivot();
    float getAlpha()const;
    void  setAlpha(float);
    bool setAlphaNoInvalidation(float);
    float getTransitionAlpha()const;
    void setTransitionAlpha(float);

    float getRotation()const;
    void  setRotation(float rotation);
    float getRotationX()const;
    void  setRotationX(float);
    float getRotationY()const;
    void  setRotationY(float);
    StateListAnimator* getStateListAnimator()const;
    void setStateListAnimator(StateListAnimator*);
    bool getClipToOutline()const;
    void setClipToOutline(bool clip2Outline);
    void setOutlineProvider(ViewOutlineProvider provider);
    ViewOutlineProvider getOutlineProvider()const;
    void invalidateOutline();
    ViewPropertyAnimator& animate();
    LayoutParams*getLayoutParams()const;
    int getRawLayoutDirection()const;
    bool isLayoutDirectionInherited()const;
    void setLayoutParams(LayoutParams*lp);
    virtual ViewOverlay*getOverlay();
    virtual bool isLayoutRequested()const;
    virtual bool isInLayout()const;
    bool isLayoutValid()const;
    bool hasRtlSupport()const;
    bool isRtlCompatibilityMode()const;
    bool isTextDirectionInherited()const;
    bool isTextDirectionResolved()const;
    virtual bool resolveTextDirection();
    virtual void requestLayout();
    void forceLayout();
    virtual void resolveLayoutParams();
    void layout(int l, int t, int r, int b);
};

class View::AttachInfo{
public:
    class InvalidateInfo{
    private:
        static std::vector<InvalidateInfo*>sPool;
    public:
        View* target;
        nsecs_t time;
        Rect rect;
        static InvalidateInfo*obtain();
        InvalidateInfo();
        void recycle();
    };
    Display*mDisplay;
    ViewGroup*mRootView;
    bool mHardwareAccelerated;
    bool mOverscanRequested;
    float mApplicationScale;
    int mWindowLeft;
    int mWindowTop;
    int mAccessibilityWindowId;
    int mAccessibilityFetchFlags;
    int mSystemUiVisibility;
    int mDisabledSystemUiVisibility;
    int mGlobalSystemUiVisibility;
    int mDisplayState;
    Drawable*mAccessibilityFocusDrawable;
    Rect mOverscanInsets;
    Rect mContentInsets;
    Rect mVisibleInsets;
    Rect mStableInsets;
    Rect mOutsets;
    KeyEvent::DispatcherState mKeyDispatchState;
    bool mAlwaysConsumeNavBar;
    bool mAlwaysConsumeSystemBars;
    bool mHasWindowFocus;
    bool mScalingRequired;
    bool mUse32BitDrawingCache;
    bool mViewVisibilityChanged;
    bool mViewScrollChanged;
    bool mHandlingPointerEvent;
    bool mIgnoreDirtyState;
    int mWindowVisibility;
    int64_t mDrawingTime;
    bool mInTouchMode;
    bool mUnbufferedDispatchRequested;
    bool mRecomputeGlobalAttributes;
    bool mKeepScreenOn;
    bool mHasSystemUiListeners;
    bool mDebugLayout;
    bool mNextFocusLooped;
    UIEventSource*mEventSource;
    std::function<void(int)>mPlaySoundEffect;
    std::function<bool(int,bool)>mPerformHapticFeedback;
    Cairo::RefPtr<Canvas> mCanvas;
    Drawable*mAutofilledDrawable;
    void*mDragToken;
    Cairo::RefPtr<Cairo::ImageSurface>mDragSurface;
    View* mTooltipHost;
    View* mViewRequestingLayout;
    ViewTreeObserver* mTreeObserver;
    std::vector<View*> mScrollContainers;
    AttachInfo(Context*ctx);
    ~AttachInfo();
};

class View::TransformationInfo{
public:
    Matrix mMatrix;
    Matrix mInverseMatrix;
    float mAlpha = 1.f;
    float mTransitionAlpha = 1.f;
    TransformationInfo();
};

class View::TintInfo{
public:
    const ColorStateList*mTintList;
    int mBlendMode;
    int mTintMode;
    bool mHasTintMode;
    bool mHasTintList;
    TintInfo();
    ~TintInfo();
};
class View::ForegroundInfo {
public:
    Drawable* mDrawable;
    TintInfo* mTintInfo;
    int mGravity;
    bool mInsidePadding;
    bool mBoundsChanged;
    Rect mSelfBounds;
    Rect mOverlayBounds;
public:
    ForegroundInfo();
    ~ForegroundInfo();
};

class View::ListenerInfo{
public:
    bool mPreferKeepClear;
    std::vector<Rect> mSystemGestureExclusionRects;
    std::vector<Rect> mKeepClearRects;
    std::vector<Rect> mUnrestrictedKeepClearRects;
    View::OnFocusChangeListener mOnFocusChangeListener;
    std::vector<View::OnLayoutChangeListener> mOnLayoutChangeListeners;
    std::vector<View::OnAttachStateChangeListener> mOnAttachStateChangeListeners;
    View::OnScrollChangeListener mOnScrollChangeListener;
    View::OnClickListener mOnClickListener;
    View::OnLongClickListener mOnLongClickListener;
    View::OnContextClickListener mOnContextClickListener;
    OnCreateContextMenuListener mOnCreateContextMenuListener;
    View::OnKeyListener mOnKeyListener;
    View::OnTouchListener mOnTouchListener;
    View::OnHoverListener mOnHoverListener;
    View::OnCapturedPointerListener mOnCapturedPointerListener;
    View::OnGenericMotionListener mOnGenericMotionListener;
    std::vector<View::OnUnhandledKeyEventListener> mUnhandledKeyListeners;
    View::OnDragListener mOnDragListener;
    OnSystemUiVisibilityChangeListener mOnSystemUiVisibilityChangeListener;
    OnApplyWindowInsetsListener mOnApplyWindowInsetsListener;
};

class View::CheckForTap{
protected:
    View*mView;
    float mX,mY;
    Runnable mRunnable;
public:
    CheckForTap(View*v);
    void setAnchor(float,float);
    virtual void run();
    void postDelayed(long);
    void removeCallbacks();
};

class View::CheckForLongPress:public CheckForTap{
private:
    int mOriginalWindowAttachCount;
    bool mOriginalPressedState;
public:
    CheckForLongPress(View*v);
    void run()override;
    void rememberWindowAttachCount();
    void rememberPressedState();
};

class View::SendViewScrolledAccessibilityEvent{
private:
    friend View;
    View*mView;
    Runnable mRunnable;
    void reset();
public:
    int mDeltaX;
    int mDeltaY;
    bool mIsPending;
    SendViewScrolledAccessibilityEvent(View*v);
    void post(int dx, int dy);
    void run();
};

class View::TooltipInfo{
public:
    std::string mTooltipText;
    int mAnchorX,mAnchorY;
    /*TooltipPopup*/void* mTooltipPopup;
    bool mTooltipFromLongClick;
    Runnable mShowTooltipRunnable;
    Runnable mHideTooltipRunnable;
    int mHoverSlop;
public:
    bool updateAnchorPos(MotionEvent&event);
    void clearAnchorPos();
};

class View::ScrollabilityCache:public Runnable{
private:
    static constexpr float OPAQUE[] = { 255 };
    static constexpr float TRANSPARENT[] = { 0.0f };
public:
    static constexpr int OFF =0;
    static constexpr int ON  =1;
    static constexpr int FADING=2;

    static constexpr int NOT_DRAGGING = 0;
    static constexpr int DRAGGING_VERTICAL_SCROLL_BAR = 1;
    static constexpr int DRAGGING_HORIZONTAL_SCROLL_BAR = 2;

    bool fadeScrollBars;
    int fadingEdgeLength;
    int scrollBarDefaultDelayBeforeFade;
    int scrollBarFadeDuration;

    int scrollBarSize;
    int scrollBarMinTouchTarget;

    ScrollBarDrawable*scrollBar;
    View* host;
    Runnable mRunner;
    int64_t fadeStartTime;
    int state;
    int mLastColor;
    Rect mScrollBarBounds;
    Rect mScrollBarTouchBounds;

    int mScrollBarDraggingState = NOT_DRAGGING;
    int mScrollBarDraggingPos;
    Cairo::RefPtr<Cairo::LinearGradient> shader;
public:
    ScrollabilityCache(ViewConfiguration&configuration,View*host);
    virtual ~ScrollabilityCache();
    void run();
};

class View::MeasureSpec{
public:
    enum{
        MODE_SHIFT = 30,
        MODE_MASK  = 3 << MODE_SHIFT,
        UNSPECIFIED= 0,
        EXACTLY = 1 << MODE_SHIFT,
        AT_MOST = 2 << MODE_SHIFT
    };
public:
    static int makeMeasureSpec(int size,int mode);
    static int makeSafeMeasureSpec(int size, int mode);
    static int getMode(int measureSpec);
    static int getSize(int measureSpec);
    static int adjust(int measureSpec, int delta);
    static const std::string toString(int measureSpec) ;
};
using MeasureSpec= View::MeasureSpec;

class View::BaseSavedState:public AbsSavedState {
public:
    static constexpr int START_ACTIVITY_REQUESTED_WHO_SAVED = 0b1;
    static constexpr int IS_AUTOFILLED = 0b10;
    static constexpr int AUTOFILL_ID = 0b100;
public:
    // Flags that describe what data in this state is valid
    int mSavedData;
    std::string mStartActivityRequestWhoSaved;
    bool mIsAutofilled;
    bool mHideHighlight;
    int mAutofillViewId;
public:
    BaseSavedState(Parcel& source);
    BaseSavedState(Parcelable* superState);
    void writeToParcel(Parcel& out, int flags)override;
};

class View::AccessibilityDelegate {
public:
    virtual void sendAccessibilityEvent(View& host, int eventType);

    virtual bool performAccessibilityAction(View& host, int action,Bundle* args);

    virtual void sendAccessibilityEventUnchecked(View& host,AccessibilityEvent& event);

    virtual bool dispatchPopulateAccessibilityEvent(View& host,AccessibilityEvent& event);

    virtual void onPopulateAccessibilityEvent(View& host,AccessibilityEvent& event);

    virtual void onInitializeAccessibilityEvent(View& host,AccessibilityEvent& event);

    virtual void onInitializeAccessibilityNodeInfo(View& host,AccessibilityNodeInfo& info);

    virtual void addExtraDataToAccessibilityNodeInfo(View& host,AccessibilityNodeInfo& info,
            const std::string& extraDataKey,Bundle* arguments);

    virtual bool onRequestSendAccessibilityEvent(ViewGroup& host, View& child,AccessibilityEvent& event);

    virtual AccessibilityNodeProvider* getAccessibilityNodeProvider(View& host);
    virtual AccessibilityNodeInfo* createAccessibilityNodeInfo(View& host);
};

class View::DragShadowBuilder {
private:
    View* mView;
public:
    DragShadowBuilder(View* view);
    DragShadowBuilder();
    View* getView();
    void onProvideShadowMetrics(Point& outShadowSize, Point& outShadowTouchPoint);
    void onDrawShadow(Canvas& canvas);
};
}//endof namespace cdroid

using namespace cdroid;

#endif
