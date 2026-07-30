#ifndef __NOOPNAVIGATOR_H__
#define __NOOPNAVIGATOR_H__
/*********************************************************************************
 * Port of androidx.navigation.NoOpNavigator. A placeholder navigator that creates
 * destinations but never actually navigates (used by NavDeepLinkBuilder fallback).
 *********************************************************************************/
#include <navigation/navigator.h>
namespace cdroid{

class NoOpNavigator : public Navigator{
public:
    NoOpNavigator(){ mName = "NoOp"; }
    NavDestination* createDestination() override;
};

}//namespace cdroid
#endif
