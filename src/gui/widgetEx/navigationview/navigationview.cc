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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <widgetEx/navigationview/navigationview.h>
#include <widgetEx/widgetex_styleable.h>
#include <widget/internal_R.h>
#include <menu/menubuilder.h>
#include <menu/menuinflater.h>
#include <menu/menuitem.h>
#include <menu/menuitemimpl.h>
#include <widget/linearlayout.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include <drawable/colorstatelist.h>
#include <drawable/drawable.h>
#include <core/typedarray.h>

namespace cdroid{
using namespace cdroid::internal;

// Presenter-style menu view: a vertical LinearLayout of item rows, rebuilt on
// menu change (the androidx NavigationMenuPresenter's RecyclerView adapter
// simplified to CDROID's LinearLayout substrate; the MenuBuilder data flow and
// item visuals follow material's NavigationMenuItemView).
class NavigationView::NavigationMenuPresenter {
public:
    NavigationView* mOwner;
    LinearLayout* mMenuView = nullptr;
    void initForMenu(Context* context, MenuBuilder* menu) {
        (void)menu;
        if (mMenuView == nullptr) {
            mMenuView = new LinearLayout(context, nullptr, 0);
            mMenuView->setOrientation(LinearLayout::VERTICAL);
        }
    }
    void updateMenuView(bool cleared) {
        (void)cleared;
        mOwner->updateMenuView();
    }
};

DECLARE_WIDGET(NavigationView)

NavigationView::NavigationView(Context* context, const AttributeSet& attrs)
    : NavigationView(context, &attrs, 0) {}

NavigationView::NavigationView(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : FrameLayout(context, attrs, defStyleAttr) {
    init(context, attrs, defStyleAttr);
}

NavigationView::~NavigationView() {
    delete mPresenter;
    delete mMenu;
}

void NavigationView::init(Context* context, const AttributeSet* attrs, int defStyleAttr) {
    (void)defStyleAttr;
    mListener = nullptr;
    mItemBackground = nullptr;
    mItemIconSize = 0;
    mItemHorizontalPadding = 0;
    mItemVerticalPadding = 0;
    mItemIconPadding = 0;
    mItemTextAppearance = 0;
    mItemMaxLines = 1;
    mHeaderView = nullptr;
    mMaxWidth = 0;

    mPresenter = new NavigationMenuPresenter();
    mPresenter->mOwner = this;

    // AOSP NavigationView ctor: create the menu, wire the callback, init the
    // presenter, then read the styleable.
    mMenu = new MenuBuilder(context);
    MenuBuilder::Callback cb;
    cb.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item)->bool{
        return onMenuItemClick(&item);
    };
    mMenu->setCallback(cb);

    // The generated styleable (widgetEx/res attrs -> widgetex_styleable.h) drives
    // every read: attribute ids and indexes come from the same single source the
    // aapt2 build pins, so no attr table lives in program code.
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::NavigationView, defStyleAttr);
    // android:maxWidth is a framework attr; read it by its fw id through the
    // tag AttributeSet (AOSP reads it via the same styleable index).
    mMaxWidth = attrs ? attrs->getAttributeIntValue(std::string(), "maxWidth", 0) : 0;
    mItemIconSize = ta->getDimensionPixelSize(cdroid::internal::R::styleable::NavigationView_itemIconSize, 0);
    mItemIconTint = ta->getColorStateList(cdroid::internal::R::styleable::NavigationView_itemIconTint);
    mItemTextAppearance = ta->getResourceId(cdroid::internal::R::styleable::NavigationView_itemTextAppearance, 0);
    mItemTextColor = ta->getColorStateList(cdroid::internal::R::styleable::NavigationView_itemTextColor);
    const int itemBackgroundRes = ta->getResourceId(cdroid::internal::R::styleable::NavigationView_itemBackground, 0);
    if (itemBackgroundRes) mItemBackground = context->getDrawable(itemBackgroundRes);
    mItemHorizontalPadding = ta->getDimensionPixelSize(cdroid::internal::R::styleable::NavigationView_itemHorizontalPadding, 0);
    mItemVerticalPadding = ta->getDimensionPixelSize(cdroid::internal::R::styleable::NavigationView_itemVerticalPadding, 0);
    mItemIconPadding = ta->getDimensionPixelSize(cdroid::internal::R::styleable::NavigationView_itemIconPadding, 0);
    mItemMaxLines = ta->getInt(cdroid::internal::R::styleable::NavigationView_itemMaxLines, 1);

    mPresenter->initForMenu(context, mMenu);

    // Layout: headers stacked above the menu list (material wraps both in a
    // custom scrim frame; CDROID's FrameLayout children stack vertically here).
    LinearLayout* content = new LinearLayout(context, nullptr, 0);
    content->setOrientation(LinearLayout::VERTICAL);
    mHeaderView = new LinearLayout(context, nullptr, 0);
    mHeaderView->setOrientation(LinearLayout::VERTICAL);
    content->addView(mHeaderView);
    content->addView(mPresenter->mMenuView);
    addView(content, new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_menu)) {
        inflateMenu(ta->getResourceId(cdroid::internal::R::styleable::NavigationView_menu, 0));
    }
    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_headerLayout)) {
        inflateHeaderView(ta->getResourceId(cdroid::internal::R::styleable::NavigationView_headerLayout, 0));
    }
}

void NavigationView::setNavigationItemSelectedListener(OnNavigationItemSelectedListener* listener) {
    mListener = listener;
}

void NavigationView::inflateMenu(int resId) {
    mPresenter->updateMenuView(true);
    MenuInflater inflater(getContext());
    inflater.inflate(resId, mMenu);
    updateMenuView();
}

Menu* NavigationView::getMenu() {
    return mMenu;
}

View* NavigationView::inflateHeaderView(int res) {
    View* view = LayoutInflater::from(getContext())->inflate(res, mHeaderView, false);
    addHeaderView(view);
    return view;
}

void NavigationView::addHeaderView(View* view) {
    if (mHeaderView) mHeaderView->addView(view);
}

void NavigationView::removeHeaderView(View* view) {
    if (mHeaderView) mHeaderView->removeView(view);
}

int NavigationView::getHeaderCount() const{
    return mHeaderView ? mHeaderView->getChildCount() : 0;
}

View* NavigationView::getHeaderView(int index) const{
    return mHeaderView ? mHeaderView->getChildAt(index) : nullptr;
}

const RefPtr<ColorStateList> NavigationView::getItemIconTintList() const {
    return mItemIconTint;
}

void NavigationView::setItemIconTintList(const RefPtr<ColorStateList>& tint) {
    mItemIconTint = tint;
    updateMenuView();
}

const RefPtr<ColorStateList> NavigationView::getItemTextColor() const {
    return mItemTextColor;
}

void NavigationView::setItemTextColor(const RefPtr<ColorStateList>& textColor) {
    mItemTextColor = textColor;
    updateMenuView();
}

Drawable* NavigationView::getItemBackground() const {
    return mItemBackground;
}

void NavigationView::setItemBackgroundResource(int resId) {
    setItemBackground(getContext()->getDrawable(resId));
}

void NavigationView::setItemBackground(Drawable* itemBackground) {
    mItemBackground = itemBackground;
    updateMenuView();
}

int NavigationView::getItemHorizontalPadding() const {
    return mItemHorizontalPadding;
}

void NavigationView::setItemHorizontalPadding(int padding) {
    mItemHorizontalPadding = padding;
    updateMenuView();
}

int NavigationView::getItemVerticalPadding() const {
    return mItemVerticalPadding;
}

void NavigationView::setItemVerticalPadding(int padding) {
    mItemVerticalPadding = padding;
    updateMenuView();
}

int NavigationView::getItemIconPadding() const {
    return mItemIconPadding;
}

void NavigationView::setItemIconPadding(int padding) {
    mItemIconPadding = padding;
    updateMenuView();
}

void NavigationView::setItemIconSize(int iconSize) {
    mItemIconSize = iconSize;
    updateMenuView();
}

void NavigationView::setItemIconSizeResource(int resId) {
    setItemIconSize(getContext()->getDimensionPixelSize(resId));
}

void NavigationView::setItemTextAppearance(int resId) {
    mItemTextAppearance = resId;
    updateMenuView();
}

void NavigationView::setItemMaxLines(int maxLines) {
    mItemMaxLines = maxLines;
    updateMenuView();
}

void NavigationView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // AOSP NavigationView.onMeasure: cap the width at maxWidth when not EXACT.
    switch (MeasureSpec::getMode(widthMeasureSpec)) {
        case MeasureSpec::EXACTLY:
            break;
        case MeasureSpec::AT_MOST:
            widthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    std::min(MeasureSpec::getSize(widthMeasureSpec), mMaxWidth), MeasureSpec::EXACTLY);
            break;
        default:
            widthMeasureSpec = MeasureSpec::makeMeasureSpec(mMaxWidth, MeasureSpec::EXACTLY);
            break;
    }
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

// --- item view factory (material NavigationMenuItemView simplified) ----------

View* NavigationView::createItemView(MenuItem* item) {
    Context* context = getContext();
    LinearLayout* row = new LinearLayout(context, nullptr, 0);
    row->setOrientation(LinearLayout::HORIZONTAL);
    row->setGravity(Gravity::CENTER_VERTICAL);
    if (mItemHorizontalPadding || mItemVerticalPadding) {
        row->setPadding(mItemHorizontalPadding, mItemVerticalPadding,
                        mItemHorizontalPadding, mItemVerticalPadding);
    }
    if (mItemBackground) {
        row->setBackground(mItemBackground->mutate());
    }

    Drawable* icon = item->getIcon();
    if (icon) {
        ImageView* iconView = new ImageView(context, nullptr, 0);
        iconView->setImageDrawable(icon);
        if (mItemIconSize > 0) {
            iconView->setLayoutParams(new LinearLayout::LayoutParams(mItemIconSize, mItemIconSize));
        }
        if (mItemIconTint) {
            iconView->setImageTintList(mItemIconTint);
        }
        row->addView(iconView);
    }

    TextView* textView = new TextView(context, nullptr, 0);
    textView->setText(item->getTitle());
    if (mItemTextAppearance) {
        textView->setTextAppearance(mItemTextAppearance);
    } else if (mItemTextColor) {
        textView->setTextColor(mItemTextColor);
    }
    textView->setMaxLines(mItemMaxLines);
    LinearLayout::LayoutParams* lp = new LinearLayout::LayoutParams(
            0, ViewGroup::LayoutParams::WRAP_CONTENT);
    lp->weight = 1;
    if (mItemIconPadding) lp->leftMargin = mItemIconPadding;
    row->addView(textView, lp);

    row->setOnClickListener([this, item](View&) {
        onMenuItemClick(item);
    });
    return row;
}

bool NavigationView::onMenuItemClick(MenuItem* item) {
    if (dynamic_cast<MenuItemImpl*>(item)) {
        ((MenuItemImpl*)item)->invoke();
    }
    return mListener && mListener->onNavigationItemSelected(item);
}

void NavigationView::updateMenuView() {
    if (mPresenter->mMenuView == nullptr) return;
    mPresenter->mMenuView->removeAllViews();
    const std::vector<MenuItemImpl*>& items = mMenu->getVisibleItems();
    for (MenuItemImpl* item : items) {
        if (item->hasSubMenu()) {
            // material renders submenus as indented children on click; CDROID
            // expands one level inline (simplified sub-menu presentation).
            TextView* subHeader = new TextView(getContext(), nullptr, 0);
            subHeader->setText(item->getTitle());
            if (mItemTextColor) subHeader->setTextColor(mItemTextColor);
            mPresenter->mMenuView->addView(subHeader);
            continue;
        }
        mPresenter->mMenuView->addView(createItemView(item));
    }
}

}//namespace cdroid
