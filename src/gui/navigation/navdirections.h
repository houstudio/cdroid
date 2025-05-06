#ifndef __NAV_DIRECTIONS_H__
#define __NAV_DIRECTIONS_H__
namespace cdroid{
class NavDirections {
public:
    /**
     * Returns a action id to navigate with.
     *
     * @return id of an action
     */
    virtual int getActionId()=0;

    /**
     * Returns arguments to pass to the destination
     */
    virtual Bundle* getArguments()=0;
};
}
#endif/*__NAV_DIRECTIONS_H__*/
