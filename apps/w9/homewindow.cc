#include <homewindow.h>
#include <washoptions.h>
#include <cdroid.h>
#include <R.h>
HomeWindow::HomeWindow():Window(0,0,1280,480){
   LayoutInflater::from(getContext())->inflate("@layout/content_main",this);
   RelativeLayout*rl=(RelativeLayout*)findViewById(w9::R::id::relativeLayout);
   View*v=findViewById(w9::R::id::horizontalScroll);
   v->setHorizontalScrollBarEnabled(true);
   v->setOnScrollChangeListener([](View& view, int x, int y, int oldX, int oldY){
      //LOGD("x=%d->%d",oldX,x);
   });
   LinearLayout*ll=(LinearLayout*)findViewById(w9::R::id::linearLayout);
   const char* res[]={
	   "@mipmap/zhinengxi",
	   "@mipmap/chenshan",
	   "@mipmap/chujun",
	   "@mipmap/husexi",
	   "@mipmap/mianma",
	   "@mipmap/niuzai",
	   "@mipmap/tuoshui",
	   "@mipmap/zhenqihuli",
	   "@mipmap/zhensi",
	   "@mipmap/zhiyanqinxin",
	   nullptr
   };
   for(int i=0;res[i];i++){
	ImageView*img=new ImageView(100,100);
	LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
	lp->gravity=Gravity::CENTER_VERTICAL;
	ll->addView(img,lp).setId(100+i);
	img->setImageResource(res[i]);
	img->setOnClickListener(std::bind(&HomeWindow::onClick,this,std::placeholders::_1));
   }
   View::OnLayoutChangeListener layoutlistener={[](View& v, int left, int top, int width, int height,
		   int oldLeft, int oldTop, int oldwidth, int oldheight){
       v.setRotation(270.0f);
       v.setTranslationX(-0);
       v.setTranslationY(-160);
       LOGD("LayoutChangeListener size=%dx%d",width,height);
   }};
   //rl->addOnLayoutChangeListener(layoutlistener);
}

void HomeWindow::onClick(View&v){
   switch(v.getId()){
   case 100:
	   new WashOptionsWindow(-1);
	   break;
   default:
	   LOGD("TODO for %d",v.getId());
	   break;
   }
}
