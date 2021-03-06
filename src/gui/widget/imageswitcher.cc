#include <widget/imageswitcher.h>
#include <widget/imageview.h>

namespace cdroid{

DECLARE_WIDGET(ImageSwicther)

ImageSwicther::ImageSwicther(int w,int h)
    :ViewSwitcher(w,h){
}

ImageSwicther::ImageSwicther(Context*ctx,const AttributeSet&atts)
  :ViewSwitcher(ctx,atts){
}

void ImageSwicther::setImageResource(const std::string&resid){
    ImageView* image = (ImageView*)getNextView();
    image->setImageResource(resid);
    showNext();
}

void ImageSwicther::setImageDrawable(Drawable* drawable){
    ImageView* image = (ImageView*)getNextView();
    image->setImageDrawable(drawable);
    showNext();
}

}
