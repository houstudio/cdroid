#ifndef __TOOLBARACTIONBAR_H__
#define __TOOLBARACTIONBAR_H__
#include <widget/actionbar.h>
#include <widget/toolbar.h>
#include <string>
#include <vector>
namespace cdroid{

class Menu;
class MenuItem;
class MenuBuilder;
class WindowCallback;
class DecorToolbar;

// Port of androidx.appcompat.app.ToolbarActionBar — bridges a Toolbar so it behaves as its host
// Activity's ActionBar. Holds a DecorToolbar (a ToolbarWidgetWrapper) and delegates every setter to
// it, exactly as upstream holds `DecorToolbar mDecorToolbar`. Menu/home dispatch routes through the
// host (a WindowCallback = the Window/Activity) via the panel methods onCreatePanelMenu /
// onPreparePanel / onMenuItemSelected / onMenuOpened / onPanelClosed.
class ToolbarActionBar : public ActionBar{
public:
    // windowCallback is the owning Activity (CDROID's Window plays androidx's Window.Callback role).
    ToolbarActionBar(Toolbar* toolbar, const std::string& title, WindowCallback* windowCallback);
    ~ToolbarActionBar() override;

    void setDisplayOptions(int options) override;
    void setDisplayOptions(int options, int mask) override;
    int  getDisplayOptions() const override;
    void setTitle(const std::string& title) override;
    void setSubtitle(const std::string& subtitle) override;
    void show() override;
    void hide() override;
    bool isShowing() const override;
    int  getHeight() const override;
    Context* getThemedContext() override;
    void setCustomView(View* view) override;
    void setIcon(Drawable* icon) override;
    void setLogo(Drawable* logo) override;
    void setHomeAsUpIndicator(Drawable* indicator) override;
    void setHomeActionContentDescription(const std::string& description) override;
    void setHomeButtonEnabled(bool enabled) override; // no-op, matches androidx
    void addOnMenuVisibilityListener(const OnMenuVisibilityListener& listener) override;
    void removeOnMenuVisibilityListener(const OnMenuVisibilityListener& listener) override;
    bool openOptionsMenu() override;
    bool closeOptionsMenu() override;
    void invalidateOptionsMenu() override;

    // deprecated Tab/List — throw, matching androidx ToolbarActionBar.
    int  getNavigationMode() const override;
    void setNavigationMode(int mode) override;
    void* newTab() override;

    // Rebuilds the toolbar menu from the host's onCreatePanelMenu/onPreparePanel.
    void populateOptionsMenu();
private:
    Menu* getMenu();                       // installs menu callbacks once, returns the menu
    void dispatchMenuVisibilityChanged(bool isVisible);

    DecorToolbar* mDecorToolbar;           // owned (a ToolbarWidgetWrapper)
    WindowCallback* mWindowCallback;       // the host Activity; not owned
    bool mMenuCallbackSet = false;
    bool mLastMenuVisibility = false;
    bool mToolbarMenuPrepared = false;
    bool mClosingActionMenu = false;
    std::vector<OnMenuVisibilityListener> mMenuVisibilityListeners;
};

}//namespace
#endif
