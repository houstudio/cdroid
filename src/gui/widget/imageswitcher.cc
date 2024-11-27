#include <widget/imageswitcher.h>
#include <widget/imageview.h>

namespace cdroid{

DECLARE_WIDGET(ImageSwitcher)

ImageSwitcher::ImageSwitcher(int w,int h)
    :ViewSwitcher(w,h){
}

ImageSwitcher::ImageSwitcher(Context*ctx,const AttributeSet&atts)
  :ViewSwitcher(ctx,atts){
}

void ImageSwitcher::setImageResource(const std::string&resid){
    ImageView* image = (ImageView*)getNextView();
    image->setImageResource(resid);
    showNext();
}

void ImageSwitcher::setImageDrawable(Drawable* drawable){
    ImageView* image = (ImageView*)getNextView();
    image->setImageDrawable(drawable);
    showNext();
}

std::string ImageSwitcher::getAccessibilityClassName()const{
    return "ImageSwitcher";
}

}
