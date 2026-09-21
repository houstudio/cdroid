// Port of SupportWearDemos WearableDrawersDemo: a WearableDrawerLayout with a
// navigation drawer (top) and an action drawer (bottom), both peeked on enter.
//
// Upstream: ~/research/androidx/samples/SupportWearDemos/ src/main/java/com/
// example/android/support/wear/app/drawers/{WearableDrawersDemo,NavItem,
// DemoNavDrawerAdapter,FrameLayoutFragment,ScrollViewFragment}.java
//
// Deviations from upstream: (1) the two Fragment subclasses are stateless
// layout swappers (onCreateView only inflates one layout), and CDROID's demo
// apps do not drive android.app.Fragment transactions — onNavItemSelected
// re-inflates the NavItem's layout straight into R.id.fragment_container
// instead (same replace-into-container semantics); NavItem therefore carries
// the layout res id where upstream carries the Fragment class. (2) Upstream
// NavItem holds one Drawable instance (GC keeps it alive across adapter
// reads); CDROID's adapter hands out owned drawables per call, so NavItem
// carries the framework icon res id and DemoNavDrawerAdapter mints a fresh
// drawable per getItemDrawable.
#include <core/app.h>
#include <cdroid.h>
#include <cdlog.h>
#include <core/activityfactory.h>
#include <widget/R.h>
#include <widget/toast.h>
#include <menu/menuitem.h>
#include <widgetEx/wear/wearabledrawerlayout.h>
#include <widgetEx/wear/wearabledrawercontroller.h>
#include <widgetEx/wear/wearablenavigationdrawerview.h>
#include <widgetEx/wear/wearablenavigationdraweradapter.h>
#include <widgetEx/wear/wearabledraweractionview.h>
#include "R.h"

using namespace cdroid;

namespace {

/** Represents one top-level navigational item (upstream NavItem.java; the
    Fragment class field becomes the fragment's layout resource here, and the
    Drawable instance becomes the framework icon resource). */
struct NavItem {
    std::string mTitle;
    int mLayoutResId;
    int mIconResId;

    NavItem(const std::string& title, int layoutResId, int iconResId)
        : mTitle(title), mLayoutResId(layoutResId), mIconResId(iconResId) {}
};

} // namespace

/** Simple and declarative WearableNavigationDrawerAdapter (upstream
    DemoNavDrawerAdapter.java). */
class DemoNavDrawerAdapter : public WearableNavigationDrawerView::WearableNavigationDrawerAdapter {
private:
    Context* mContext;
    const std::vector<NavItem> mNavItems;

public:
    explicit DemoNavDrawerAdapter(Context* context, const std::vector<NavItem>& navItems)
        : mContext(context), mNavItems(navItems) {}

    std::string getItemText(int pos) const override {
        return mNavItems[pos].mTitle;
    }

    Drawable* getItemDrawable(int pos) const override {
        return mContext->getDrawable(mNavItems[pos].mIconResId);
    }

    int getCount() const override {
        return (int)mNavItems.size();
    }
};

/** Main Window for demoing the Wearable Drawers (upstream Activity). */
class WearableDrawersDemo : public Window {
private:
    WearableNavigationDrawerView* mNavDrawer;
    WearableActionDrawerView* mActionDrawer;
    std::vector<NavItem> mNavItems;

public:
    WearableDrawersDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        LayoutInflater::from(getContext())->inflate(
                weardemos::R::layout::wearable_drawers_demo, this, true);
        // Upstream's NavItems reference ScrollViewFragment / FrameLayoutFragment
        // and android.R.drawable.star_big_off / star_big_on.
        mNavItems = {
            NavItem("ScrollView",  weardemos::R::layout::wearable_drawers_scroll_view_demo,
                    cdroid::R::drawable::star_big_off),
            NavItem("FrameLayout", weardemos::R::layout::wearable_drawers_frame_layout_demo,
                    cdroid::R::drawable::star_big_on),
        };

        onNavItemSelected(0);

        mNavDrawer = (WearableNavigationDrawerView*)findViewById(weardemos::R::id::nav_drawer);
        mNavDrawer->setAdapter(new DemoNavDrawerAdapter(getContext(), mNavItems));
        mNavDrawer->addOnItemSelectedListener([this](int pos) { onNavItemSelected(pos); });
        mNavDrawer->getController()->peekDrawer();

        mActionDrawer = (WearableActionDrawerView*)findViewById(weardemos::R::id::action_drawer);
        mActionDrawer->setOnMenuItemClickListener(
                [this](MenuItem& menuItem) { return onActionClicked(menuItem); });
        mActionDrawer->getController()->peekDrawer();
    }

private:
    // Upstream onActionClicked(MenuItem): toast the title and re-peek the
    // action drawer so it collapses back after the click.
    bool onActionClicked(MenuItem& menuItem) {
        Toast::makeText(getContext(), menuItem.getTitle() + " clicked", Toast::LENGTH_SHORT)->show();
        mActionDrawer->getController()->peekDrawer();
        return true;
    }

    // Upstream onNavItemSelected(int): reflectively instantiated the NavItem's
    // Fragment and replaced R.id.fragment_container with it (see header note).
    void onNavItemSelected(int pos) {
        ViewGroup* container = (ViewGroup*)findViewById(weardemos::R::id::fragment_container);
        container->removeAllViews();
        LayoutInflater::from(getContext())->inflate(mNavItems[pos].mLayoutResId, container, true);
    }
};
REGISTER_ACTIVITY(WearableDrawersDemo);
