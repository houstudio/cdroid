#ifndef __FRAGMENTNAVIGATOR_H__
#define __FRAGMENTNAVIGATOR_H__
/*********************************************************************************
 * Port of androidx.navigation.fragment.FragmentNavigator. A Navigator that swaps
 * Fragments via FragmentTransaction.replace for each destination's className.
 *********************************************************************************/
#include <string>
#include <navigation/navigator.h>
#include <navigation/navdestination.h>
#include <navigation/navoptions.h>
#include <core/bundle.h>
#include <core/attributeset.h>
#include <porting/cdlog.h>
namespace cdroid{
namespace fragment{ class FragmentManager; }
class FragmentNavigator : public Navigator{
public:
    class Destination : public NavDestination{
    public:
        explicit Destination(FragmentNavigator* owner) : NavDestination((Navigator*)owner){}
        // Reads the Fragment class name from XML android:name.
        void onInflate(cdroid::Context* context, const AttributeSet& attrs) override;
        void setClassName(const std::string& cls){ mClassName = cls; }
        const std::string& getClassName() const { return mClassName; }
    private:
        std::string mClassName;
    };

    FragmentNavigator(fragment::FragmentManager* fm, int containerId);
    NavDestination* createDestination() override;
    void navigate(NavDestination* destination, Bundle* args, NavOptions* navOptions) override;
    bool popBackStack() override;
private:
    fragment::FragmentManager* mFragmentManager;
    int mContainerId;
};
}//namespace cdroid
#endif
