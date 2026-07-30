#ifndef __FRAGMENTHOSTCALLBACK_H__
#define __FRAGMENTHOSTCALLBACK_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentHostCallback<H>. CDROID host type is
 * fixed to Window (typedef Window Activity), so this is non-template: onGetHost()
 * returns cdroid::Window*. Subclassed by the host (FragmentWindow::HostCallbacks).
 *********************************************************************************/
#include <fragment/fragmentcontainer.h>
namespace cdroid{
class Context;
class Handler;
class LayoutInflater;
class Window;
namespace fragment{

class FragmentHostCallback : public FragmentContainer{
public:
    ~FragmentHostCallback() override = default;
    virtual cdroid::Context* getContext() = 0;
    virtual cdroid::Handler* getHandler() = 0;
    virtual cdroid::LayoutInflater* onGetLayoutInflater() = 0;
    virtual cdroid::Window* onGetHost() = 0;
    virtual int onGetWindowAnimations() { return 0; }
    // FragmentContainer
    cdroid::View* onFindViewById(int id) override = 0;
    bool onHasView() override = 0;
};

}//namespace fragment
}//namespace cdroid
#endif
