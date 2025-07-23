#ifndef __MENU_HELPER_H__
#define __MENU_HELPER_H__
#include <menu/menupresenter.h>
namespace cdroid{
class MenuHelper {
public:
    virtual void setPresenterCallback(const MenuPresenter::Callback& cb)=0;
    virtual void dismiss()=0;
    virtual ~MenuHelper()=default;
};
}
#endif
