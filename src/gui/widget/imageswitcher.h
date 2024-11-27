#ifndef __IMAGESWITCHER_H__
#define __IMAGESWITCHER_H__
#include <widget/viewswitcher.h>

namespace cdroid{

class ImageSwitcher:public ViewSwitcher{
public:
    ImageSwitcher(int w,int h);
    ImageSwitcher(Context*ctx,const AttributeSet&atts);
    void setImageResource(const std::string&res);
    void setImageDrawable(Drawable* drawable);
    std::string getAccessibilityClassName()const override;
};

}

#endif
