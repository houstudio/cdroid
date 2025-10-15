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
    static std::vector<int>VIEW_STATE_IDS;
    static void appendState(std::vector<int>&states,const std::string&s,int value);
public:
    enum{
        VIEW_STATE_WINDOW_FOCUSED =1<<0 ,
        VIEW_STATE_SELECTED       =1<<1 ,
        VIEW_STATE_FOCUSED        =1<<2 ,
        VIEW_STATE_ENABLED        =1<<3 ,
        VIEW_STATE_PRESSED        =1<<4 ,
        VIEW_STATE_ACTIVATED      =1<<5 ,
        VIEW_STATE_HOVERED        =1<<6 ,
        VIEW_STATE_CHECKED        =1<<7 ,
        VIEW_STATE_CHECKABLE      =1<<8 , 
        VIEW_STATE_DRAG_CAN_ACCEPT=1<<9 ,
        VIEW_STATE_DRAG_HOVERED   =1<<10,

        VIEW_STATE_SINGLE         =1<<11,
        VIEW_STATE_FIRST          =1<<12,
        VIEW_STATE_MIDDLE         =1<<13,
        VIEW_STATE_LAST           =1<<14
    };
    enum{
        WINDOW_FOCUSED = 1 ,
        SELECTED       = 2 ,
        FOCUSED        = 3 ,
        ENABLED        = 4 ,
        PRESSED        = 5 ,
        ACTIVATED      = 6 ,
        HOVERED        = 7 ,
        CHECKED        = 8 ,
        CHECKABLE      = 9 ,
        DRAG_ACCPETABLE= 10,
        DRAG_HOVERED   = 11,

        SINGLE    = 12,
        FIRST     = 13,
        MIDDLE    = 14,
        LAST      = 15
    };
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
    static int parseState(std::vector<int>&states,const AttributeSet&attss);
    static std::vector<int> get(int mask);
};
}
#endif
