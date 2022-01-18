#include<cdroid.h>

static const char*texts[]={"Creates 中国智造"," the specified format and dimensions.",
            "Initially the surface contents"," are set to 0.","(Specifically, within each pixel,",
            " each color or alpha channel","belonging to format will be 0.","The contents","of bits within a pixel,",
            " but not belonging","必须使用UTF8编码 " };

class MWindow:public Window{
private:
   int dx,dy;
public:
   MWindow(int x,int y,int w,int h):Window(x,y,w,h){dx=dy=50;}
   virtual bool onMessage(DWORD msgid,DWORD wp,ULONG lp){
      if(1000==msgid){
         //sendMessage(msgid,wp,lp,lp);
         int x=getX();
         int y=getY();
         if(x+getWidth()>1280)dx*=-1;
         if(y+getHeight()>720)dy*=-1;
         if(x+dx<0)dx*=-1;
         if(y+dy<0)dy*=-1;
         setPos(x+dx,y+dy);
         return true;
      }
      return true;//Window::onMessage(msgid,wp,lp);
   }
   void onDraw(Canvas&c)override{
       srand(time(nullptr)*(long)this);
       //setBgColor(0xFF000000|rand());
       Window::onDraw(c); 
   }
   void setDir(int x,int y){dx=x;dy=y;}
};


int main(int argc,const char*argv[]){
    App app(argc,argv);
    MWindow*w1=new MWindow(100,100,640,480);
    MWindow*w2=new MWindow(300,200,480,320);

    MWindow*w3=new MWindow(123,211,240,160);
    ArrayAdapter<std::string>*mAdapter=new  ArrayAdapter<std::string>();
    ListView *lv1=new ListView(600,200);
    ListView *lv2=new ListView(400,200);
    lv1->setAdapter(mAdapter);

    for(int j=0;j<8;j++){
        mAdapter->add(texts[j]);
    }
    printf("lv1=%p lv2=%p\r\n",lv1,lv2);
    w1->addView(lv1).setId(0);
    w2->addView(lv2).setId(0);
    
    if(argc>5){
        //w1->sendMessage(1000,0,2000,100);w1->setDir(50,40);
        //w2->sendMessage(1000,0,1000,80);w2->setDir(66,53);
        //w3->sendMessage(1000,0,200,85);w3->setDir(23,13);
    }
    return app.exec();
}
