#include<windows.h>
#include<cdlog.h>


class AnimateWindow:public Window{
protected:
   Matrix matrix;
   double cx;
public :
   AnimateWindow(int x,int y,int w,int h):Window(x,y,w,h){
      cx=0.2f;
      matrix=identity_matrix();
   }
   
   void onDraw(Canvas&canvas){
      canvas.set_source_rgb(1,0,0);
      RECT r=getClientRect();
      canvas.set_matrix(matrix);
      Window::onDraw(canvas);
      canvas.rectangle(0,0,getWidth()-1,getHeight()-1);
      canvas.stroke();
   }

   virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp)override{
       if(msg==1000){
           matrix.scale(cx,cx);cx+=0.01f;
           matrix.rotate(cx);
           invalidate(NULL);
           sendMessage(msg,0,0,500);
           return true;
       }
       return Window::onMessage(msg,wp,lp);;
   }
};

int main(int argc,const char*argv[]){

    App app(argc,argv);
    Window*w=new AnimateWindow(100,100,800,600);
  
    w->addView(new TextView("HelloWorld",400,80));
    
    w->sendMessage(1000,0,0,500);
    return app.exec();
}


