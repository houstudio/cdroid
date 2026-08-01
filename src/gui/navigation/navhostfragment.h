/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
    // Publish/clear this host's NavController on its View (and any id-matching ancestor) so
    // Navigation::findNavController(view) resolves from any child/sibling (androidx
    // NavHostFragment.onViewCreated / onDestroyView).
    void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState) override;
    void onDestroyView() override;
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
