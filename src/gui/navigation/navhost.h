#ifndef __NAVHOST_H__
#define __NAVHOST_H__
/*********************************************************************************
 * Port of androidx.navigation.NavHost. A host that owns a NavController.
 *********************************************************************************/
namespace cdroid{
class NavController;
class NavHost{
public:
    virtual ~NavHost() = default;
    virtual NavController* getNavController() = 0;
};
}//namespace cdroid
#endif
