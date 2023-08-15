#include <cdroid.h>
class TouchWindow:public Window{
private:
    Point mTouchPoint;
    int mGridSize;
    bool mClear;
public:
    TouchWindow(int w,int h):Window(0,0,w,h){
	 mGridSize =10;
    }
    bool onTouchEvent(MotionEvent&event)override{
        mTouchPoint.x = event.getX();
	mTouchPoint.y = event.getY();
	mClear = event.getActionMasked()==MotionEvent::ACTION_UP;
	LOGI("Point(%d,%d)",mTouchPoint.x,mTouchPoint.y);
	invalidate();
	return true;
    }
    void onDraw(Canvas&canvas){
	if(mClear){
	    canvas.set_source_rgb(0,0,0);
	    canvas.rectangle(0,0,getWidth(),getHeight());
	    canvas.fill();
	}
        for(int x=0,i=0;x<getWidth();x+=mGridSize,i++){
	    canvas.move_to(x,0);
	    canvas.line_to(x,getHeight());
	    canvas.set_source_rgba(.5,.5,.5,(i%10==0)?1.f:0.4f);
	    canvas.stroke();
	}
	for(int y=0,i=0;y<getHeight();y+=mGridSize,i++){
	    canvas.move_to(0,y);
	    canvas.line_to(getWidth(),y);
	    canvas.set_source_rgba(.5,.5,.5,(i%10==0)?1.f:0.4f);
	    canvas.stroke();
	}
	canvas.stroke();
	canvas.set_source_rgb(0,1,0);
	canvas.arc(mTouchPoint.x,mTouchPoint.y,6,0,M_PI*2.f);
	canvas.fill();
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Point sz;
    WindowManager::getInstance().getDefaultDisplay().getRealSize(sz);
    LOGI("DisplaySize=%dx%d",sz.x,sz.y);
    Window*w=new TouchWindow(-1,-1);
    w->invalidate();
    return app.exec();
}
