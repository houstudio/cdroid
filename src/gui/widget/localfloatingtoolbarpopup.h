/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36
 *   com.android.internal.widget.floatingtoolbar.LocalFloatingToolbarPopup.
 * A popup window used by the floating toolbar to render menu items in the local
 * app process. Holds 2 panels (main + overflow) and an overflow button to
 * transition between panels.
 *
 * Nested classes mirror AOSP exactly (OverflowPanel / OverflowPanelViewHelper /
 * MenuItemRepr / LogAccelerateInterpolator) to avoid namespace pollution.
 *
 * Phase 3a: PopupWindow + content container + main panel + positioning core
 *   (refreshCoordinatesAndOverflowDirection) + show/dismiss/hide/updateCoordinates
 *   + layoutMainPanelItems + MenuItemRepr.
 * Phase 3b: overflow panel (custom ListView) + overflow button + adapter +
 *   layoutOverflowPanelItems + sizing helpers + full setPanelsStatesAtRestingPosition
 *   (positions the content container at (margin, margin) -> symmetric margins) +
 *   open/close overflow (instant toggle + AVD morph; the width/height/button tween
 *   is Stage 3b-anim, deferred).
 *   TODO(Phase 3b-anim): open/close overflow width/height/button Animation tweens,
 *     show/dismiss/hide alpha AnimatorSet, LogAccelerateInterpolator wiring.
 *   TODO(Phase 3c): touchable-region insets computer + app-bounds clamp + theme tint.
 *********************************************************************************/
#ifndef __LOCAL_FLOATING_TOOLBAR_POPUP_H__
#define __LOCAL_FLOATING_TOOLBAR_POPUP_H__
#include <widget/floatingtoolbarpopup.h>
#include <widget/popupwindow.h>
#include <widget/listview.h>
#include <widget/adapter.h>
#include <widget/imagebutton.h>
#include <core/rect.h>
#include <menu/menuitem.h>
#include <map>
#include <string>
#include <vector>
namespace cdroid{
class Context;
class View;
class ViewGroup;
class LayoutInflater;
class MenuItem;

class LocalFloatingToolbarPopup : public FloatingToolbarPopup {
public:
    /** Represents the identity of a MenuItem rendered in a FloatingToolbarPopup.
     *  Ported from AOSP public static final class MenuItemRepr. */
    class MenuItemRepr {
    public:
        int itemId = 0;
        int groupId = 0;
        std::string title;
        bool hasIcon = false;   // Drawable not compared by value; presence flag is enough
        static MenuItemRepr of(MenuItem* menuItem);
        bool operator<(const MenuItemRepr& o) const;   // for std::map key
        bool equals(const MenuItemRepr& o) const;
        static bool reprEquals(const std::vector<MenuItem*>& a, const std::vector<MenuItem*>& b);
    };

    LocalFloatingToolbarPopup(Context* context, View* parent);
    ~LocalFloatingToolbarPopup() override;

    bool setOutsideTouchable(bool outsideTouchable, const std::function<void()>& onDismiss) override;
    void setWidthChanged(bool widthChanged) override;
    void setSuggestedWidth(int suggestedWidth) override;
    void show(const std::vector<MenuItem*>& menuItems,
              const MenuItem::OnMenuItemClickListener& menuItemClickListener,
              const Rect& contentRect) override;
    void dismiss() override;
    void hide() override;
    bool isShowing() override;
    bool isHidden() override;

private:
    /* Minimum and maximum number of items allowed in the overflow. */
    static constexpr int MIN_OVERFLOW_SIZE = 2;
    static constexpr int MAX_OVERFLOW_SIZE = 4;

    // Forward declarations of nested classes (defined at the bottom of this class).
    class OverflowPanel;
    class OverflowItemAdapter;
    class OverflowPanelViewHelper;

    void show(const Rect& contentRectOnScreen);
    void updateCoordinates(const Rect& contentRectOnScreen);
    void refreshCoordinatesAndOverflowDirection(const Rect& contentRectOnScreen);
    void refreshViewPort();
    bool hasOverflow() const { return mHasOverflow; }
    int calculateOverflowHeight(int maxItemSize) const;
    void updateOverflowHeight(int suggestedHeight);
    void updatePopupSize();
    int getAdjustedToolbarWidth(int suggestedWidth) const;
    int getOverflowWidth() const;

    void layoutMenuItems(const std::vector<MenuItem*>& menuItems,
                         const MenuItem::OnMenuItemClickListener& menuItemClickListener,
                         int suggestedWidth);
    void updateMenuItems(const std::vector<MenuItem*>& menuItems,
                         const MenuItem::OnMenuItemClickListener& menuItemClickListener);
    bool isLayoutRequired(const std::vector<MenuItem*>& menuItems) const;
    std::vector<MenuItem*> layoutMainPanelItems(const std::vector<MenuItem*>& menuItems,
                                                int toolbarWidth);
    void layoutOverflowPanelItems(const std::vector<MenuItem*>& menuItems);
    void preparePopupContent();
    void clearPanels();
    void setPanelsStatesAtRestingPosition();
    void setContentAreaAsTouchableSurface();
    void setZeroTouchableSurface();
    void setTouchableSurfaceInsetsComputer();
    void positionContentYCoordinatesIfOpeningOverflowUpwards();
    bool isInRTLMode() const;

    void openOverflow();    // Phase 3b: instant; TODO(3b-anim): width/height/button tween.
    void closeOverflow();
    bool isOverflowAnimating() const;   // TODO(3b-anim): tracks open/close tween state.
    void maybeComputeTransitionDurationScale();
    int getAdjustedDuration(int originalDuration) const;

    ImageButton* createOverflowButton();
    OverflowPanel* createOverflowPanel();
    ViewGroup* createMainPanel();
    static ViewGroup* createContentContainer(Context* context);
    static PopupWindow* createPopupWindow(ViewGroup* content);
    static Size measure(View* view);
    static void setSize(View* view, int width, int height);
    static void setSize(View* view, const Size& size);
    static void setWidth(View* view, int width);
    static void setHeight(View* view, int height);
    static View* createMenuItemButton(Context* context, MenuItem* menuItem,
                                      int iconTextSpacing, bool showIcon);
    static void updateMenuItemButton(View* button, MenuItem* menuItem,
                                     int iconTextSpacing, bool showIcon);
    static Context* applyDefaultTheme(Context* context);

    // Animation helpers — TODO(3b-anim): currently no-op stubs.
    void runShowAnimation();
    void runDismissAnimation();
    void runHideAnimation();
    void cancelDismissAndHideAnimations();
    void cancelOverflowAnimations();

    Context* mContext;
    View* mParent;
    PopupWindow* mPopupWindow;

    int mMarginHorizontal;
    int mMarginVertical;
    int mLineHeight;
    int mIconTextSpacing;

    ViewGroup* mContentContainer;   // holds all contents
    ViewGroup* mMainPanel;          // holds menu items initially displayed
    OverflowPanel* mOverflowPanel;          // holds menu items hidden in the overflow
    ImageButton* mOverflowButton;           // opens/closes the overflow
    OverflowItemAdapter* mOverflowPanelAdapter;     // owned; set on mOverflowPanel
    OverflowPanelViewHelper* mOverflowPanelViewHelper;

    /* Overflow-button icon drawables are NOT cached as members here. Unlike AOSP
     * (where ImageView borrows drawables), CDROID's ImageView takes ownership and
     * deletes the drawable on replace/destroy. So each setImageDrawable call below
     * fetches a fresh instance from the context (Assets caches the ConstantState and
     * hands out new instances). See createOverflowButton / setPanelsStatesAtRestingPosition.
     *   mArrow    == ft_avd_tooverflow  (back arrow; shown when overflow is open)
     *   mOverflow == ft_avd_toarrow     (more icon;   shown when overflow is closed)
     *   mToArrow / mToOverflow == the AVD morphs (TODO 3b-anim). */

    Rect mViewPortOnScreen;
    int mCoordsOnWindow[2];
    int mTmpCoords[2];

    bool mDismissed = true;
    bool mHidden = false;

    Size mOverflowButtonSize = Size::MakeEmpty();
    Size mOverflowPanelSize = Size::MakeEmpty();    // empty => no overflow
    Size mMainPanelSize = Size::MakeEmpty();
    bool mHasOverflow = false;

    std::map<MenuItemRepr, MenuItem*> mMenuItems;
    MenuItem::OnMenuItemClickListener mOnMenuItemClickListener;
    View::OnClickListener mMenuItemButtonOnClickListener;
    AdapterView::OnItemClickListener mOverflowPanelItemClickListener;

    bool mOpenOverflowUpwards = false;
    bool mIsOverflowOpen = false;
    int mTransitionDurationScale = 0;

    Rect mPreviousContentRect;
    int mSuggestedWidth = 0;
    bool mWidthChanged = true;

    // =================================================================================
    //  Nested classes (mirror AOSP static inner classes)
    // =================================================================================

    /** A custom ListView for the overflow panel. Ported from AOSP OverflowPanel. */
    class OverflowPanel : public ListView {
    public:
        using ListView::awakenScrollBars;   // expose protected base (AOSP overrides to expose)
        explicit OverflowPanel(LocalFloatingToolbarPopup* popup);
    protected:
        void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    private:
        LocalFloatingToolbarPopup* mPopup;
    };

    /** Adapter backing the overflow ListView. AOSP uses an anonymous
     *  ArrayAdapter<MenuItem> with an overridden getView that delegates to
     *  OverflowPanelViewHelper; this nested class is the C++ equivalent. */
    class OverflowItemAdapter : public Adapter {
    public:
        explicit OverflowItemAdapter(LocalFloatingToolbarPopup* popup) : mPopup(popup) {}
        int getCount() const override;
        void* getItem(int position) const override;
        long getItemId(int position) const override;
        View* getView(int position, View* convertView, ViewGroup* parent) override;
        bool areAllItemsEnabled() const override { return true; }
        bool isEnabled(int /*position*/) const override { return true; }
        void add(MenuItem* menuItem);
        void clear();
    private:
        LocalFloatingToolbarPopup* mPopup;
        std::vector<MenuItem*> mItems;
    };

    /** A helper for generating views for the overflow panel.
     *  Ported from AOSP OverflowPanelViewHelper. */
    class OverflowPanelViewHelper {
    public:
        OverflowPanelViewHelper(Context* context, int iconTextSpacing);
        ~OverflowPanelViewHelper();
        View* getView(MenuItem* menuItem, int minimumWidth, View* convertView);
        int calculateWidth(MenuItem* menuItem);
    private:
        View* createMenuButton(MenuItem* menuItem);
        bool shouldShowIcon(MenuItem* menuItem) const;
        Context* mContext;
        int mIconTextSpacing;
        int mSidePadding;
        View* mCalculator;   // off-screen button reused to measure overflow item widths
    };
};

};//namespace
#endif/*__LOCAL_FLOATING_TOOLBAR_POPUP_H__*/
