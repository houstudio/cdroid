#ifndef __CHECKABLE_H__
#define __CHECKABLE_H__
#include <core/callbackbase.h>
namespace cdroid{
#define FUNCTION_AS_CHECKABLE 1
//we cant use virtual class as Checkeable,
//it will cause listview's setItemCheck crashed in  ListView::setupChild
class Checkable{
public:
    virtual ~Checkable()=default;
#ifdef FUNCTION_AS_CHECKABLE
    CallbackBase <void,bool> setChecked;
    CallbackBase <bool> isChecked;
    CallbackBase <void> toggle;
#else
    virtual void setChecked(bool checked) = 0;
    virtual bool isChecked()const = 0;
    virtual void toggle() = 0;
#endif
};
}
#endif
