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

// AOSP View.java VIEW_STATE_IDS: pairs of (R.attr.state_xxx, VIEW_STATE_bit).
// Used by View.getDrawableState() to build the drawable state array from the
// view's bitmask. CDROID-private states (state_drag_*) included.
std::vector<int>StateSet::VIEW_STATE_IDS={
    (int)attr::state_window_focused, VIEW_STATE_WINDOW_FOCUSED,
    (int)attr::state_selected     , VIEW_STATE_SELECTED ,
    (int)attr::state_focused      , VIEW_STATE_FOCUSED  ,
    (int)attr::state_enabled      , VIEW_STATE_ENABLED  ,
    (int)attr::state_pressed      , VIEW_STATE_PRESSED  ,
    (int)attr::state_activated    , VIEW_STATE_ACTIVATED,
    (int)attr::state_hovered      , VIEW_STATE_HOVERED  ,
    (int)attr::state_checked      , VIEW_STATE_CHECKED  ,
    (int)attr::state_checkable    , VIEW_STATE_CHECKABLE,
    // CDROID-private drag states (0x010dxxxx IDs from attrs_cdroid.xml).
    0x010d010a /*state_drag_acceptable*/, VIEW_STATE_DRAG_CAN_ACCEPT,
    0x010d010b /*state_drag_hoved*/     , VIEW_STATE_DRAG_HOVERED,

    (int)attr::state_single       , VIEW_STATE_SINGLE,
    (int)attr::state_first        , VIEW_STATE_FIRST,
    (int)attr::state_middle       , VIEW_STATE_MIDDLE,
    (int)attr::state_last         , VIEW_STATE_LAST
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

// AOSP StateSet.parseState: reads state_* attrs from the <item> AttributeSet.
// Uses getAttributeBooleanValue (namespace-keyed, AOSP-faithful) — checks whether
// each state attr is present and true/false, pushing +attrId or -attrId.
int StateSet::parseState(std::vector<int>&states,const AttributeSet&atts){
    // Framework state attrs (0x010100xx IDs). getAttributeBooleanValue(namespace,
    // name, def) does an O(n) name→index scan then reads the typed Res_value —
    // faithful to AOSP's AttributeSet.getAttributeBooleanValue.
    static const struct { const char* name; int attrId; } frameworkStates[] = {
        {"state_window_focused", (int)attr::state_window_focused},
        {"state_selected",       (int)attr::state_selected},
        {"state_focused",        (int)attr::state_focused},
        {"state_enabled",        (int)attr::state_enabled},
        {"state_checkable",      (int)attr::state_checkable},
        {"state_checked",        (int)attr::state_checked},
        {"state_pressed",        (int)attr::state_pressed},
        {"state_hovered",        (int)attr::state_hovered},
        {"state_activated",      (int)attr::state_activated},
        {"state_single",         (int)attr::state_single},
        {"state_first",          (int)attr::state_first},
        {"state_middle",         (int)attr::state_middle},
        {"state_last",           (int)attr::state_last},
    };
    for (const auto& s : frameworkStates) {
        if (atts.hasAttribute(s.name)) {
            const bool val = atts.getBoolean(s.name, false);
            states.push_back(val ? s.attrId : -s.attrId);
        }
    }
    // CDROID-private drag states (0x010dxxxx IDs).
    if (atts.hasAttribute("state_drag_hoved")) {
        const bool val = atts.getBoolean("state_drag_hoved", false);
        states.push_back(val ? 0x010d010b : -0x010d010b);
    }
    if (atts.hasAttribute("state_drag_acceptable")) {
        const bool val = atts.getBoolean("state_drag_acceptable", false);
        states.push_back(val ? 0x010d010a : -0x010d010a);
    }
    return (int)states.size();
}

}
