#ifndef __BACKBUTTON_DISMISS_CONTROLLER_H__
#define __BACKBUTTON_DISMISS_CONTROLLER_H__
#include <widgetEx/wear/dismisscontroller.h>
namespace cdroid{
class DismissibleFrameLayout;
class BackButtonDismissController:public DismissController {
private:
    bool dismiss();
public:
    BackButtonDismissController(Context* context, DismissibleFrameLayout* layout);

    void disable(DismissibleFrameLayout* layout);
};
}/*endof namespace*/
#endif/*__BACKBUTTON_DISMISS_CONTROLLER_H__*/
