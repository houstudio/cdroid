#ifndef __TOOLBARWIDGETWRAPPER_H__
#define __TOOLBARWIDGETWRAPPER_H__
/*********************************************************************************
 * Port of androidx.appcompat.widget.ToolbarWidgetWrapper — the DecorToolbar implementation that
 * adapts a Toolbar to behave as the window's decor action bar. Owns the display-option state, the
 * navigation icon (app-supplied mNavIcon wins over the theme default mDefaultNavigationIcon), the
 * cached title/subtitle, and the home/up click dispatch (synthesizes R::id::home and routes it
 * through the WindowCallback exactly as upstream).
 *
 * Only the style=false path is supported (the only ctor call site is ToolbarActionBar's
 * `new ToolbarWidgetWrapper(toolbar, false)`); the style=true theme-attr block is not ported.
 *********************************************************************************/
#include <widget/decortoolbar.h>
#include <widget/actionbar.h>
#include <string>
namespace cdroid{
class Toolbar;
class WindowCallback;
class ActionMenuPresenter;
class Context;
class Drawable;
class View;
class ViewGroup;

class ToolbarWidgetWrapper : public DecorToolbar{
public:
    ToolbarWidgetWrapper(Toolbar* toolbar, bool style);
    ~ToolbarWidgetWrapper() override;

    ViewGroup* getViewGroup() override;
    Context* getContext() override;
    bool hasExpandedActionView() override;
    void collapseActionView() override;
    void setWindowCallback(WindowCallback* cb) override;
    void setWindowTitle(const std::string& title) override;
    std::string getTitle() override;
    void setTitle(const std::string& title) override;
    std::string getSubtitle() override;
    void setSubtitle(const std::string& subtitle) override;
    void setIcon(Drawable* d) override;
    void setLogo(Drawable* d) override;
    bool canShowOverflowMenu() override;
    bool isOverflowMenuShowing() override;
    bool isOverflowMenuShowPending() override;
    bool showOverflowMenu() override;
    bool hideOverflowMenu() override;
    void setMenuPrepared() override;
    void setMenu(Menu* menu, const MenuPresenter::Callback& cb) override;
    void dismissPopupMenus() override;
    int getDisplayOptions() override;
    void setDisplayOptions(int opts) override;
    bool isTitleTruncated() override;
    void setCollapsible(bool collapsible) override;
    void setHomeButtonEnabled(bool enable) override;
    void setCustomView(View* view) override;
    View* getCustomView() override;
    void setNavigationIcon(Drawable* icon) override;
    void setNavigationContentDescription(const std::string& description) override;
    void setDefaultNavigationIcon(Drawable* icon) override;
    void setBackgroundDrawable(Drawable* d) override;
    int getHeight() override;
    void setVisibility(int visible) override;
    int getVisibility() override;
    void setMenuCallbacks(const MenuPresenter::Callback& presenterCallback,
                          const MenuBuilder::Callback& menuBuilderCallback) override;
    Menu* getMenu() override;
    int  getNavigationMode() override;
    void setNavigationMode(int mode) override;

    // CDROID seam: used by ToolbarActionBar.populateOptionsMenu to flip the home-gate, since CDROID
    // has no PhoneWindow onPreparePanel driver to call setMenuPrepared() organically (see plan 接缝5).
    bool isMenuPrepared() const { return mMenuPrepared; }
private:
    static constexpr int AFFECTS_LOGO_MASK = ActionBar::DISPLAY_SHOW_HOME | ActionBar::DISPLAY_USE_LOGO;

    int  detectDisplayOptions();
    void updateNavigationIcon();
    void updateHomeAccessibility();
    void updateToolbarLogo();
    void setTitleInt(const std::string& title);

    Toolbar* mToolbar;
    WindowCallback* mWindowCallback = nullptr;
    ActionMenuPresenter* mActionMenuPresenter = nullptr; // owned
    int mDisplayOpts = 0;
    int mNavigationMode = ActionBar::NAVIGATION_MODE_STANDARD;
    View* mCustomView = nullptr;                 // borrowed
    Drawable* mIcon = nullptr;                   // borrowed
    Drawable* mLogo = nullptr;                   // borrowed
    Drawable* mNavIcon = nullptr;                // borrowed; app Up indicator (setHomeAsUpIndicator)
    Drawable* mDefaultNavigationIcon = nullptr;  // borrowed; theme homeAsUpIndicator fallback
    bool mTitleSet = false;
    std::string mTitle;
    std::string mSubtitle;
    std::string mHomeDescription;
    int mDefaultNavigationContentDescription = 0;
    bool mMenuPrepared = false;
};

}//namespace
#endif
