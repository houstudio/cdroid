#ifndef __IMAGE_BUTTON_H__
#define __IMAGE_BUTTON_H__
#include <widget/button.h>
#include <widget/imageview.h>
namespace cdroid{

class ImageButton:public ImageView{
public:
    ImageButton(Context*ctx,const AttributeSet& attrs);
    ImageButton(int w,int h);
};

}//namespace
#endif

