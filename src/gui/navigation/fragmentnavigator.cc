#include <navigation/fragmentnavigator.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/fragmentmanager.h>

namespace cdroid{

FragmentNavigator::FragmentNavigator(fragment::FragmentManager* fm, int containerId)
    : mFragmentManager(fm), mContainerId(containerId){
    mName = "fragment";
}

NavDestination* FragmentNavigator::createDestination(){
    return new Destination(this);
}

void FragmentNavigator::navigate(NavDestination* destination, Bundle* args, NavOptions* /*navOptions*/){
    Destination* d = dynamic_cast<Destination*>(destination);
    if(!d || !mFragmentManager) return;
    fragment::FragmentFactory factory;
    fragment::Fragment* fragment = factory.instantiate(d->getClassName());
    if(!fragment) return;
    fragment->setArguments(args ? new Bundle(*args) : nullptr);
    fragment::FragmentTransaction* t = mFragmentManager->beginTransaction();
    t->replace(mContainerId, fragment);
    t->addToBackStack(d->getRoute());
    t->commit();
}

bool FragmentNavigator::popBackStack(){
    return mFragmentManager ? mFragmentManager->popBackStackImmediate() : false;
}

}//namespace cdroid
