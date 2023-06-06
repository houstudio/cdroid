#include<widget/imagebutton.h>
#include<cdlog.h>
#include<app.h>
namespace cdroid{

DECLARE_WIDGET2(ImageButton,"cdroid:attr/imageButtonStyle")

ImageButton::ImageButton(Context*ctx,const AttributeSet& attrs)
  :ImageView(ctx,attrs){
}

ImageButton::ImageButton(int w,int h):ImageView(w,h){
}

}
