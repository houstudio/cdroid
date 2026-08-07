#ifndef __COMPONENTNAME_H__
#define __COMPONENTNAME_H__
/*********************************************************************************
 * Port of android.content.ComponentName — an opaque identifier for an application component
 * (an Activity/Service/etc.), defined by package name + class name. CDROID uses it to name the
 * Window ("Activity") a navigation <activity> destination targets; the package→Window lookup lives
 * at the Context.startActivity seam (cf. FragmentFactory + REGISTER_FRAGMENT).
 *********************************************************************************/
#include <string>
namespace cdroid{

class ComponentName{
public:
    ComponentName() = default;
    ComponentName(const std::string& pkg, const std::string& cls) : mPackage(pkg), mClass(cls) {}

    const std::string& getPackageName() const { return mPackage; }
    const std::string& getClassName() const { return mClass; }

    // android.flattenToShortString / flattenToString: "package/class" (class only if no package).
    std::string getFlatClassName() const {
        return mPackage.empty() ? mClass : (mPackage + "/" + mClass);
    }
    std::string flattenToString() const { return getFlatClassName(); }

    bool equals(const ComponentName& o) const { return mPackage == o.mPackage && mClass == o.mClass; }
    bool operator==(const ComponentName& o) const { return equals(o); }
    bool operator!=(const ComponentName& o) const { return !equals(o); }

    std::string toString() const {
        std::string s = "ComponentInfo{";
        s += getFlatClassName();
        s += "}";
        return s;
    }
private:
    std::string mPackage;
    std::string mClass;
};

}//namespace
#endif
