/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36
 *   com.android.internal.widget.floatingtoolbar.FloatingToolbar.
 * A floating toolbar for showing contextual menu items: shows as many menu item
 * buttons as can fit in the horizontal toolbar and the remaining menu items in a
 * vertical overflow view when the overflow button is clicked. The horizontal
 * toolbar morphs into the vertical overflow view.
 * This class is responsible only for the public API; rendering is delegated to
 * FloatingToolbarPopup.
 *********************************************************************************/
#ifndef __FLOATING_TOOLBAR_H__
#define __FLOATING_TOOLBAR_H__
#include <string>
#include <functional>
#include <vector>
#include <core/rect.h>
#include <view/view.h>   // View::OnLayoutChangeListener
#include <menu/menuitem.h>   // MenuItem::OnMenuItemClickListener
namespace cdroid{
class Window;
class Menu;
class FloatingToolbarPopup;

class FloatingToolbar {
public:
    static constexpr const char* FLOATING_TOOLBAR_TAG = "floating_toolbar";

    explicit FloatingToolbar(Window* window);
    ~FloatingToolbar();

    FloatingToolbar& setMenu(Menu* menu);
    FloatingToolbar& setOnMenuItemClickListener(const MenuItem::OnMenuItemClickListener& menuItemClickListener);
    FloatingToolbar& setContentRect(const Rect& rect);
    FloatingToolbar& setSuggestedWidth(int suggestedWidth);
    FloatingToolbar& show();
    FloatingToolbar& updateLayout();
    void dismiss();
    void hide();
    bool isShowing();
    bool isHidden();
    void setOutsideTouchable(bool outsideTouchable, const std::function<void()>& onDismiss);

private:
    void doShow();
    static std::vector<MenuItem*> getVisibleAndEnabledMenuItems(Menu* menu);
    void registerOrientationHandler();
    void unregisterOrientationHandler();

    Window* mWindow;
    FloatingToolbarPopup* mPopup;
    Rect mContentRect;
    Menu* mMenu = nullptr;
    MenuItem::OnMenuItemClickListener mMenuItemClickListener;   // default = NO_OP (returns false)
    View::OnLayoutChangeListener mOrientationChangeHandler;
};
};//namespace
#endif/*__FLOATING_TOOLBAR_H__*/
