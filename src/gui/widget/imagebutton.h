#ifndef __IMAGE_BUTTON_H__
#define __IMAGE_BUTTON_H__
#include <widget/button.h>
#include <widget/imageview.h>
namespace cdroid{

class ImageButton:public ImageView{
public:
    ImageButton(int w,int h);
    ImageButton(Context*ctx,const AttributeSet& attrs);
    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
};

}//namespace
#endif

