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
        DRAG_HOVERED   = 11
    };
    static const std::vector<int>NOTHING;
    static const std::vector<int>WILD_CARD;
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
