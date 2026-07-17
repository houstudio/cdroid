/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02101-1301  USA
 *********************************************************************************/
#ifndef __FLOATING_ACTION_MODE_H__
#define __FLOATING_ACTION_MODE_H__
#include <view/actionmode.h>
namespace cdroid{
class Context;
class View;
class MenuBuilder;
class MenuPopupHelper;
class MenuInflater;

class FloatingActionMode : public ActionMode {
public:
    FloatingActionMode(Context* context, View* anchor, const ActionMode::Callback& callback);
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
    Context* mContext;
    View* mAnchor;
    ActionMode::Callback mCallback;
    MenuBuilder* mMenu;
    MenuPopupHelper* mPopup;
    MenuInflater* mInflater;
    std::string mTitle;
    std::string mSubtitle;
    View* mCustomView = nullptr;
    bool mFinished = false;
    std::function<void()> mOnFinished;
};
}//namespace
#endif/*__FLOATING_ACTION_MODE_H__*/
