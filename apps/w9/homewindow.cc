#include <homewindow.h>
#include <washoptions.h>
#include <cdroid.h>
#include <R.h>
#include <cdlog.h>

HomeWindow::HomeWindow():Window(0,0,-1,-1){
   LayoutInflater::from(getContext())->inflate("@layout/main",this);

   HorizontalScrollView*sv=(HorizontalScrollView*)findViewById(w9::R::id::horizontalScroll);
   View*v=findViewById(w9::R::id::horizontalScroll);
   v->setHorizontalScrollBarEnabled(true);
   v->setOnScrollChangeListener([](View& view, int x, int y, int oldX, int oldY){
      //LOGD("x=%d->%d",oldX,x);
   });
   screenWidth=getWidth();
   mCurScrollX=mOldScrollX=0;
   v->setOnTouchListener([this](View&v, MotionEvent&event){
         switch (event.getAction()) {
	 case MotionEvent::ACTION_DOWN:
               mOldScrollX = v.getScrollX();
               break;
	 case MotionEvent::ACTION_UP:
               mCurScrollX = v.getScrollX();
               //每次手滑动距离大于200或者小于-200才可以触发ScrolView滚动一屏幕距离，否则恢复原位
	       LOGD("distance=%d mCurScrollX=%d",std::abs(mCurScrollX - mOldScrollX),mCurScrollX);
               if ((mCurScrollX - mOldScrollX) > 200) {
                   v.scrollTo(mOldScrollX + screenWidth, 0);
               } else if ((mCurScrollX - mOldScrollX) < -200) {
                   v.scrollTo(mOldScrollX - screenWidth, 0);
               } else {
                   v.scrollTo(mOldScrollX, 0);
               }
               break;
         }
         return false;
   });
   
   v=findViewById(w9::R::id::often);
   auto btn_listener=std::bind(&HomeWindow::onButtonClick,this,std::placeholders::_1);
   v->setOnClickListener(btn_listener);
   v=findViewById(w9::R::id::care);
   v->setOnClickListener(btn_listener);
   v=findViewById(w9::R::id::favorite);
   if(v)v->setOnClickListener(btn_listener);

   LinearLayout*ll=(LinearLayout*)findViewById(w9::R::id::linearLayout);
   const char* res[]={
	   "@mipmap/zhinengxi",	   "@mipmap/chenshan",	   "@mipmap/chujun",	   "@mipmap/husexi",
	   "@mipmap/mianma",	   "@mipmap/niuzai",	   "@mipmap/tuoshui",	   "@mipmap/zhenqihuli",
	   "@mipmap/zhensi",	   "@mipmap/zhiyanqinxin",   nullptr
   };
   const char* texts[]={"智能洗","衬衫","除菌","护色洗","棉麻","牛仔","脱水","蒸汽护理","真丝","智氧清洗"};
   for(int i=0;res[i];i++){
	LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
	lp->gravity=Gravity::CENTER_VERTICAL;
	TextView*txt=new TextView(texts[i],100,100);
	txt->setGravity(Gravity::BOTTOM|Gravity::CENTER_HORIZONTAL);
	txt->setTextSize(36);
	ll->addView(txt,lp).setId(100+i);
	txt->setBackgroundResource(res[i]);
	txt->setOnClickListener(std::bind(&HomeWindow::onWashOptionClick,this,std::placeholders::_1));
   }
   ll->requestLayout();

   anim=ValueAnimator::ofFloat({1.f,0.f});
   anim->setDuration(200);
   LOGD("anim=%p",anim);
   /*anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        const float t=anim.getAnimatedValue().get<float>();
        this->setPos(0,this->getHeight()*(t));
        this->setAlpha(1.f-t);this->setAlpha(t);
   }));
   anim->start();*/
}

HomeWindow::~HomeWindow(){
   delete anim;
}
void HomeWindow::onButtonClick(View&v){
    LOGD("click %d",v.getId());
}

void HomeWindow::onWashOptionClick(View&v){
    switch(v.getId()){
    case 100:{
	       Window*w=new WashOptionsWindow(-1);
	       anim->removeAllUpdateListeners();
               anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([w,this](ValueAnimator&anim){
                    const float t=anim.getAnimatedValue().get<float>();
                    w->setPos(0*w->getWidth()*t,w->getHeight()*(t));
		    w->setAlpha(1.f-t);this->setAlpha(t);
		    LOGV("top=%d ",w->getLeft(),w->getTop());
               }));
               anim->start();
	   }break;
    default:
	   LOGD("TODO for %d",v.getId());
	   break;
   }
}
