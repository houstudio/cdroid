/*********************************************************************************
 * Shared-element scene transition (route B) pins: App::startActivity(Intent,
 * ActivityOptions.makeSceneTransitionAnimation) drives the enter flight in the
 * started window (snapshot ghost in its overlay, target hidden until landing,
 * window-level enter suppressed), and Window::close() drives the return flight
 * in the caller's overlay (host retired from the compositor at once, teardown
 * at landing). A name that resolves to nothing falls back to the untouched
 * window-level enter (the AOSP app-transition fallback).
 *********************************************************************************/
#include <guienvironment.h>
#include <core/intent.h>
#include <core/activityfactory.h>
#include <widget/activityoptions.h>
#include <widget/textview.h>
#include <widget/framelayout.h>
#include <algorithm>

using namespace cdroid;

namespace {
// Tree search by transitionName. FragmentManager::findViewByTransitionName is the
// same walk, but androidx keeps it package-private (the C++ port keeps it private)
// — test-local copy.
View* findByTransitionName(View* root, const std::string& name){
    if(!root) return nullptr;
    if(root->getTransitionName() == name) return root;
    if(ViewGroup* vg = dynamic_cast<ViewGroup*>(root)){
        for(int i = 0; i < vg->getChildCount(); i++){
            if(View* found = findByTransitionName(vg->getChildAt(i), name)) return found;
        }
    }
    return nullptr;
}

// The started window: a hero target carrying transitionName "hero". Its ctor
// installs a programmatic window enter — the scene transition must suppress it.
class SceneTargetWindow : public Window{
public:
    SceneTargetWindow() : Window(0, 0, -1, -1){
        setEnterTransition(ActivityTransition::slide(Gravity::RIGHT));
        FrameLayout* root = new FrameLayout(&App::getInstance());
        TextView* hero = new TextView(&App::getInstance());
        hero->setText("HERO");
        hero->setTransitionName("hero");
        hero->setBackgroundColor(0xFFEF6C00);
        root->addView(hero, new ViewGroup::LayoutParams(320, 120));
        addView(root, new ViewGroup::LayoutParams(-1, -1));
    }
};
REGISTER_ACTIVITY(SceneTargetWindow);

Window* newestWindow(){
    std::vector<Window*> windows;
    WindowManager::getInstance().getWindows(windows);
    return windows.empty() ? nullptr : windows.back();
}
} // namespace

TEST(SCENE, SharedElementEnterAndReturn){
    App& app = App::getInstance();
    Window* caller = GUIEnvironment::stage();

    // The caller's shared element: laid out before the capture.
    TextView* hero = new TextView(&app);
    hero->setText("HERO");
    hero->setTransitionName("hero");
    hero->setBackgroundColor(0xFFEF6C00);
    GUIEnvironment::content()->addView(hero, new ViewGroup::LayoutParams(200, 64));
    pumpFor(100);
    ASSERT_GT(hero->getWidth(), 0);

    // startActivity with a scene transition: the coordinator captures the hero
    // (bounds + pixels) and the enter flight prepares on the first traversal.
    ActivityOptions* opts = ActivityOptions::makeSceneTransitionAnimation(caller, hero, "hero");
    Intent intent;
    intent.setClassName("gui_test", "SceneTargetWindow");
    app.startActivity(intent, opts);
    pumpFor(80);

    Window* target = newestWindow();
    ASSERT_NE(target, nullptr);
    ASSERT_NE(target, caller);
    View* targetHero = findByTransitionName(target, "hero");
    ASSERT_NE(targetHero, nullptr);
    // Target hidden until landing, ghost flying in the window's own overlay, and the
    // window-level enter (installed by the ctor above) suppressed — a scene transition
    // replaces the app transition.
    EXPECT_EQ(targetHero->getVisibility(), View::INVISIBLE);
    EXPECT_FALSE(target->getOverlay()->isEmpty());
    EXPECT_EQ(target->getEnterTransition(), nullptr);

    pumpFor(400); // flight (300ms) lands
    EXPECT_EQ(targetHero->getVisibility(), View::VISIBLE);
    EXPECT_TRUE(target->getOverlay()->isEmpty());

    // Return flight: close() retires the host from the compositor at once and flies
    // the ghost in the CALLER's overlay; the caller's own hero is hidden until landing.
    target->close();
    pumpFor(30);
    std::vector<Window*> windows;
    WindowManager::getInstance().getWindows(windows);
    EXPECT_EQ(std::find(windows.begin(), windows.end(), target), windows.end());
    EXPECT_FALSE(caller->getOverlay()->isEmpty());
    EXPECT_EQ(hero->getVisibility(), View::INVISIBLE);

    pumpFor(600); // land + the posted teardown
    EXPECT_TRUE(caller->getOverlay()->isEmpty());
    EXPECT_EQ(hero->getVisibility(), View::VISIBLE);
    WindowManager::getInstance().getWindows(windows);
    EXPECT_EQ(std::find(windows.begin(), windows.end(), target), windows.end());

    GUIEnvironment::content()->removeView(hero);
    delete hero;
}

TEST(SCENE, NoPairFallsBackToWindowEnter){
    App& app = App::getInstance();

    TextView* hero = new TextView(&app);
    hero->setTransitionName("hero");
    GUIEnvironment::content()->addView(hero, new ViewGroup::LayoutParams(200, 64));
    pumpFor(100);

    // "nomatch" resolves to no view in SceneTargetWindow: prepareEnter returns false,
    // the coordinator is dropped and the window-level enter stays fully intact.
    ActivityOptions* opts = ActivityOptions::makeSceneTransitionAnimation(
        GUIEnvironment::stage(), hero, "nomatch");
    Intent intent;
    intent.setClassName("gui_test", "SceneTargetWindow");
    app.startActivity(intent, opts);
    pumpFor(120);

    Window* target = newestWindow();
    ASSERT_NE(target, nullptr);
    EXPECT_NE(target->getEnterTransition(), nullptr); // NOT suppressed
    EXPECT_TRUE(target->getOverlay()->isEmpty());

    target->close();
    pumpFor(600); // window-level exit animation + teardown
    std::vector<Window*> windows;
    WindowManager::getInstance().getWindows(windows);
    EXPECT_EQ(std::find(windows.begin(), windows.end(), target), windows.end());

    GUIEnvironment::content()->removeView(hero);
    delete hero;
}
