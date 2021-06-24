#ifndef __CHECKABLE_H__
#define __CHECKABLE_H__
namespace cdroid{
class Checkable{
public:
    virtual void setChecked(bool checked)=0;
    virtual bool isChecked()const=0;
    virtual void toggle()=0;
};
}
#endif
