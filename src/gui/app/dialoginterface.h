#ifndef __DIALOG_INTERFACE_H__
#define __DIALOG_INTERFACE_H__
namespace cdroid{

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
    DECLARE_UIEVENT(void,OnCancelListener,DialogInterface&);
    DECLARE_UIEVENT(void,OnDismissListener,DialogInterface&);
    DECLARE_UIEVENT(void,OnShowListener,DialogInterface&);
    DECLARE_UIEVENT(void,OnClickListener,DialogInterface&,int);
    DECLARE_UIEVENT(void,OnMultiChoiceClickListener,DialogInterface&,int ,bool);
    DECLARE_UIEVENT(void,OnKeyListener,DialogInterface&,int, KeyEvent&);
public:
    virtual void cancel()=0;
    virtual void dismiss()=0;
};

}//endof namespace
#endif
