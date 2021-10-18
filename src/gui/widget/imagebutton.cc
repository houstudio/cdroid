#include<widget/imagebutton.h>
#include<cdlog.h>
#include<app.h>
namespace cdroid{

ImageButton::ImageButton(Context*ctx,const AttributeSet& attrs,const std::string&defstyle)
  :ImageView(ctx,attrs,defstyle){
   setFocusable(true);
}

ImageButton::ImageButton(int w,int h):ImageView(w,h){
   setFocusable(true);
}

}
