#include <navigation/appbarconfiguration.h>
#include <widget/drawerlayout.h>

namespace cdroid{

// The legacy DrawerLayout-typed face over the Openable member.
DrawerLayout* AppBarConfiguration::getDrawerLayout() const {
    return dynamic_cast<DrawerLayout*>(mOpenableLayout);
}

AppBarConfiguration::Builder& AppBarConfiguration::Builder::setDrawerLayout(DrawerLayout* drawer) {
    mOpenableLayout = drawer;
    return *this;
}

}//namespace cdroid
