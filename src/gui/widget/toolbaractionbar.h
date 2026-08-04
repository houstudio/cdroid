#ifndef __TOOLBARACTIONBAR_H__
#define __TOOLBARACTIONBAR_H__
#include <widget/actionbar.h>
#include <widget/toolbar.h>
#include <string>
#include <vector>
namespace cdroid{

class Menu;
class MenuItem;
class Window;

// Bridges a Toolbar so it behaves as its host Activity's ActionBar. Single-layer CDROID
// port that folds androidx ToolbarActionBar + ToolbarWidgetWrapper into one class:
// CDROID has no Window.Callback abstraction and ships no WindowDecorActionBar, so the
// DecorToolbar indirection (a single implementation) is inlined here. The host Activity
// plays the Window.Callback role — menu dispatch routes directly to its
// onCreateOptionsMenu / onPrepareOptionsMenu / onOptionsItemSelected.
class ToolbarActionBar : public ActionBar{
public:
    // host is the owning Activity; title seeds the toolbar title (mirrors androidx
    // passing Activity.getTitle() into the ToolbarActionBar ctor).
    ToolbarActionBar(Toolbar* toolbar, const std::string& title, Window* host);
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
    void setHomeButtonEnabled(bool enabled) override; // no-op, matches androidx
    void addOnMenuVisibilityListener(const OnMenuVisibilityListener& listener) override;
    void removeOnMenuVisibilityListener(const OnMenuVisibilityListener& listener) override;
    void invalidateOptionsMenu() override;

    // deprecated Tab/List — throw, matching androidx ToolbarActionBar.
    int  getNavigationMode() const override;
    void setNavigationMode(int mode) override;
    void* newTab() override;

    // Rebuilds the toolbar menu from the host's onCreateOptionsMenu/onPrepareOptionsMenu.
    void populateOptionsMenu();
private:
    // AFFECTS_LOGO_MASK from androidx ToolbarWidgetWrapper.
    static constexpr int AFFECTS_LOGO_MASK = DISPLAY_USE_LOGO | DISPLAY_SHOW_HOME;

    Menu* getMenu();                       // installs menu callbacks once, returns toolbar menu
    void applyDisplayOptions(int newOpts); // bit -> visual translation (inlined wrapper logic)
    void updateNavigationIcon();
    void updateToolbarLogo();
    void fireHomePressed();                // synthesize R::id::home item -> host onOptionsItemSelected
    void dispatchMenuVisibilityChanged(bool isVisible);
    int  detectDisplayOptions();

    Toolbar* mToolbar;
    Window* mHost;                         // owning Activity (Activity == typedef Window); not owned
    std::string mTitle;
    std::string mSubtitle;
    bool mTitleSet = false;
    int mDisplayOpts = 0;
    bool mMenuPrepared = false;
    bool mMenuCallbackSet = false;
    bool mLastMenuVisibility = false;
    Drawable* mNavIcon = nullptr;            // setHomeAsUpIndicator; borrowed
    Drawable* mDefaultNavigationIcon = nullptr; // theme homeAsUpIndicator; borrowed
    Drawable* mIcon = nullptr;               // borrowed
    Drawable* mLogo = nullptr;               // borrowed
    View* mCustomView = nullptr;             // borrowed
    std::vector<OnMenuVisibilityListener> mMenuVisibilityListeners;
};

}//namespace
#endif
