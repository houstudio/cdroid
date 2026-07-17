/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36
 *   com.android.internal.widget.floatingtoolbar.FloatingToolbarPopup (interface).
 *********************************************************************************/
#ifndef __FLOATING_TOOLBAR_POPUP_H__
#define __FLOATING_TOOLBAR_POPUP_H__
#include <functional>
#include <vector>
#include <core/rect.h>
#include <menu/menuitem.h>   // MenuItem::OnMenuItemClickListener
namespace cdroid{
class Context;
class View;

/** Popup window used by {@link FloatingToolbar} to render menu items.
 *  Mirrors AOSP FloatingToolbarPopup (interface). */
class FloatingToolbarPopup {
public:
    virtual ~FloatingToolbarPopup() = default;

    /** Sets the suggested dp width of this floating toolbar. The actual width will be about
     *  this size but there are no guarantees that it will be exactly the suggested width. */
    virtual void setSuggestedWidth(int suggestedWidth) = 0;

    /** Sets if the floating toolbar width changed. */
    virtual void setWidthChanged(bool widthChanged) = 0;

    /** Shows this popup at coordinates derived from the content rect, which may be adjusted
     *  to make sure the popup is entirely on-screen. */
    virtual void show(const std::vector<MenuItem*>& menuItems,
                      const MenuItem::OnMenuItemClickListener& menuItemClickListener,
                      const Rect& contentRect) = 0;

    /** Gets rid of this popup. No-op if not currently showing. */
    virtual void dismiss() = 0;

    /** Hides this popup. No-op if not showing. Use isHidden() to distinguish hidden vs dismissed. */
    virtual void hide() = 0;

    /** Returns true if this popup is currently showing. */
    virtual bool isShowing() = 0;

    /** Returns true if this popup is currently hidden. */
    virtual bool isHidden() = 0;

    /** Makes this toolbar "outside touchable" and sets the onDismiss listener.
     *  @return true if the "outsideTouchable" setting was modified. */
    virtual bool setOutsideTouchable(bool outsideTouchable,
                                     const std::function<void()>& onDismiss) = 0;

    /** Returns a LocalFloatingToolbarPopup implementation. */
    static FloatingToolbarPopup* createInstance(Context* context, View* parent);
};
};//namespace
#endif/*__FLOATING_TOOLBAR_POPUP_H__*/
