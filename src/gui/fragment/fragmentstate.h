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
#ifndef __FRAGMENTSTATE_H__
#define __FRAGMENTSTATE_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentState — the per-fragment saved state used by
 * saveBackStack/restoreBackStack.
 *
 * androidx FragmentState is the structural identity (className, who, ids, lifecycle/visibility
 * flags) and is Parcelable; the user/view saved-state slices (onSaveInstanceState Bundle,
 * SavedStateRegistry state, view-hierarchy SparseArray, arguments) live in the SURROUNDING Bundle
 * that FragmentStateManager.saveState() returns and FragmentStore.mSavedState[who] stores.
 *
 * CDROID holds these in one struct because (a) BaseBundle is map<string,any> and does not nest
 * Bundles, and (b) the slice pointers (viewState, savedInstanceState, arguments) need explicit
 * ownership, which a Bundle of `any` cannot provide. The fields below named like the androidx
 * FragmentState fields are an exact 1:1 port; the trailing "saved-state slices" section is the
 * CDROID folding of that surrounding Bundle (no Parcel — in-memory only).
 *********************************************************************************/
#include <string>
#include <core/bundle.h>
#include <core/parcelable.h>
#include <core/sparsearray.h>
#include <savedstate/savedstate.h>
#include <lifecycle/lifecycle.h>
namespace cdroid{
namespace fragment{

struct FragmentState{
    // --- androidx FragmentState fields (FragmentState.java:29-43), exact port ---
    std::string className;
    std::string who;
    bool fromLayout = false;
    bool inDynamicContainer = false;
    int fragmentId = 0;
    int containerId = 0;
    std::string tag;
    bool retainInstance = false;
    bool removing = false;
    bool detached = false;
    bool hidden = false;
    lifecycle::Lifecycle::State maxLifecycleState = lifecycle::Lifecycle::State::RESUMED;
    std::string targetWho;
    int targetRequestCode = 0;
    bool userVisibleHint = true;

    // --- CDROID saved-state slices (androidx packs these into the surrounding Bundle keyed by
    //     FRAGMENT_STATE_KEY / SAVED_INSTANCE_STATE_KEY / REGISTRY_STATE_KEY / VIEW_STATE_KEY /
    //     ARGUMENTS_KEY). Owned by this struct. ---
    SparseArray<Parcelable*>* viewState = nullptr;        // mView->saveHierarchyState
    Bundle* savedInstanceState = nullptr;                  // fragment.onSaveInstanceState
    savedstate::SavedState registryState;                  // SavedStateRegistry performSave
    Bundle* arguments = nullptr;                           // mArguments (owned copy)

    FragmentState() = default;
    ~FragmentState(){ delete viewState; delete savedInstanceState; delete arguments; }
    FragmentState(const FragmentState&) = delete;
    FragmentState& operator=(const FragmentState&) = delete;
};

}//namespace fragment
}//namespace cdroid
#endif/*__FRAGMENTSTATE_H__*/
