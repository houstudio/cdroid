#ifndef __TOGGLE_BUTTON_H__
#define __TOGGLE_BUTTON_H__
#include <widget/compoundbutton.h>
namespace cdroid{
class ToggleButton:public CompoundButton{
private:
    float mDisabledAlpha;
    std::string mTextOn;
    std::string mTextOff;
    Drawable*mIndicatorDrawable;
    void syncTextState();
    void updateReferenceToIndicatorDrawable(Drawable* backgroundDrawable);
protected:
    void drawableStateChanged()override;
public:
    ToggleButton(int w,int h);
    ToggleButton(Context*ctx,const AttributeSet& attrs,const std::string&defstyle=nullptr);
    const std::string getTextOn()const;
    void setTextOn(const std::string& textOn);
    const std::string getTextOff()const;
    void setTextOff(const std::string& textOff);
    void setChecked(bool checked)override;
    View& setBackgroundDrawable(Drawable* d)override;
};
}
#endif
