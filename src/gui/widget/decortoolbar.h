#ifndef __DECORTOOLBAR_H__
#define __DECORTOOLBAR_H__
/*********************************************************************************
 * Port of androidx.appcompat.widget.DecorToolbar — "Common interface for a toolbar that sits as
 * part of the window decor." Layouts that control window decor use this to interact with different
 * bar implementations.
 *
 * CDROID, like androidx, has a single implementation (ToolbarWidgetWrapper): the legacy
 * WindowDecorActionBar / FEATURE_ACTION_BAR theme machinery is intentionally not ported (no
 * DecorView). The interface is kept for structural parity — ToolbarActionBar holds a DecorToolbar*
 * and delegates every setter to it, exactly as upstream holds `DecorToolbar mDecorToolbar`.
 *
 * Deviations from the upstream interface (documented):
 *  - No `int resId` overloads (setIcon/setLogo/setNavigationIcon/setNavigationContentDescription):
 *    CDROID has no Context::getText(int); resource lookups are string-keyed.
 *  - setWindowCallback takes a WindowCallback* (CDROID's Window plays android.view.Window.Callback).
 *  - Deprecated list navigation (setDropdownParams / setEmbeddedTabView / Spinner), progress display
 *    (initProgress), and hierarchy-state save/restore (SparseArray<Parcelable>) are omitted — CDROID's
 *    ActionBar surface does not expose them. See ActionBar stubs for NAVIGATION_MODE_TABS/LIST.
 *********************************************************************************/
#include <string>
#include <menu/menu.h>
#include <menu/menubuilder.h>
#include <menu/menupresenter.h>
namespace cdroid{
class Context;
class Drawable;
class View;
class ViewGroup;
class WindowCallback;

class DecorToolbar{
public:
    virtual ~DecorToolbar() = default;

    virtual ViewGroup* getViewGroup() = 0;
    virtual Context* getContext() = 0;

    virtual bool hasExpandedActionView() = 0;
    virtual void collapseActionView() = 0;

    virtual void setWindowCallback(WindowCallback* cb) = 0;

    virtual void setWindowTitle(const std::string& title) = 0;
    virtual std::string getTitle() = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual std::string getSubtitle() = 0;
    virtual void setSubtitle(const std::string& subtitle) = 0;

    virtual void setIcon(Drawable* d) = 0;
    virtual void setLogo(Drawable* d) = 0;

    virtual bool canShowOverflowMenu() = 0;
    virtual bool isOverflowMenuShowing() = 0;
    virtual bool isOverflowMenuShowPending() = 0;
    virtual bool showOverflowMenu() = 0;
    virtual bool hideOverflowMenu() = 0;

    virtual void setMenuPrepared() = 0;
    virtual void setMenu(Menu* menu, const MenuPresenter::Callback& cb) = 0;
    virtual void dismissPopupMenus() = 0;

    virtual int getDisplayOptions() = 0;
    virtual void setDisplayOptions(int opts) = 0;

    virtual bool isTitleTruncated() = 0;
    virtual void setCollapsible(bool collapsible) = 0;
    virtual void setHomeButtonEnabled(bool enable) = 0;
    virtual int  getNavigationMode() = 0;
    virtual void setNavigationMode(int mode) = 0;

    virtual void setCustomView(View* view) = 0;
    virtual View* getCustomView() = 0;

    virtual void setNavigationIcon(Drawable* icon) = 0;
    virtual void setNavigationContentDescription(const std::string& description) = 0;
    virtual void setDefaultNavigationIcon(Drawable* icon) = 0;

    virtual void setBackgroundDrawable(Drawable* d) = 0;
    virtual int getHeight() = 0;
    virtual void setVisibility(int visible) = 0;
    virtual int getVisibility() = 0;

    virtual void setMenuCallbacks(const MenuPresenter::Callback& presenterCallback,
                                  const MenuBuilder::Callback& menuBuilderCallback) = 0;
    virtual Menu* getMenu() = 0;
};

}//namespace
#endif
