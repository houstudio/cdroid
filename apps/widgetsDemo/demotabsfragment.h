#pragma once
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentpageradapter.h>
#include <vector>

namespace cdroid {
struct DemoEntry;  // demoregistry.h
}

// A category screen: TabLayout + ViewPager over every leaf demo under the
// prefix (recursively flattened — sub-category demos become plain tabs), so
// demos switch laterally by tab, the way the pre-refactor widgetsDemo paged.
// Tab pages are the registered demo fragments themselves.
class DemoTabsPagerAdapter : public cdroid::FragmentPagerAdapter {
    std::vector<const cdroid::DemoEntry*> mLeaves;  // borrowed from the registry
public:
    DemoTabsPagerAdapter(cdroid::FragmentManager* fm,
                         std::vector<const cdroid::DemoEntry*>&& leaves);
    cdroid::Fragment* getItem(int position) override;
    int getCount() override;
    std::string getPageTitle(int position) override;
};

class DemoTabsFragment : public cdroid::Fragment {
    std::string mPrefix;
    int mInitialPage = 0;
    cdroid::ViewPager* mPager = nullptr;  // created in onViewCreated
public:
    static DemoTabsFragment* newInstance(const std::string& prefix, int initialPage);

    void onCreate(cdroid::Bundle* savedInstanceState) override;
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
    void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState) override;

    // The hosted pager (null before onViewCreated) — AUTOCYCLE sweeps tabs
    // through it, the way the pre-refactor app swept its ViewPager pages.
    cdroid::ViewPager* getViewPager() const { return mPager; }
};
