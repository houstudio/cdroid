#ifndef __NAVHOSTFRAGMENT_H__
#define __NAVHOSTFRAGMENT_H__
/*********************************************************************************
 * Port of androidx.navigation.fragment.NavHostFragment. A Fragment that hosts a
 * NavController and registers a FragmentNavigator so route navigation swaps child
 * Fragments via FragmentTransaction.
 *********************************************************************************/
#include <navigation/navhost.h>
#include <navigation/navcontroller.h>
#include <fragment/fragment.h>
namespace cdroid{

class NavGraph;
class FragmentNavigator;

class NavHostFragment : public fragment::Fragment, public NavHost{
public:
    // graphRef: optional navigation-graph resource ref (e.g. "@navigation/nav_graph"). When set,
    // the graph is inflated and applied on resume, auto-navigating to its startDestination — no
    // app-side inflate/setGraph/navigate code needed (mirrors androidx app:navGraph / create()).
    explicit NavHostFragment(const std::string& graphRef = "");
    ~NavHostFragment() override;
    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
    // NavHost
    NavController* getNavController() override { return mNavController; }

    // Set the nav graph (call after the host is attached).
    void setGraph(NavGraph* graph);

protected:
    virtual NavController* onCreateNavController();

private:
    NavController* mNavController = nullptr;
    std::string mGraphRef;     // graph resource ref captured at construction (androidx graphId)
    bool mGraphLoaded = false; // guard: apply the graph once per attach
};

}//namespace cdroid
#endif
