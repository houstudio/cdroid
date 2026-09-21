/*********************************************************************************
 * The existing showcase pages, each wrapped as a demo Fragment and registered
 * under its ApiDemos-style path. The page layouts live in assets/layout/page_*.xml
 * and the wiring hooks in pages.cc — both unchanged.
 *
 * Layouts/ is the androidx wing ApiDemos does not have (ConstraintLayout,
 * MotionLayout/Carousel): those pages are CDROID originals and stay top-level.
 *********************************************************************************/
#include <cdroid.h>
#include "demoregistry.h"
#include "pages.h"
#include <R.h>

using namespace cdroid;

// A demo page = one inflated layout + one post-inflation setup hook. Rank =
// the pre-refactor tab order (kept verbatim: Buttons, Progress, Text, Images,
// Animation, Lists, Misc, DateTime, Constraint, Motion, Switchers).
#define DEMO_PAGE_FRAGMENT(ClassName, PathValue, LayoutRes, SetupFn, Rank)            \
    class ClassName : public Fragment {                                               \
    public:                                                                           \
        cdroid::View* onCreateView(cdroid::LayoutInflater* inflater,                   \
                                   cdroid::ViewGroup* container,                      \
                                   cdroid::Bundle* savedInstanceState) override {     \
            return inflater->inflate(LayoutRes, container, false);                    \
        }                                                                             \
        void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState) override { \
            Fragment::onViewCreated(view, savedInstanceState);                        \
            SetupFn(view);                                                            \
        }                                                                             \
    };                                                                                \
    REGISTER_DEMO_FRAGMENT_IMPL(PathValue, ClassName, Rank);

DEMO_PAGE_FRAGMENT(ButtonsPage,    "Views/Buttons",            widgetsDemo::R::layout::page_buttons,    setupButtons,    1)
DEMO_PAGE_FRAGMENT(ProgressPage,   "Views/Progress",           widgetsDemo::R::layout::page_progress,   setupProgress,   2)
DEMO_PAGE_FRAGMENT(TextPage,       "Views/Text",               widgetsDemo::R::layout::page_text,       setupText,       3)
DEMO_PAGE_FRAGMENT(ImagesPage,     "Views/Images",             widgetsDemo::R::layout::page_images,     setupImages,     4)
DEMO_PAGE_FRAGMENT(AnimationPage,  "Views/Animation",          widgetsDemo::R::layout::page_animation,  setupAnimation,  5)
DEMO_PAGE_FRAGMENT(ListsPage,      "Views/Lists",              widgetsDemo::R::layout::page_lists,      setupLists,      6)
DEMO_PAGE_FRAGMENT(MiscPage,       "Views/Misc",               widgetsDemo::R::layout::page_misc,       setupMisc,       7)
DEMO_PAGE_FRAGMENT(DateTimePage,   "Views/DateTime/Pickers",   widgetsDemo::R::layout::page_datetime,   setupDateTime,   8)
DEMO_PAGE_FRAGMENT(ConstraintPage, "Layouts/ConstraintLayout", widgetsDemo::R::layout::page_constraint, setupConstraint, 9)
DEMO_PAGE_FRAGMENT(MotionPage,     "Layouts/MotionLayout",     widgetsDemo::R::layout::page_motion,     setupMotion,    10)
DEMO_PAGE_FRAGMENT(SwitchersPage,  "Views/Switchers",          widgetsDemo::R::layout::page_flipper,    setupFlipper,   11)
