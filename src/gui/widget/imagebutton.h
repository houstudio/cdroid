#ifndef __IMAGE_BUTTON_H__
#define __IMAGE_BUTTON_H__
#include <widget/button.h>
#include <widget/imageview.h>
namespace cdroid{

class ImageButton:public ImageView{
protected:
    bool onSetAlpha(int alph)override;
public:
    ImageButton(int w,int h);
    ImageButton(Context*ctx,const AttributeSet& attrs);
    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
    std::string getAccessibilityClassName()const override;
};

}//namespace
#endif

