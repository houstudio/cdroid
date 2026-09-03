#pragma once
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentpageradapter.h>
#include "pages.h"

// One Fragment per showcase page. A single class parameterized by the page
// index (androidx pattern: fragment reads its position from its arguments
// Bundle), inflating the page layout and running the matching setup*() hook.
class DemoPageFragment : public cdroid::Fragment {
    int mPage = 0;
public:
    static DemoPageFragment* newInstance(int page);

    void onCreate(cdroid::Bundle* savedInstanceState) override;
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
    void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState) override;
};

// androidx FragmentPagerAdapter: each tab page is a Fragment held by the
// FragmentManager (views may be destroyed off-screen, instances retained).
class DemoFragmentPagerAdapter : public cdroid::FragmentPagerAdapter {
public:
    DemoFragmentPagerAdapter(cdroid::FragmentManager* fm);
    cdroid::Fragment* getItem(int position) override;
    int getCount() override;
    std::string getPageTitle(int position) override;
};
