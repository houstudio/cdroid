#include <widget/actionbar.h>
#include <stdexcept>
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

// Concrete bit-toggle convenience methods. Each flips a single display bit by
// delegating to the pure-virtual setDisplayOptions(options, mask).
void ActionBar::setDisplayUseLogoEnabled(bool useLogo){
    setDisplayOptions(useLogo ? DISPLAY_USE_LOGO : 0, DISPLAY_USE_LOGO);
}

void ActionBar::setDisplayShowHomeEnabled(bool showHome){
    setDisplayOptions(showHome ? DISPLAY_SHOW_HOME : 0, DISPLAY_SHOW_HOME);
}

void ActionBar::setDisplayHomeAsUpEnabled(bool showHomeAsUp){
    setDisplayOptions(showHomeAsUp ? DISPLAY_HOME_AS_UP : 0, DISPLAY_HOME_AS_UP);
}

void ActionBar::setDisplayShowTitleEnabled(bool showTitle){
    setDisplayOptions(showTitle ? DISPLAY_SHOW_TITLE : 0, DISPLAY_SHOW_TITLE);
}

void ActionBar::setDisplayShowCustomEnabled(bool showCustom){
    setDisplayOptions(showCustom ? DISPLAY_SHOW_CUSTOM : 0, DISPLAY_SHOW_CUSTOM);
}

void ActionBar::setDefaultDisplayHomeAsUpEnabled(bool /*enabled*/){
    // Default no-op; honoured only before a decor ActionBar is constructed, which
    // CDROID does not build.
}

// Deprecated Tab/List navigation — not supported under the Toolbar-backed ActionBar.
void ActionBar::setNavigationMode(int mode){
    if(mode == NAVIGATION_MODE_TABS || mode == NAVIGATION_MODE_LIST){
        throw std::runtime_error(
            "ActionBar: Tab/List navigation is deprecated and unsupported; use Toolbar + TabLayout");
    }
}

void* ActionBar::newTab(){
    throw std::runtime_error(
        "ActionBar: Tabs are deprecated and unsupported; use Toolbar + TabLayout");
}

}//namespace
