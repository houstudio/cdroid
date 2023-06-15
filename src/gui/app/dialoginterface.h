#ifndef __DIALOG_INTERFACE_H__
#define __DIALOG_INTERFACE_H__
#include <view/view.h>
namespace cdroid{
class Dialog;
class DialogInterface{
public:
    enum{
       /** The identifier for the positive button. */
       BUTTON_POSITIVE = -1,
       /** The identifier for the negative button. */
       BUTTON_NEGATIVE = -2,
       /** The identifier for the neutral button. */
       BUTTON_NEUTRAL  = -3
    };
    DECLARE_UIEVENT(void,OnCancelListener,Dialog&);
    DECLARE_UIEVENT(void,OnDismissListener,Dialog&);
    DECLARE_UIEVENT(void,OnShowListener,Dialog&);
    DECLARE_UIEVENT(void,OnClickListener,Dialog&,int);
    DECLARE_UIEVENT(void,OnMultiChoiceClickListener,Dialog&,int ,bool);
    DECLARE_UIEVENT(void,OnKeyListener,Dialog&,int, KeyEvent&);
public:
    virtual void cancel()=0;
    virtual void dismiss()=0;
};

}//endof namespace
#endif
