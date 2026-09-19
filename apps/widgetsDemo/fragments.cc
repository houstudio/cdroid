#include <core/app.h>
#include "fragments.h"
#include <R.h>

using namespace cdroid;

namespace {
constexpr int kPageCount = 11;

int pageLayout(int page) {
    namespace R = widgetsDemo::R;
    static const int LAYOUTS[kPageCount] = {
        R::layout::page_buttons, R::layout::page_progress, R::layout::page_text,
        R::layout::page_images, R::layout::page_animation, R::layout::page_lists,
        R::layout::page_misc, R::layout::page_datetime, R::layout::page_constraint,
        R::layout::page_motion, R::layout::page_flipper,
    };
    return LAYOUTS[page];
}

void (*pageSetup(int page))(View*) {
    static void (*const SETUP[kPageCount])(View*) = {
        setupButtons, setupProgress, setupText, setupImages, setupAnimation,
        setupLists, setupMisc, setupDateTime, setupConstraint, setupMotion,
        setupFlipper,
    };
    return SETUP[page];
}
} // namespace

DemoPageFragment* DemoPageFragment::newInstance(int page) {
    Bundle* args = new Bundle();
    args->putInt("page", page);
    DemoPageFragment* f = new DemoPageFragment();
    f->setArguments(args);
    return f;
}

void DemoPageFragment::onCreate(Bundle* savedInstanceState) {
    Fragment::onCreate(savedInstanceState);
    if (getArguments()) mPage = getArguments()->getInt("page");
}

View* DemoPageFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                     Bundle* savedInstanceState) {
    return inflater->inflate(pageLayout(mPage), container, false);
}

void DemoPageFragment::onViewCreated(View* view, Bundle* savedInstanceState) {
    Fragment::onViewCreated(view, savedInstanceState);
    // Idempotent: ViewPager may rebind a recycled page view.
    pageSetup(mPage)(view);
}

// ---------------------------------------------------------------------------
DemoFragmentPagerAdapter::DemoFragmentPagerAdapter(FragmentManager* fm)
    : FragmentPagerAdapter(fm, FragmentPagerAdapter::BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {}

Fragment* DemoFragmentPagerAdapter::getItem(int position) {
    return DemoPageFragment::newInstance(position);
}

int DemoFragmentPagerAdapter::getCount() {
    return kPageCount;
}

std::string DemoFragmentPagerAdapter::getPageTitle(int position) {
    namespace R = widgetsDemo::R;
    static const int TITLES[kPageCount] = {
        R::string::tab_lights, R::string::tab_energy, R::string::tab_messages,
        R::string::tab_gallery, R::string::tab_motion_fx, R::string::tab_devices,
        R::string::tab_dashboard, R::string::tab_schedule, R::string::tab_climate,
        R::string::tab_curtain, R::string::tab_flipper,
    };
    Context* ctx = &App::getInstance();
    return ctx->getString(TITLES[position]);
}
