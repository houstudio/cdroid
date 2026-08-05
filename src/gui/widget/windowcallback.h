#ifndef __WINDOWCALLBACK_H__
#define __WINDOWCALLBACK_H__
/*********************************************************************************
 * Faithful partial port of android.view.Window.Callback.
 *
 * androidx's ToolbarActionBar / ToolbarWidgetWrapper hold a `Window.Callback` and dispatch the
 * options panel / home-up through it. CDROID's Window plays the Activity (and thus Window.Callback)
 * role, so those consumers hold a WindowCallback* exactly as they hold a Window.Callback upstream.
 *
 * Only the panel / options-menu subset of android.view.Window.Callback is ported here. The
 * input-dispatch (dispatchKey/Touch/Motion...), window-lifecycle, action-mode and search callbacks
 * are intentionally omitted: CDROID's Window already handles input dispatch via its View/ViewGroup
 * overrides and owns its own ActionMode, so re-declaring them would be redundant. Add a method if a
 * future consumer needs it.
 *********************************************************************************/
namespace cdroid{
class Menu;
class MenuItem;
class View;

class WindowCallback{
public:
    virtual ~WindowCallback() = default;

    // Instantiate the view to display in the panel for 'featureId' (null = use the standard menu).
    virtual View* onCreatePanelView(int featureId) = 0;
    // Initialize the contents of the menu for panel 'featureId' (called once, first show).
    virtual bool onCreatePanelMenu(int featureId, Menu& menu) = 0;
    // Prepare a panel right before it is shown (called every show).
    virtual bool onPreparePanel(int featureId, View* view, Menu& menu) = 0;
    // A panel's menu was opened by the user.
    virtual bool onMenuOpened(int featureId, Menu& menu) = 0;
    // A menu item was selected — this is the home/up entry point (featureId == FEATURE_OPTIONS_PANEL).
    virtual bool onMenuItemSelected(int featureId, MenuItem& item) = 0;
    // A panel was closed.
    virtual void onPanelClosed(int featureId, Menu& menu) = 0;
};

}//namespace
#endif
