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
#include <drawable/stateset.h>
#include <bitset.h>
#include <widget/internal_R.h>
namespace cdroid{
using namespace cdroid::internal::R;

const std::vector<int> StateSet::NOTHING = {0};
const std::vector<int> StateSet::WILD_CARD = {};
// State-set constants use the real R::attr IDs (AOSP-aligned). These are the same
// values that StateListDrawable items carry and that stateSetMatches compares.
const std::vector<int>StateSet::PRESSED_STATE_SET = {(int)attr::state_pressed};
const std::vector<int>StateSet::ENABLED_STATE_SET = {(int)attr::state_enabled};
const std::vector<int>StateSet::FOCUSED_STATE_SET = {(int)attr::state_focused};
const std::vector<int>StateSet::SELECTED_STATE_SET= {(int)attr::state_selected};
const std::vector<int>StateSet::CHECKED_STATE_SET = {(int)attr::state_checked};

// AOSP StateSet.VIEW_STATE_IDS: pairs of (R.attr.state_xxx, VIEW_STATE_bit) —
// exactly the base View set (10 states). checked/checkable/single/first/
// middle/last are NOT here; subclasses merge them via View.mergeDrawableStates().
std::vector<int>StateSet::VIEW_STATE_IDS={
    (int)attr::state_window_focused,  VIEW_STATE_WINDOW_FOCUSED,
    (int)attr::state_selected     ,  VIEW_STATE_SELECTED ,
    (int)attr::state_focused      ,  VIEW_STATE_FOCUSED  ,
    (int)attr::state_enabled      ,  VIEW_STATE_ENABLED  ,
    (int)attr::state_pressed      ,  VIEW_STATE_PRESSED  ,
    (int)attr::state_activated    ,  VIEW_STATE_ACTIVATED,
    (int)attr::state_accelerated  ,  VIEW_STATE_ACCELERATED,
    (int)attr::state_hovered      ,  VIEW_STATE_HOVERED  ,
    (int)attr::state_drag_can_accept, VIEW_STATE_DRAG_CAN_ACCEPT,
    (int)attr::state_drag_hovered ,  VIEW_STATE_DRAG_HOVERED,
};

void StateSet::trimStateSet(std::vector<int>&states,int newsize){
    states.resize(newsize);
}

std::vector<int> StateSet::get(int mask){
    std::vector<int> states;
    for( int i = 0 ; i < (int)VIEW_STATE_IDS.size() ; i += 2 ){
        if( mask & VIEW_STATE_IDS[i+1] )
           states.push_back(VIEW_STATE_IDS[i]);
    }
    return states;
}

bool StateSet::isWildCard(const std::vector<int>& stateSetOrSpec){
    return (stateSetOrSpec.size() == 0) || (stateSetOrSpec[0] == 0);
}

bool StateSet::stateSetMatches(const std::vector<int>& stateSpec,const std::vector<int>& stateSet){
    // androidx does NOT short-circuit on an empty stateSet. It walks the spec so that an
    // all-must-not-match spec matches empty (nothing triggers the negative), while a positive
    // must-match fails (not found) and WILD_CARD {0} breaks out early as true.
    const size_t stateSpecSize = stateSpec.size();
    const size_t stateSetSize = stateSet.size();
    for (size_t i = 0; i < stateSpecSize; i++) {
        int stateSpecState = stateSpec[i];
        if (stateSpecState == 0) { // We've reached the end of the cases to match against.
            return true;
        }
        const bool mustMatch = (stateSpecState > 0);
        if (stateSpecState<0) { // We use negative values to indicate must-NOT-match states.
            stateSpecState = -stateSpecState;
        }
        bool found = false;
        for (int j = 0; j < (int)stateSetSize; j++) {
            const int state = stateSet[j];
            if (state == 0) {  // We've reached the end of states to match.
                if (mustMatch){// We didn't find this must-match state.
                    return false;
                } else { //Continue checking other must-not-match states.
                    break;
                }
            }
            if (state == stateSpecState) {
                if (mustMatch) {// Continue checking other other must-match states.
                    found = true;
                    break;
                } else { // Any match of a must-not-match state returns false.
                    return false;
                }
            }
        }
        if (mustMatch && !found) {
            // We've reached the end of states to match and we didn't
            // find a must-match state.
            return false;
        }
    }
    return true;
}

bool StateSet::stateSetMatches(const std::vector<int>& stateSpec,int state){
    const size_t stateSpecSize = stateSpec.size();
    for (size_t i = 0; i < stateSpecSize; i++) {
        int stateSpecState = stateSpec[i];
        if (stateSpecState == 0)// We've reached the end of the cases to match against.
            return true;

        if (stateSpecState > 0) {
            if(state != stateSpecState) return false;
        }else{// We use negative values to indicate must-NOT-match states.
            if(state == -stateSpecState)// We matched a must-not-match case.
                return false;
        }
    }
    return true;
}

bool StateSet::containsAttribute(const std::vector<std::vector<int>>& stateSpecs,int attr){
    for (auto spec : stateSpecs) {
        if (spec.empty())  break;
        for (int specAttr : spec) {
            if ( (specAttr == attr) || (-specAttr == attr) )
                return true;
        }
    }
    return false;
}

// AOSP StateListDrawable.extractStateSet: walks the <item> AttributeSet by
// index, takes each attribute's name resource id, and pushes +attrId or
// -attrId from its boolean value. Attributes without a resource id (and the
// StateListDrawableItem drawable/id attrs) are skipped.
int StateSet::parseState(std::vector<int>&states,const AttributeSet&atts){
    const int numAttrs = atts.getAttributeCount();
    for (int i = 0; i < numAttrs; i++) {
        const int stateResId = atts.getAttributeNameResource(i);
        switch (stateResId) {
            case 0:
                break;
            case attr::drawable:
            case attr::id:
                // Ignore attributes from StateListDrawableItem and
                // AnimatedStateListDrawableItem.
                continue;
            default:
                states.push_back(atts.getAttributeBooleanValue(i, false) ? stateResId : -stateResId);
        }
    }
    return (int)states.size();
}

}
