#ifndef __ACTION_BAR_H__
#define __ACTION_BAR_H__
#include <widget/viewgroup.h>
namespace cdroid{

class ActionBar{
public:
    class LayoutParams:public ViewGroup::MarginLayoutParams{
    public:
        int gravity = Gravity::NO_GRAVITY;
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, int gravity);
        LayoutParams(int gravity);
        LayoutParams(const LayoutParams& source);
        LayoutParams(const ViewGroup::LayoutParams& source);
    };
private:
public:
};
}//namespace

#endif
