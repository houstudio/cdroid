#include <widget/actionbar.h>
namespace cdroid{


ActionBar::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
    :ViewGroup::MarginLayoutParams(c, attrs){
    gravity = attrs.getGravity("layout_gravity",Gravity::NO_GRAVITY);
}

ActionBar::LayoutParams::LayoutParams(int width, int height)
  :ViewGroup::MarginLayoutParams(width, height){
    this->gravity = Gravity::CENTER_VERTICAL | Gravity::START;
}

ActionBar::LayoutParams::LayoutParams(int width, int height, int gravity)
  :ViewGroup::MarginLayoutParams(width, height){
    this->gravity = gravity;
}

ActionBar::LayoutParams::LayoutParams(int gravity)
   :LayoutParams(WRAP_CONTENT, MATCH_PARENT, gravity){
}

ActionBar::LayoutParams::LayoutParams(const ActionBar::LayoutParams& source)
   :ViewGroup::MarginLayoutParams(source){
    this->gravity = source.gravity;
}

ActionBar::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
  :ViewGroup::MarginLayoutParams(source){
}

}//namespace

