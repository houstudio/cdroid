#ifndef __VIEWSWITCHER_H__
#define __VIEWSWITCHER_H__
#include <widget/viewanimator.h>

namespace cdroid{

class ViewSwitcher:public ViewAnimator{
public:
    typedef std::function<View*()>ViewFactory;
private:
    ViewFactory mFactory;
    View*obtainView();
public:
    ViewSwitcher(int w,int h);
    ViewSwitcher(Context*ctx,const AttributeSet&atts,const std::string&defstyle=nullptr);
    View& addView(View* child, int index, ViewGroup::LayoutParams* params);
    View* getNextView();
    void setFactory(ViewFactory factory);
    void reset();
};

}

#endif
