#include <app/espresso/espresso.h>

#include <stdexcept>

#include <app/espresso/datainteraction.h>
#include <app/espresso/defaultfailurehandler.h>
#include <app/espresso/rootmatchers.h>
#include <app/espresso/rootsoracle.h>
#include <app/espresso/rootviewpicker.h>
#include <app/espresso/uicontrollerimpl.h>
#include <app/espresso/viewactions.h>
#include <app/espresso/viewfinder.h>
#include <app/espresso/viewinteraction.h>
#include <app/espresso/viewmatchers.h>

#include <core/app.h>
#include <core/looper.h>

namespace cdroid {
namespace espresso {

namespace {

/* Espresso's object graph (AOSP: ViewInteractionModule/Dagger) — assembled
 * lazily on the main thread, owning each other for the process lifetime. */
struct EspressoGraph {
    std::shared_ptr<RootMatcherPtr> rootMatcherRef =
            std::make_shared<RootMatcherPtr>(RootMatchers::DEFAULT());

    std::unique_ptr<DefaultFailureHandler> defaultFailureHandler;
    FailureHandler* failureHandler = nullptr;  // custom or the default above

    std::unique_ptr<UiControllerImpl> uiController;
    std::unique_ptr<RootsOracle> rootsOracle;
    std::unique_ptr<RootViewPicker> rootViewPicker;

    EspressoGraph()
            : defaultFailureHandler(new DefaultFailureHandler(&App::getInstance())),
              failureHandler(defaultFailureHandler.get()),
              uiController(new UiControllerImpl(Looper::getMainLooper())),
              rootsOracle(new RootsOracle()),
              rootViewPicker(new RootViewPicker(
                      [this] { return *rootMatcherRef; }, *uiController,
                      Looper::getMainLooper())) {}
};

EspressoGraph& graph() {
    static EspressoGraph g;
    return g;
}

} // namespace

ViewInteraction Espresso::onView(MatcherPtr<View> viewMatcher) {
    EspressoGraph& g = graph();
    // AOSP injects a single ViewFinderImpl; the per-interaction part is the
    // matcher, so a fresh finder accompanies each interaction (owned by it).
    auto viewFinder = std::make_shared<ViewFinderImpl>(viewMatcher, *g.rootViewPicker);
    return ViewInteraction(*g.uiController, std::move(viewFinder), *g.failureHandler,
            std::move(viewMatcher), g.rootMatcherRef);
}

DataInteraction Espresso::onData(MatcherPtr<void*> dataMatcher) {
    return DataInteraction(std::move(dataMatcher));
}

void Espresso::setFailureHandler(FailureHandler* failureHandler) {
    graph().failureHandler = failureHandler ? failureHandler
                                            : graph().defaultFailureHandler.get();
}

void Espresso::registerIdlingResources(const std::vector<IdlingResourcePtr>& resources) {
    IdlingResourceRegistry::getInstance().registerResources(resources);
}

bool Espresso::unregisterIdlingResources(const std::vector<IdlingResourcePtr>& resources) {
    return IdlingResourceRegistry::getInstance().unregisterResources(resources);
}

std::vector<IdlingResourcePtr> Espresso::getIdlingResources() {
    return IdlingResourceRegistry::getInstance().getResources();
}

void Espresso::closeSoftKeyboard() {
    onView(ViewMatchers::isRoot()).perform({ViewActions::closeSoftKeyboard()});
}

void Espresso::pressBack() {
    onView(ViewMatchers::isRoot()).perform({ViewActions::pressBack()});
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
