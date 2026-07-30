#include <navigation/noopnavigator.h>
#include <navigation/navdestination.h>
namespace cdroid{

NavDestination* NoOpNavigator::createDestination(){
    NavDestination* destination = new NavDestination((Navigator*)this);
    destination->setNavigatorName("NoOp");
    return destination;
}

}//namespace cdroid
