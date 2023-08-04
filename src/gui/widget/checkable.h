#ifndef __CHECKABLE_H__
#define __CHECKABLE_H__
#include <core/callbackbase.h>
namespace cdroid{
//#define FUNCTION_AS_CHECKABLE 1
class Checkable{
public:
#ifdef FUNCTION_AS_CHECKABLE
    CallbackBase<void,bool>setChecked;
    CallbackBase<bool>isChecked;
    CallbackBase<void>toggle;
#else
    virtual void setChecked(bool checked)=0;
    virtual bool isChecked()const=0;
    virtual void toggle()=0;
#endif
};
}
#endif
