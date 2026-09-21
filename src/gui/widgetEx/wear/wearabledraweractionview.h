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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0210-1301  USA
*/
#ifndef __WEARABLE_DRAWER_ACTION_VIEW_H__
#define __WEARABLE_DRAWER_ACTION_VIEW_H__
#include <widgetEx/wear/wearabledrawerview.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <menu/menuitem.h>
namespace cdroid{

class Menu;
class TextView;
class ImageView;

/** Ease of use class for creating a Wearable action drawer. Used with WearableDrawerLayout to
 *  create a drawer for users to easily pull up contextual actions, specified through a Menu
 *  (app:actionMenu XML attribute or {@link #getMenu} plus MenuInflater). The full Menu and
 *  MenuItem APIs are not implemented; see WearableActionDrawerMenu.
 *  androidx.wear.widget.drawer.WearableActionDrawerView.java (lines 79-480). */
class WearableActionDrawerView: public WearableDrawerView{
private:
    static constexpr const char* TAG = "WearableActionDrawer";

public:
    WearableActionDrawerView(Context* context);
    WearableActionDrawerView(Context* context, const AttributeSet* attrs);
    WearableActionDrawerView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    WearableActionDrawerView(Context* context, const AttributeSet* attrs,
            int defStyleAttr, int defStyleRes);
    ~WearableActionDrawerView() override;

    void onDrawerOpened() override;

    /** Prevent the window from being swiped closed while it is open by saying that it can scroll
        horizontally. (lines 216-220) */
    bool canScrollHorizontally(int direction) override;

    void onPeekContainerClicked(View& v) override;

    // package-private upstream; WearableDrawerLayout reaches it through the drawer view
    int preferGravity() const override;

    /** Set a {@link MenuItem::OnMenuItemClickListener} for this action drawer. (lines 236-241) */
    void setOnMenuItemClickListener(const MenuItem::OnMenuItemClickListener& listener);

    /** Sets the title for this action drawer. If {@code title} is {@code null} (@Nullable
        upstream, so a pointer here), then the title will be removed. (lines 243-261) */
    void setTitle(const std::string* title);

    // package-private upstream
    bool hasTitle() const;

    // package-private upstream; also invoked for the single-action peek click
    void onMenuItemClicked(int position);

    // package-private upstream; refreshes the peek icon/ellipsis from the first menu item
    void updatePeekIcons();

    /** Returns the Menu object that this WearableActionDrawer represents. Applications should
        use this method to obtain the WearableActionDrawer's Menu object and inflate or add
        content to it as necessary. Lazily creates the menu (and installs this view as its
        listener) on first call. (lines 313-375)
        @return the Menu presented by this view */
    Menu* getMenu();

private:
    /** View holder for the drawer title. (lines 377-384) */
    class TitleViewHolder: public RecyclerView::ViewHolder{
    public:
        TextView* textView;

        explicit TitleViewHolder(View* view);
    };

    /** View holder for an action row. (lines 466-479) */
    class ActionItemViewHolder: public RecyclerView::ViewHolder{
    public:
        View* view;
        ImageView* iconView;
        TextView* textView;

        ActionItemViewHolder(WearableActionDrawerView& outer, View* view);
    };

    /** Binds the Menu into the action list RecyclerView. (lines 386-464) */
    class ActionListAdapter: public RecyclerView::Adapter{
    public:
        static constexpr int TYPE_ACTION = 0;
        static constexpr int TYPE_TITLE = 1;

        // Upstream ignores the menu argument and reads getMenu() (line 406-408)
        explicit ActionListAdapter(WearableActionDrawerView& outer, Menu* /*menu*/);

        int getItemCount() override;

        void onBindViewHolder(RecyclerView::ViewHolder& viewHolder, int position) override;

        RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override;

        int getItemViewType(int position) override;

    private:
        /** The enclosing WearableActionDrawerView (Java inner classes get this implicitly). */
        WearableActionDrawerView& mOuter;
        Menu* mActionMenu;
        View::OnClickListener mItemClickListener;
    };

    friend class ActionListAdapter;
    friend class ActionItemViewHolder;

    RecyclerView* mActionList = nullptr; // created here, owned by the view tree (setDrawerContent)
    int  mTopPadding = 0;
    int  mBottomPadding = 0;
    int  mLeftPadding = 0;
    int  mRightPadding = 0;
    int  mFirstItemTopPadding = 0;
    int  mLastItemBottomPadding = 0;
    int  mIconRightMargin = 0;
    bool mShowOverflowInPeek = false;
    ImageView* mPeekActionIcon = nullptr; // borrowed children of the peek container
    ImageView* mPeekExpandIcon = nullptr;
    ActionListAdapter* mActionListAdapter = nullptr; // owned
    MenuItem::OnMenuItemClickListener mOnMenuItemClickListener;
    Menu* mMenu = nullptr; // owned WearableActionDrawerMenu
    /** @Nullable CharSequence mTitle upstream; std::string has no null, so mHasTitle carries the
        null flag (drives the title row and the list-position offsets). */
    std::string mTitle;
    bool mHasTitle = false;

    void setContentIfFirstCall();
};

}/*endof namespace*/
#endif/*__WEARABLE_DRAWER_ACTION_VIEW_H__*/
