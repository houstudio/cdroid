/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36 com.android.internal.view.FloatingActionMode.
 * CDROID's concrete ActionMode for TYPE_FLOATING. Renders via a FloatingToolbar
 * (ported in widget/floatingtoolbar.*), like AOSP. The toolbar is passed in by the
 * creator (Window, analog of DecorView.createFloatingActionMode).
 *********************************************************************************/
#ifndef __FLOATING_ACTION_MODE_H__
#define __FLOATING_ACTION_MODE_H__
#include <string>
#include <functional>
#include <view/actionmode.h>
namespace cdroid{
class Context;
class View;
class Menu;
class MenuBuilder;
class MenuInflater;
class MenuItem;
class FloatingToolbar;

class FloatingActionMode : public ActionMode {
public:
    FloatingActionMode(Context* context, const ActionMode::Callback& callback,
                       View* originatingView);
    ~FloatingActionMode() override;

    bool show();
    void setTitle(const std::string& title) override;
    void setSubtitle(const std::string& subtitle) override;
    void setCustomView(View* view) override;
    void invalidate() override;
    void finish() override;
    std::string getTitle() override;
    std::string getSubtitle() override;
    View* getCustomView() override;
    Menu* getMenu() override;
    MenuInflater* getMenuInflater() override;

    void setOnFinishedListener(std::function<void()> listener) { mOnFinished = std::move(listener); }
private:
    void onDestroy();
    void invalidateContentRect();

    Context* mContext;
    View* mOriginatingView;        // AOSP: mOriginatingView
    ActionMode::Callback mCallback;
    FloatingToolbar* mFloatingToolbar;   // owned; created from the root view (Window)
    MenuBuilder* mMenu;
    MenuInflater* mInflater = nullptr;
    std::string mTitle;
    std::string mSubtitle;
    View* mCustomView = nullptr;
    bool mFinished = false;
    std::function<void()> mOnFinished;
};
}/*endof namespace*/
#endif/*__FLOATING_ACTION_MODE_H__*/
