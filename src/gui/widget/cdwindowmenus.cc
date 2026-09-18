/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Window's menu slice, split out of cdwindow.cc along the AOSP class boundaries: the
// options-menu/context-menu dispatch is android.app.Activity's delegation through
// Window.Callback, and the panel plumbing mirrors PhoneWindow/PhoneWindowMenuCallback.
// Definitions moved verbatim — no signature or behavior change.
#include <widget/cdwindow.h>
#include <widget/toolbar.h>
#include <widget/toolbaractionbar.h>
#include <widget/internal_R.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/menuinflater.h>
#include <menu/contextmenubuilder.h>
#include <menu/contextmenu.h>
#include <menu/menudialoghelper.h>
#include <view/floatingactionmode.h>
#include <porting/cdlog.h>

namespace cdroid {
using namespace cdroid::internal;

void Window::setActionBar(Toolbar* toolbar){
    delete mActionBar;
    // CDROID's Activity plays the AppCompatActivity role: adopting a Toolbar builds a
    // ToolbarActionBar that bridges it (mirrors androidx AppCompatDelegateImpl +
    // framework Activity.setActionBar).
    mActionBar = toolbar ? new ToolbarActionBar(toolbar, getText(), this) : nullptr;
    if(mActionBar) mActionBar->invalidateOptionsMenu();
}

ActionBar* Window::getActionBar(){
    return mActionBar;
}

bool Window::onCreateOptionsMenu(Menu& /*menu*/){
    return true;
}

bool Window::onPrepareOptionsMenu(Menu& /*menu*/){
    return true;
}

bool Window::onOptionsItemSelected(MenuItem& /*item*/){
    // Non-home options items reach FragmentActivity's override (which dispatches to Fragments).
    // Home/up is folded to onNavigateUp() upstream in onMenuItemSelected (mirrors AOSP
    // Activity.onMenuItemSelected for FEATURE_OPTIONS_PANEL), so it never arrives here.
    return false;
}

bool Window::onContextItemSelected(MenuItem& /*item*/){
    return false;
}

bool Window::onNavigateUp(){
    // CDROID has no manifest parentActivityIntent; the default Up behavior finishes the
    // activity (mirrors androidx Activity.onNavigateUp -> finish when no parent). Override
    // in subclasses (e.g. NavController-driven hosts) for custom Up handling.
    close();
    return true;
}

void Window::invalidateOptionsMenu(){
    if(mActionBar) mActionBar->invalidateOptionsMenu();
}

MenuInflater* Window::getMenuInflater(){
    if(!mMenuInflater) mMenuInflater = new MenuInflater(getContext());
    return mMenuInflater;
}

void Window::openOptionsMenu(){
    if(mActionBar) mActionBar->openOptionsMenu();
}

void Window::closeOptionsMenu(){
    if(mActionBar) mActionBar->closeOptionsMenu();
}

// --- WindowCallback (android.view.Window.Callback, panel/options subset) ---
// CDROID honours a single options panel (FEATURE_OPTIONS_PANEL); other feature ids are no-ops.
View* Window::onCreatePanelView(int /*featureId*/){
    return nullptr; // no custom panel view -> standard options menu
}

bool Window::onCreatePanelMenu(int featureId, Menu& menu){
    return (featureId == FEATURE_OPTIONS_PANEL) ? onCreateOptionsMenu(menu) : false;
}

bool Window::onPreparePanel(int featureId, View* /*view*/, Menu& menu){
    return (featureId == FEATURE_OPTIONS_PANEL) ? onPrepareOptionsMenu(menu) : true;
}

bool Window::onMenuOpened(int /*featureId*/, Menu& /*menu*/){
    return true;
}

bool Window::onMenuItemSelected(int featureId, MenuItem& item){
    // Home -> Up fold. AOSP does this in Activity.onMenuItemSelected for FEATURE_OPTIONS_PANEL;
    // CDROID folds it here (the Window.Callback entry point ToolbarActionBar dispatches through).
    if(featureId == FEATURE_OPTIONS_PANEL && item.getItemId() == R::id::home && mActionBar &&
       (mActionBar->getDisplayOptions() & ActionBar::DISPLAY_HOME_AS_UP)){
        return onNavigateUp();
    }
    return onOptionsItemSelected(item);
}

void Window::onPanelClosed(int /*featureId*/, Menu& /*menu*/){
    // No PhoneWindow panel state machine beyond the toolbar popup; nothing to do here.
}

// =====================================================================================
//  Context menu
// =====================================================================================
bool Window::showContextMenuForChild(View* originalView){
    if(originalView == nullptr) return false;
    ContextMenuBuilder* builder = new ContextMenuBuilder(getContext());
    MenuBuilder::Callback cb;
    cb.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item)->bool{
        return onContextItemSelected(item);
    };
    builder->setCallback(cb);
    // showDialog builds the menu via originalView.createContextMenu (which invokes the
    // OnCreateContextMenuListener registered by registerForContextMenu -> onCreateContextMenu)
    // and presents it as a dialog; item selection routes back through the callback above.
    MenuDialogHelper* helper = builder->showDialog(originalView);
    return helper != nullptr;
}

bool Window::showContextMenuForChild(View* originalView, float /*x*/, float /*y*/){
    // Anchored variant: CDROID shows the context menu as a centered AlertDialog, so the
    // touch coordinates are not used (no floating popup anchored to (x,y) here).
    return showContextMenuForChild(originalView);
}

void Window::registerForContextMenu(View* view){
    if(!view) return;
    view->setOnCreateContextMenuListener(
        [this](ContextMenu& menu, View& v, ContextMenuInfo* info){ onCreateContextMenu(menu, v, info); });
}

void Window::unregisterForContextMenu(View* view){
    if(view) view->setOnCreateContextMenuListener(View::OnCreateContextMenuListener{});
}

void Window::openContextMenu(View* view){
    if(view) view->showContextMenu();
}

void Window::onCreateContextMenu(ContextMenu&, View&, ContextMenuInfo*){}

void Window::closeContextMenu(){
    // CDROID shows the context menu as a self-dismissing AlertDialog via MenuDialogHelper;
    // there is no window panel to close programmatically (no FEATURE_CONTEXT_MENU).
}

// =====================================================================================
//  ActionMode (DecorView)
// =====================================================================================
ActionMode* Window::startActionModeForChild(View* originalView, const ActionMode::Callback& callback, int type){
    return startActionModeInternal(originalView, callback, type);
}

ActionMode* Window::startActionModeInternal(View* originatingView, const ActionMode::Callback& callback, int type){
    if (mActionMode != nullptr) {
        ActionMode* prev = mActionMode;
        mActionMode = nullptr;
        prev->finish();
    }

    // DecorView analog: FloatingActionMode creates its own FloatingToolbar from the root view.
    FloatingActionMode* mode = new FloatingActionMode(getContext(), callback, originatingView);
    mode->setType(type);
    mode->setOnFinishedListener([this, mode]() {
        mActionMode = nullptr;
        post(Runnable([mode] { delete mode; }));
    });
    if (!mode->show()) {
        delete mode;
        return nullptr;
    }
    mActionMode = mode;
    return mode;
}

void Window::playSoundImpl(int effectId){
    LOGD("%d",effectId);
}

}  //endof namespace
