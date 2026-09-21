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
#include <widgetEx/wear/wearabledraweractionview.h>
#include <widgetEx/wear/wearabledraweractionmenu.h>
#include <widgetEx/widgetex_styleable.h>
#include <widgetEx/wear/resourcesutil.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widget/imageview.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/layoutinflater.h>
#include <menu/menuinflater.h>
#include <porting/cdlog.h>
#include <memory>

namespace cdroid{
using namespace cdroid::internal;

// androidx.wear.widget.drawer.WearableActionDrawerView.java (lines 79-480)
DECLARE_WIDGET2(WearableActionDrawerView, "androidx.wear.widget.drawer.WearableActionDrawerView");

WearableActionDrawerView::WearableActionDrawerView(Context* context)
    : WearableActionDrawerView(context, nullptr) {
}

WearableActionDrawerView::WearableActionDrawerView(Context* context, const AttributeSet* attrs)
    : WearableActionDrawerView(context, attrs, 0) {
}

WearableActionDrawerView::WearableActionDrawerView(Context* context, const AttributeSet* attrs,
        int defStyleAttr)
    : WearableActionDrawerView(context, attrs, defStyleAttr, 0) {
}

WearableActionDrawerView::WearableActionDrawerView(Context* context, const AttributeSet* attrs,
        int defStyleAttr, int defStyleRes)
    : WearableDrawerView(context, attrs, defStyleAttr, defStyleRes) {

    setLockedWhenClosed(true);

    bool showOverflowInPeek = false;
    int menuRes = 0;
    if (attrs != nullptr) {
        auto typedArray = context->obtainStyledAttributes(attrs,
                internal::R::styleable::WearableActionDrawerView, defStyleAttr, 0 /* defStyleRes */);
        // ViewCompat.saveAttributeDataForStyleable (upstream line 123) is an androidx debugging
        // aid with no CDROID counterpart.

        mTitle = typedArray->getString(
                internal::R::styleable::WearableActionDrawerView_drawerTitle);
        mHasTitle = !mTitle.empty(); // @Nullable CharSequence upstream: an unset read is null
        showOverflowInPeek = typedArray->getBoolean(
                internal::R::styleable::WearableActionDrawerView_showOverflowInPeek, false);
        menuRes = (int) typedArray->getResourceId(
                internal::R::styleable::WearableActionDrawerView_actionMenu, 0);
    }

    // Upstream fetches AccessibilityManager through Context.ACCESSIBILITY_SERVICE; CDROID
    // exposes it as a singleton (same adaptation as WearableDrawerLayout).
    mShowOverflowInPeek = showOverflowInPeek
            || AccessibilityManager::getInstance(context).isEnabled();

    if (!mShowOverflowInPeek) {
        View* peekView = LayoutInflater::from(context)->inflate(
                (int) internal::R::layout::ws_action_drawer_peek_view,
                getPeekContainer(), false /* attachToRoot */);
        setPeekContent(peekView);
        mPeekActionIcon = (ImageView*) peekView->findViewById(
                (int) internal::R::id::ws_action_drawer_peek_action_icon);
        mPeekExpandIcon = (ImageView*) peekView->findViewById(
                (int) internal::R::id::ws_action_drawer_expand_icon);
    } else {
        mPeekActionIcon = nullptr;
        mPeekExpandIcon = nullptr;
        getPeekContainer()->setContentDescription(
                context->getString((int) internal::R::string::ws_action_drawer_content_description));
    }

    if (menuRes != 0) {
        // This must occur after initializing mPeekActionIcon, otherwise updatePeekIcons will
        // exit early.
        MenuInflater menuInflater(context);
        menuInflater.inflate(menuRes, getMenu());
    }

    const int screenWidthPx = ResourcesUtil::getScreenWidthPx(*context);
    const int screenHeightPx = ResourcesUtil::getScreenHeightPx(*context);

    Resources& res = context->getResources();
    mTopPadding = res.getDimensionPixelOffset(
            (int) internal::R::dimen::ws_action_drawer_item_top_padding);
    mBottomPadding = res.getDimensionPixelOffset(
            (int) internal::R::dimen::ws_action_drawer_item_bottom_padding);
    // The ws_action_drawer_* fraction resources are not pinned in widgetex_styleable.h, so their
    // ids are resolved by name through the arsc (upstream references R.fraction directly).
    mLeftPadding = ResourcesUtil::getFractionOfScreenPx(*context, screenWidthPx,
            res.getIdentifier("ws_action_drawer_item_left_padding", "fraction", ""));
    mRightPadding = ResourcesUtil::getFractionOfScreenPx(*context, screenWidthPx,
            res.getIdentifier("ws_action_drawer_item_right_padding", "fraction", ""));

    mFirstItemTopPadding = ResourcesUtil::getFractionOfScreenPx(*context, screenHeightPx,
            res.getIdentifier("ws_action_drawer_item_first_item_top_padding", "fraction", ""));
    mLastItemBottomPadding = ResourcesUtil::getFractionOfScreenPx(*context, screenHeightPx,
            res.getIdentifier("ws_action_drawer_item_last_item_bottom_padding", "fraction", ""));

    mIconRightMargin = res.getDimensionPixelOffset(
            (int) internal::R::dimen::ws_action_drawer_item_icon_right_margin);

    mActionList = new RecyclerView(context);
    mActionList->setId((int) internal::R::id::action_list);
    mActionList->setLayoutManager(std::make_unique<LinearLayoutManager>(context));
    mActionListAdapter = new ActionListAdapter(*this, getMenu());
    // Do not bind the mActionListAdapter to the action list here. We will bind it when/if the
    // drawer is first opened to avoid the inflation cost in the case that the drawer is never
    // used
    setDrawerContent(mActionList);
}

WearableActionDrawerView::~WearableActionDrawerView() {
    // Upstream relies on GC. CDROID: the adapter and the menu are owned here. updatePeekIcons
    // may have detached mActionList (setDrawerContent(nullptr) removes without deleting), so
    // free it only when no parent owns it.
    delete mActionListAdapter;
    delete mMenu;
    if (mActionList != nullptr && mActionList->getParent() == nullptr) {
        delete mActionList;
    }
}

void WearableActionDrawerView::onDrawerOpened() {
    setContentIfFirstCall();
    if (mActionListAdapter->getItemCount() > 0) {
        RecyclerView::ViewHolder* holder = mActionList->findViewHolderForAdapterPosition(0);
        if (holder != nullptr && holder->itemView != nullptr) {
            holder->itemView->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_FOCUSED);
        }
    }
}

void WearableActionDrawerView::setContentIfFirstCall() {
    if (mActionList->getAdapter() == nullptr) {
        mActionList->setAdapter(mActionListAdapter);
    }
}

bool WearableActionDrawerView::canScrollHorizontally(int direction) {
    // Prevent the window from being swiped closed while it is open by saying that it can scroll
    // horizontally.
    return isOpened();
}

void WearableActionDrawerView::onPeekContainerClicked(View& v) {
    if (mShowOverflowInPeek) {
        WearableDrawerView::onPeekContainerClicked(v);
    } else {
        onMenuItemClicked(0);
    }
}

int WearableActionDrawerView::preferGravity() const {
    return Gravity::BOTTOM;
}

void WearableActionDrawerView::setOnMenuItemClickListener(
        const MenuItem::OnMenuItemClickListener& listener) {
    mOnMenuItemClickListener = listener;
}

void WearableActionDrawerView::setTitle(const std::string* title) {
    // TextUtils.equals(title, mTitle): null equals null, else content equality (lines 248-250).
    const bool equals = (title == nullptr) ? !mHasTitle : (mHasTitle && *title == mTitle);
    if (equals) {
        return;
    }

    const bool hadTitle = mHasTitle;
    mHasTitle = title != nullptr;
    mTitle = title != nullptr ? *title : std::string();
    if (!hadTitle) {
        mActionListAdapter->notifyItemInserted(0);
    } else if (title == nullptr) {
        mActionListAdapter->notifyItemRemoved(0);
    } else {
        mActionListAdapter->notifyItemChanged(0);
    }
}

bool WearableActionDrawerView::hasTitle() const {
    return mHasTitle;
}

void WearableActionDrawerView::onMenuItemClicked(int position) {
    if (position >= 0 && position < getMenu()->size()) { // Sanity check.
        // The unchecked cast upstream (line 270); this menu only stores
        // WearableActionDrawerMenuItems.
        auto* menuItem = static_cast<WearableActionDrawerMenu::WearableActionDrawerMenuItem*>(
                getMenu()->getItem(position));
        if (menuItem->invoke()) {
            return;
        }

        if (mOnMenuItemClickListener) {
            mOnMenuItemClickListener(*menuItem);
        }
    }
}

void WearableActionDrawerView::updatePeekIcons() {
    if (mPeekActionIcon == nullptr || mPeekExpandIcon == nullptr) {
        return;
    }

    Menu* menu = getMenu();
    const int numberOfActions = menu->size();

    // Only show drawer content (and allow it to be opened) when there's more than one action.
    if (numberOfActions > 1) {
        setDrawerContent(mActionList);
        mPeekExpandIcon->setVisibility(View::VISIBLE);
    } else {
        setDrawerContent(nullptr);
        mPeekExpandIcon->setVisibility(View::GONE);
    }

    if (numberOfActions >= 1) {
        Drawable* firstActionDrawable = menu->getItem(0)->getIcon();
        // Because the ImageView will tint the Drawable white, attempt to get a mutable copy of
        // it. If a copy isn't made, the icon will be white in the expanded state, rendering it
        // invisible.
        if (firstActionDrawable != nullptr) {
            firstActionDrawable = firstActionDrawable->getConstantState()->newDrawable()->mutate();
            firstActionDrawable->clearColorFilter();
        }

        mPeekActionIcon->setImageDrawable(firstActionDrawable);
        mPeekActionIcon->setContentDescription(menu->getItem(0)->getTitle());
    }
}

Menu* WearableActionDrawerView::getMenu() {
    if (mMenu == nullptr) {
        // The anonymous WearableActionDrawerMenuListener (lines 325-371)
        WearableActionDrawerMenu::WearableActionDrawerMenuListener listener;
        listener.menuItemChanged = [this](int position) {
            if (mActionListAdapter != nullptr) {
                const int listPosition = hasTitle() ? position + 1 : position;
                mActionListAdapter->notifyItemChanged(listPosition);
            }
            if (position == 0) {
                updatePeekIcons();
            }
        };

        listener.menuItemAdded = [this](int position) {
            if (mActionListAdapter != nullptr) {
                const int listPosition = hasTitle() ? position + 1 : position;
                mActionListAdapter->notifyItemInserted(listPosition);
            }
            // Handle transitioning from 0->1 items (set peek icon) and
            // 1->2 (switch to ellipsis.)
            if (position <= 1) {
                updatePeekIcons();
            }
        };

        listener.menuItemRemoved = [this](int position) {
            if (mActionListAdapter != nullptr) {
                const int listPosition = hasTitle() ? position + 1 : position;
                mActionListAdapter->notifyItemRemoved(listPosition);
            }
            // Handle transitioning from 2->1 items (remove ellipsis), and
            // also the removal of item 1, which could cause the peek icon
            // to change.
            if (position <= 1) {
                updatePeekIcons();
            }
        };

        listener.menuChanged = [this]() {
            if (mActionListAdapter != nullptr) {
                mActionListAdapter->notifyDataSetChanged();
            }
            updatePeekIcons();
        };

        mMenu = new WearableActionDrawerMenu(getContext(), listener);
    }

    return mMenu;
}

// TitleViewHolder (lines 377-384)

WearableActionDrawerView::TitleViewHolder::TitleViewHolder(View* view)
    : RecyclerView::ViewHolder(view),
      textView((TextView*) view->findViewById((int) internal::R::id::ws_action_drawer_title)) {
}

// ActionListAdapter (lines 386-464)

WearableActionDrawerView::ActionListAdapter::ActionListAdapter(WearableActionDrawerView& outer,
        Menu* /*menu*/)
    : mOuter(outer),
      mActionMenu(outer.getMenu()), // upstream ignores the ctor arg and reads getMenu() (line 407)
      mItemClickListener([this](View& v) {
          const int childPos = mOuter.mActionList->getChildAdapterPosition(&v)
                  - (mOuter.hasTitle() ? 1 : 0);
          if (childPos == RecyclerView::NO_POSITION) {
              LOGW("%s: invalid child position", WearableActionDrawerView::TAG);
              return;
          }
          mOuter.onMenuItemClicked(childPos);
      }) {
}

int WearableActionDrawerView::ActionListAdapter::getItemCount() {
    return mActionMenu->size() + (mOuter.hasTitle() ? 1 : 0);
}

void WearableActionDrawerView::ActionListAdapter::onBindViewHolder(
        RecyclerView::ViewHolder& viewHolder, int position) {
    const int titleAwarePosition = mOuter.hasTitle() ? position - 1 : position;
    if (dynamic_cast<ActionItemViewHolder*>(&viewHolder) != nullptr) {
        auto* holder = static_cast<ActionItemViewHolder*>(&viewHolder);
        holder->view->setPadding(
                mOuter.mLeftPadding,
                position == 0 ? mOuter.mFirstItemTopPadding : mOuter.mTopPadding,
                mOuter.mRightPadding,
                position == getItemCount() - 1 ? mOuter.mLastItemBottomPadding
                                               : mOuter.mBottomPadding);

        Drawable* icon = mActionMenu->getItem(titleAwarePosition)->getIcon();
        if (icon != nullptr) {
            icon = icon->getConstantState()->newDrawable()->mutate();
        }
        const std::string title = mActionMenu->getItem(titleAwarePosition)->getTitle();
        holder->textView->setText(title);
        holder->textView->setContentDescription(title);
        holder->iconView->setImageDrawable(icon);
    } else if (dynamic_cast<TitleViewHolder*>(&viewHolder) != nullptr) {
        auto* holder = static_cast<TitleViewHolder*>(&viewHolder);
        holder->textView->setPadding(0, mOuter.mFirstItemTopPadding, 0, mOuter.mBottomPadding);
        holder->textView->setText(mOuter.mTitle);
    }
}

RecyclerView::ViewHolder* WearableActionDrawerView::ActionListAdapter::onCreateViewHolder(
        ViewGroup* parent, int viewType) {
    switch (viewType) {
        case TYPE_TITLE: {
            View* titleView = LayoutInflater::from(parent->getContext())->inflate(
                    (int) internal::R::layout::ws_action_drawer_title_view, parent, false);
            return new TitleViewHolder(titleView);
        }

        case TYPE_ACTION:
        default: {
            View* actionView = LayoutInflater::from(parent->getContext())->inflate(
                    (int) internal::R::layout::ws_action_drawer_item_view, parent, false);
            actionView->setOnClickListener(mItemClickListener);
            return new ActionItemViewHolder(mOuter, actionView);
        }
    }
}

int WearableActionDrawerView::ActionListAdapter::getItemViewType(int position) {
    return mOuter.hasTitle() && position == 0 ? TYPE_TITLE : TYPE_ACTION;
}

// ActionItemViewHolder (lines 466-479)

WearableActionDrawerView::ActionItemViewHolder::ActionItemViewHolder(WearableActionDrawerView& outer,
        View* view)
    : RecyclerView::ViewHolder(view),
      view(view),
      iconView((ImageView*) view->findViewById((int) internal::R::id::ws_action_drawer_item_icon)),
      textView((TextView*) view->findViewById((int) internal::R::id::ws_action_drawer_item_text)) {
    ((LinearLayout::LayoutParams*) iconView->getLayoutParams())
            ->setMarginEnd(outer.mIconRightMargin);
}

}/*endof namespace*/
