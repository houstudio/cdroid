#include "demotabsfragment.h"
#include "demoregistry.h"
#include <R.h>
#include <algorithm>
#include <core/inputmethodmanager.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <widget/viewpager.h>
#include <widgetEx/tablayout/tablayout.h>

using namespace cdroid;

DemoTabsPagerAdapter::DemoTabsPagerAdapter(FragmentManager* fm,
                                           std::vector<const DemoEntry*>&& leaves)
    : FragmentPagerAdapter(fm, FragmentPagerAdapter::BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT),
      mLeaves(std::move(leaves)) {}

Fragment* DemoTabsPagerAdapter::getItem(int position) {
    return mLeaves.at(position)->create();
}

int DemoTabsPagerAdapter::getCount() {
    return (int)mLeaves.size();
}

std::string DemoTabsPagerAdapter::getPageTitle(int position) {
    const std::string& path = mLeaves.at(position)->path;
    return path.substr(path.rfind('/') + 1);
}

DemoTabsFragment* DemoTabsFragment::newInstance(const std::string& prefix, int initialPage) {
    Bundle* args = new Bundle();
    args->putString("prefix", prefix);
    args->putInt("page", initialPage);
    DemoTabsFragment* f = new DemoTabsFragment();
    f->setArguments(args);
    return f;
}

void DemoTabsFragment::onCreate(Bundle* savedInstanceState) {
    Fragment::onCreate(savedInstanceState);
    if (getArguments()) {
        mPrefix = getArguments()->getString("prefix");
        mInitialPage = getArguments()->getInt("page");
    }
}

View* DemoTabsFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                     Bundle* savedInstanceState) {
    return inflater->inflate(widgetsDemo::R::layout::demo_tabs, container, false);
}

void DemoTabsFragment::onViewCreated(View* view, Bundle* savedInstanceState) {
    Fragment::onViewCreated(view, savedInstanceState);
    namespace R = widgetsDemo::R;

    TabLayout* tabs = (TabLayout*)view->findViewById(R::id::demo_tabs);
    ViewGroup* host = (ViewGroup*)view->findViewById(R::id::demo_pager_host);
    if (tabs == nullptr || host == nullptr) return;

    // The legacy androidx ViewPager is not registered for XML inflation, so it
    // is created here and hosted in the layout's container (pre-refactor
    // widgetsDemo shape).
    ViewPager* pager = new ViewPager(view->getContext());
    mPager = pager;
    pager->setId(View::generateViewId());
    auto* adapter = new DemoTabsPagerAdapter(getParentFragmentManager(),
                                             DemoRegistry::get().leavesUnder(mPrefix));
    pager->setAdapter(adapter);
    // Creator-owns rule: anchor the adapter's deletion to the pager's lifetime.
    pager->setTag(View::generateViewId(), adapter,
                  [](void* p) { delete static_cast<DemoTabsPagerAdapter*>(p); });
    // Small static page sets: keep every tab attached (androidx guidance for
    // FragmentPagerAdapter) instead of tearing pages down on every tab hop.
    pager->setOffscreenPageLimit(std::max(0, adapter->getCount() - 1));
    pager->setCurrentItem(mInitialPage, false);
    host->addView(pager, new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
    tabs->setupWithViewPager(pager);

    // IME on page switch: with every page kept attached (high offscreen
    // limit) the outgoing editor keeps focus and stock Android's ViewPager
    // would leave the IME up too — apps opt into dismissing it themselves.
    // The AOSP app recipe, applied here: drop the pager's focus and hide the
    // IME in OnPageChangeListener (imm.hideSoftInputFromView).
    ViewPager::OnPageChangeListener imeDismiss;
    imeDismiss.onPageSelected = [pager](int) {
        if (View* focused = pager->findFocus()) {
            InputMethodManager::getInstance().hideSoftInputFromView(focused, 0);
            focused->clearFocus();
        }
    };
    pager->addOnPageChangeListener(imeDismiss);
}

REGISTER_FRAGMENT(DemoTabsFragment);
