/*********************************************************************************
 * Navigation MVP demo (runtime, no Fragment). Builds a NavGraph programmatically
 * with two route destinations, drives NavController: setGraph -> navigate("b")
 * -> popBackStack() back to "a". Verifies route navigation + NavBackStackEntry
 * back stack + Lifecycle + OnDestinationChangedListener end-to-end.
 *********************************************************************************/
#include <cdroid.h>
#include <navigation/navcontroller.h>
#include <navigation/navgraph.h>
#include <navigation/noopnavigator.h>
#include <navigation/navigatorprovider.h>
#include <navigation/navdestination.h>
#include <porting/cdlog.h>

class DemoListener : public cdroid::NavController::OnDestinationChangedListener{
public:
    void onDestinationChanged(cdroid::NavController* /*controller*/,
                              cdroid::NavDestination* destination,
                              cdroid::Bundle* /*arguments*/) override{
        LOGD("[nav_demo] navigated -> route=%s",
             destination ? destination->getRoute().c_str() : "(null)");
    }
};

int main(int argc, const char* argv[]){
    cdroid::App app(argc, argv);

    cdroid::NavController controller(&app);
    cdroid::NoOpNavigator* navigator = new cdroid::NoOpNavigator();
    controller.getNavigatorProvider()->addNavigator(navigator);

    cdroid::NavGraph* graph = new cdroid::NavGraph(controller.getNavigatorProvider());
    cdroid::NavDestination* a = navigator->createDestination(); a->setRoute("a");
    cdroid::NavDestination* b = navigator->createDestination(); b->setRoute("b");
    graph->addDestination(a);
    graph->addDestination(b);
    graph->setStartDestination("a");

    DemoListener listener;
    controller.addOnDestinationChangedListener(&listener);

    LOGD("[nav_demo] setGraph (expect 'a')");
    controller.setGraph(graph);

    LOGD("[nav_demo] navigate('b') (expect 'b')");
    controller.navigate("b");

    LOGD("[nav_demo] popBackStack (expect 'a')");
    controller.popBackStack();

    LOGD("[nav_demo] current=%s",
         controller.getCurrentDestination() ? controller.getCurrentDestination()->getRoute().c_str() : "(none)");

    return app.exec();
}
