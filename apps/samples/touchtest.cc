#include <cdroid.h>
#include <core/inputdevice.h>
#include <porting/cdinput.h>
#include <linux/input.h>
class TouchWindow:public Window{
private:
    std::map<int,Point> mTouchPoints;
    int mGridSize;
    bool mClear;
protected:
    void drawGrid(Canvas&canvas){
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
    }
public:
    TouchWindow(int w,int h):Window(0,0,w,h){
	 mGridSize =10;
	 mClear = false;
    }
#define R 6
    bool onTouchEvent(MotionEvent&event)override{
        const int pointer = event.getActionIndex();
	const int x = event.getX(pointer);
	const int y = event.getY(pointer);
        switch(event.getActionMasked()){
        case MotionEvent::ACTION_UP:
            mClear = event.getActionMasked()==MotionEvent::ACTION_UP&&(event.getX()<20)&&(event.getY()<20);
            if(mClear)invalidate();
            else invalidate(x-R,y-R,R*2,R*2);
	    break;
        case MotionEvent::ACTION_DOWN:
	case MotionEvent::ACTION_POINTER_DOWN:{
		auto it = mTouchPoints.find(pointer);
		if(it==mTouchPoints.end()){
                    it=mTouchPoints.insert({pointer,{x,y}}).first;
		}else{
		    Point& npt=it->second;
                    npt.set(x,y);
		}
		invalidate(x-R,y-R,R*2,R*2);
	    }break;
	case MotionEvent::ACTION_MOVE:
            for(int i=0;i<event.getPointerCount();i++){
	        auto it = mTouchPoints.find(i);
		if(it!=mTouchPoints.end()){
		    it->second.set(x,y);
		}
            }invalidate(x-R,y-R,R*2,R*2);
            break;
	default:break;
	}
	return true;
    }
    void onDraw(Canvas&canvas)override{
	if(mClear){
	    canvas.set_source_rgb(0,0,0);
	    canvas.paint();
	}
	drawGrid(canvas);
	for(auto it=mTouchPoints.begin();it!=mTouchPoints.end();it++){
	    Point& pt = it->second;
	    canvas.set_source_rgb(0,1,float(it->first)/mTouchPoints.size());
	    canvas.arc(pt.x,pt.y,R,0,M_PI*2.f);
	    canvas.fill();
	}
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
