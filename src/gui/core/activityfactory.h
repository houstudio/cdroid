#ifndef __ACTIVITYFACTORY_H__
#define __ACTIVITYFACTORY_H__
/*********************************************************************************
 * CDROID Activity-instantiation registry — the className -> Window* factory that backs
 * Context.startActivity (the seam androidx ActivityNavigator ends in). Mirrors the
 * FragmentFactory + REGISTER_FRAGMENT pattern (src/gui/fragment/fragmentfactory.h): apps register
 * each Window ("Activity") subclass by name; startActivity(ComponentName.className) looks it up and
 * `new`s the Window (whose constructor self-registers with WindowManager, so it appears on screen).
 *
 * Android resolves Activity by className via the framework/PackageManager; CDROID has no such
 * resolver, so this explicit registry is the CDROID equivalent.
 *********************************************************************************/
#include <string>
#include <functional>
namespace cdroid{
class Window;

class ActivityFactory{
public:
    virtual ~ActivityFactory() = default;
    static void registerActivity(const std::string& className, std::function<Window*()> ctor);
    static bool isActivityClass(const std::string& className);
    // Look up className and invoke its factory (returns a self-registered, on-screen Window*).
    virtual Window* instantiate(const std::string& className);
};

// Register a Window subclass for startActivity-by-name. The class MUST have a default constructor
// whose body chains to a Window(...) ctor (so it self-registers with WindowManager). Place at file
// scope (analogous to REGISTER_FRAGMENT).
#define REGISTER_ACTIVITY(ClassName)                                                                \
    static const int _cdroid_act_reg_##ClassName =                                                  \
        (::cdroid::ActivityFactory::registerActivity(                                               \
             #ClassName, []() -> ::cdroid::Window* { return new ClassName(); }), 0)

}//namespace
#endif
