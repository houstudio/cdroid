#ifndef __CHECKABLE_H__
#define __CHECKABLE_H__
#include <core/callbackbase.h>
namespace cdroid{
class Checkable{
public:
    CallbackBase<void,bool>setChecked;//virtual void setChecked(bool checked)=0;
    CallbackBase<bool>isChecked;//virtual bool isChecked()const=0;
    CallbackBase<void>toggle;//virtual void toggle()=0;
};
}
#endif
