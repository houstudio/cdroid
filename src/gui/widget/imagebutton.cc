#include<widget/imagebutton.h>
#include<cdlog.h>
#include<app.h>
namespace cdroid{

DECLARE_WIDGET(ImageButton)

ImageButton::ImageButton(Context*ctx,const AttributeSet& attrs)
  :ImageView(ctx,attrs){
   setFocusable(true);
}

ImageButton::ImageButton(int w,int h):ImageView(w,h){
   setFocusable(true);
}

}
