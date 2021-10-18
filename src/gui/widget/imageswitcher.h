#ifndef __IMAGESWITCHER_H__
#define __IMAGESWITCHER_H__
#include <widget/viewswitcher.h>

namespace cdroid{

class ImageSwicther:public ViewSwitcher{
public:
    ImageSwicther(int w,int h);
    ImageSwicther(Context*ctx,const AttributeSet&atts,const std::string&defstyle=nullptr);
    void setImageResource(const std::string&res);
    void setImageDrawable(Drawable* drawable);
};

}

#endif
