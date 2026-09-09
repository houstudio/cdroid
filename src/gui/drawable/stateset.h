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
#ifndef __STATESET_H__
#define __STATESET_H__
#include <vector>
#include <core/attributeset.h>

namespace cdroid{

class StateSet{
private:
    // AOSP View.java VIEW_STATE_* bits (for getDrawableState's bitmask → state-set).
    static std::vector<int>VIEW_STATE_IDS;
public:
    // AOSP StateSet.VIEW_STATE_* bits — exactly the base View set. checked/
    // checkable/single/first/middle/last are appended by subclasses via
    // View.mergeDrawableStates() with their own static state sets.
    enum{
        VIEW_STATE_WINDOW_FOCUSED =1<<0 ,
        VIEW_STATE_SELECTED       =1<<1 ,
        VIEW_STATE_FOCUSED        =1<<2 ,
        VIEW_STATE_ENABLED        =1<<3 ,
        VIEW_STATE_PRESSED        =1<<4 ,
        VIEW_STATE_ACTIVATED      =1<<5 ,
        VIEW_STATE_ACCELERATED    =1<<6 ,
        VIEW_STATE_HOVERED        =1<<7 ,
        VIEW_STATE_DRAG_CAN_ACCEPT=1<<8 ,
        VIEW_STATE_DRAG_HOVERED   =1<<9 ,
    };
    // AOSP-aligned: state values ARE the R.attr IDs (0x010100xx). StateListDrawable
    // items carry these IDs; stateSetMatches compares them by equality. CDROID-private
    // states (drag_hoved/drag_acceptable) use their CDROID-private 0x010dxxxx IDs.
    // The sequential ints (1-15) are RETIRED — replaced by real resource IDs.
    static const std::vector<int>NOTHING;
    static const std::vector<int>WILD_CARD;
    static const std::vector<int>ENABLED_STATE_SET;
    static const std::vector<int>PRESSED_STATE_SET;
    static const std::vector<int>FOCUSED_STATE_SET;
    static const std::vector<int>SELECTED_STATE_SET;
    static const std::vector<int>CHECKED_STATE_SET;
public:
    static void trimStateSet(std::vector<int>&states,int newsize);
    static bool isWildCard(const std::vector<int>&stateSetOrSpec);
    static bool stateSetMatches(const std::vector<int>&stateSpec,const std::vector<int>&stateSet);
    static bool stateSetMatches(const std::vector<int>&stateSpec,int state);
    static bool containsAttribute(const std::vector<std::vector<int>>&stateSpecs,int attr);
    static std::vector<int> get(int mask);
};
}
#endif
