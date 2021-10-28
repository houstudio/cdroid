#include <widget/viewswitcher.h>

namespace cdroid{

ViewSwitcher::ViewSwitcher(int w,int h)
   :ViewAnimator(w,h){
}

ViewSwitcher::ViewSwitcher(Context*ctx,const AttributeSet&atts)
  :ViewAnimator(ctx,atts){
}

View& ViewSwitcher::addView(View* child, int index, ViewGroup::LayoutParams* params){
    if (getChildCount() >= 2) {
        throw "Can't add more than 2 views to a ViewSwitcher";
    }
    return ViewAnimator::addView(child, index, params);
}

View* ViewSwitcher::getNextView() {
    int which = mWhichChild == 0 ? 1 : 0;
    return getChildAt(which);
}

View* ViewSwitcher::obtainView() {
    View* child = mFactory();
    ViewGroup::LayoutParams* lp = (ViewGroup::LayoutParams*) child->getLayoutParams();
    if (lp == nullptr) {
        lp = new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);
    }
    addView(child,getChildCount(), lp);
    return child;
}

void ViewSwitcher::setFactory(ViewFactory factory) {
    mFactory = factory;
    obtainView();
    obtainView();
}

void ViewSwitcher::reset() {
    mFirstTime = true;
    View* v= getChildAt(0);
    if (v) v->setVisibility(View::GONE);
    v = getChildAt(1);
    if (v) v->setVisibility(View::GONE);
}

}//namespace

