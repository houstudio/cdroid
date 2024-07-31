#include <cdroid.h>
#include <cdlog.h>
#include <view/gesturedetector.h>

class MyWindow:public Window{
private:
    GestureDetector*mDetector;
public:
    MyWindow(int x,int y,int w,int h):Window(x,y,w,h){
        GestureDetector::OnGestureListener gl;
        gl.onDown=std::bind(&MyWindow::onDown,this,std::placeholders::_1);
        gl.onSingleTapUp=std::bind(&MyWindow::onSingleTapUp,this,std::placeholders::_1);
        gl.onShowPress=std::bind(&MyWindow::onShowPress,this,std::placeholders::_1);
        gl.onLongPress=std::bind(&MyWindow::onLongPress,this,std::placeholders::_1);
        mDetector=new GestureDetector(mContext,gl);
        GestureDetector::OnDoubleTapListener dl;
        dl.onSingleTapConfirmed=std::bind(&MyWindow::onSingleTapConfirmed,this,std::placeholders::_1);
        dl.onDoubleTap=std::bind(&MyWindow::onDoubleTap,this,std::placeholders::_1);
        dl.onDoubleTapEvent=std::bind(&MyWindow::onDoubleTapEvent,this,std::placeholders::_1);
        setOnTouchListener([this](View& v, MotionEvent& event){
            return mDetector->onTouchEvent(event); 
        });
        mDetector->setOnDoubleTapListener(dl);
    }
    void onDraw(Canvas&canvas)override{
        LOGD("%p onDraw",this);
        Rect rc={0,0,getWidth(),getHeight()};
        canvas.set_source_rgb(1,0,0);
        for(int i=0;i<10;i++){
            canvas.rectangle(rc.left,rc.top,rc.width,rc.height);
            canvas.stroke();
            rc.inflate(-10,-10);
        }
    }
    void onLongPress(MotionEvent&e){
       LOGD("onLongPress");
    } 
	bool onDown(MotionEvent& e) {
		LOGD("onDown");
		return false;
	}
 
	bool onSingleTapUp(MotionEvent& e) {
		LOGD("");
		return false;
	}
    void onShowPress(MotionEvent&){
        LOGD("");
    }
////////////////////////////////////////////////////
    bool onSingleTapConfirmed(MotionEvent& e) {
		LOGD("onSingleTapConfirmed");   
		return true;
	}

	bool onDoubleTap(MotionEvent& e) {
		LOGD("onDoubleTap");   
		return true;
	}

	bool onDoubleTapEvent(MotionEvent& e) {
		LOGD("onDoubleTapEvent");   
		return true;
	}    
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new MyWindow(0,0,-1,-1);
    /*AnimatedImageDrawable*ad=new AnimatedImageDrawable(&app,"./test.webp");
    w->setBackground(ad);
    ad->start();*/
    return app.exec();
}
