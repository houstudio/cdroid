#ifndef __APPBARCONFIGURATION_H__
#define __APPBARCONFIGURATION_H__
/*********************************************************************************
 * Port of androidx.navigation.ui.AppBarConfiguration. Determines which destinations
 * are "top level" (no Up arrow, drawer button instead) and the optional drawer layout.
 * Uses route strings (modern navigation model).
 *********************************************************************************/
#include <set>
#include <string>
namespace cdroid{
class DrawerLayout;

class AppBarConfiguration{
public:
    class Builder;
    AppBarConfiguration() = default;

    const std::set<std::string>& getTopLevelDestinationRoutes() const { return mTopLevelRoutes; }
    DrawerLayout* getDrawerLayout() const { return mDrawerLayout; }
    bool isTopLevelDestination(const std::string& route) const {
        return mTopLevelRoutes.find(route) != mTopLevelRoutes.end();
    }
private:
    AppBarConfiguration(std::set<std::string> routes, DrawerLayout* drawer)
        : mTopLevelRoutes(std::move(routes)), mDrawerLayout(drawer){}
    std::set<std::string> mTopLevelRoutes;
    DrawerLayout* mDrawerLayout = nullptr;
};

class AppBarConfiguration::Builder{
public:
    Builder& addTopLevelRoute(const std::string& route){ mTopLevelRoutes.insert(route); return *this; }
    Builder& setTopLevelRoutes(const std::set<std::string>& routes){ mTopLevelRoutes = routes; return *this; }
    Builder& setDrawerLayout(DrawerLayout* drawer){ mDrawerLayout = drawer; return *this; }
    AppBarConfiguration* build(){ return new AppBarConfiguration(mTopLevelRoutes, mDrawerLayout); }
private:
    std::set<std::string> mTopLevelRoutes;
    DrawerLayout* mDrawerLayout = nullptr;
};

}//namespace cdroid
#endif
