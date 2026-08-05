#ifndef __ACTION_BAR_H__
#define __ACTION_BAR_H__
#include <view/viewgroup.h>
#include <string>
#include <functional>
#include <vector>
namespace cdroid{

class Drawable;

// Port of android.app.ActionBar. A base class with default convenience
// implementations; CDROID ships a single concrete subclass, ToolbarActionBar,
// that bridges a Toolbar to behave as an Activity's ActionBar. The legacy
// theme-decor variant (WindowDecorActionBar / FEATURE_ACTION_BAR) is intentionally
// not ported — CDROID has no DecorView/feature-window mechanism.
class ActionBar{
public:
    // Navigation modes (deprecated upstream; only STANDARD is honoured).
    static constexpr int NAVIGATION_MODE_STANDARD = 0;
    static constexpr int NAVIGATION_MODE_TABS     = 1; // deprecated; not supported
    static constexpr int NAVIGATION_MODE_LIST     = 2; // deprecated; not supported

    // Display option bits for setDisplayOptions().
    static constexpr int DISPLAY_USE_LOGO    = 0x1;
    static constexpr int DISPLAY_SHOW_HOME   = 0x2;
    static constexpr int DISPLAY_HOME_AS_UP  = 0x4;
    static constexpr int DISPLAY_SHOW_TITLE  = 0x8;
    static constexpr int DISPLAY_SHOW_CUSTOM = 0x10;

    class LayoutParams:public ViewGroup::MarginLayoutParams{
    public:
        int gravity = Gravity::NO_GRAVITY;
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, int gravity);
        LayoutParams(int gravity);
        LayoutParams(const LayoutParams& source);
        LayoutParams(const ViewGroup::LayoutParams& source);
    };

    // CDROID-style listener (matches the MenuBuilder::Callback pattern).
    class OnMenuVisibilityListener:public EventSet{
    public:
        std::function<void(bool)> onMenuVisibilityChanged;
    };

    virtual ~ActionBar() = default;

    // --- pure virtual: implementation-defined behaviour ---
    virtual void setDisplayOptions(int options) = 0;
    virtual void setDisplayOptions(int options, int mask) = 0;
    virtual int  getDisplayOptions() const = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual void setSubtitle(const std::string& subtitle) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isShowing() const = 0;
    virtual int  getHeight() const = 0;
    virtual Context* getThemedContext() = 0;
    virtual void setCustomView(View* view) = 0;
    virtual void setIcon(Drawable* icon) = 0;
    virtual void setLogo(Drawable* logo) = 0;
    virtual void setHomeButtonEnabled(bool enabled) = 0; // no-op under ToolbarActionBar
    virtual void addOnMenuVisibilityListener(const OnMenuVisibilityListener& listener) = 0;
    virtual void removeOnMenuVisibilityListener(const OnMenuVisibilityListener& listener) = 0;
    virtual void invalidateOptionsMenu() = 0;

    // --- concrete bit-toggle convenience (default impls in .cc) ---
    virtual void setDisplayUseLogoEnabled(bool useLogo);
    virtual void setDisplayShowHomeEnabled(bool showHome);
    virtual void setDisplayHomeAsUpEnabled(bool showHomeAsUp);
    virtual void setDisplayShowTitleEnabled(bool showTitle);
    virtual void setDisplayShowCustomEnabled(bool showCustom);
    virtual void setDefaultDisplayHomeAsUpEnabled(bool enabled); // default no-op

    // android.app.ActionBar.setHomeAsUpIndicator: the Up indicator drawable. Default no-op;
    // ToolbarActionBar overrides it to drive the Toolbar navigation icon.
    virtual void setHomeAsUpIndicator(Drawable* indicator) {}

    // android.app.ActionBar.setHomeActionContentDescription. Default no-op; ToolbarActionBar overrides.
    virtual void setHomeActionContentDescription(const std::string& description) {}

    // Programmatic options-menu open/close (android.app.Activity.open/closeOptionsMenu).
    // Default no-op; ToolbarActionBar overrides to drive the toolbar overflow menu.
    virtual bool openOptionsMenu() { return false; }
    virtual bool closeOptionsMenu() { return false; }

    // --- deprecated Tab/List navigation (stubbed) ---
    virtual int  getNavigationMode() const { return NAVIGATION_MODE_STANDARD; }
    virtual void setNavigationMode(int mode); // deprecated; throws on TABS/LIST
    virtual void* newTab();                   // deprecated; throws
};

}//namespace
#endif
