#include <cdroid.h>
#include <core/inputdevice.h>
#include <porting/cdinput.h>
#include <linux/input.h>
class TouchWindow:public Window{
private:
    struct TouchPoint{
        int x0,y0;
	int x1,y1;
    };
    std::map<int,TouchPoint> mTouchPoints;
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
	 mClear =true;
    }

    bool onTouchEvent(MotionEvent&event)override{
        switch(event.getActionMasked()){
        case MotionEvent::ACTION_UP:
            mClear = event.getActionMasked()==MotionEvent::ACTION_UP;
            invalidate();
	    break;
        case MotionEvent::ACTION_DOWN:
	case MotionEvent::ACTION_POINTER_DOWN:{
	        int pointer = event.getActionIndex();
		auto it = mTouchPoints.find(pointer);
		if(it==mTouchPoints.end()){
		    TouchPoint pt={-1,-1,-1,-1};
		    pt.x1=event.getX(pointer);
		    pt.y1=event.getY(pointer);
                    it=mTouchPoints.insert({pointer,pt}).first;
		}else{
		    TouchPoint& npt=it->second;
                    npt.x1=event.getX(pointer);
		    npt.y1=event.getY(pointer);
		}
		invalidate();
	    }break;
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
	    TouchPoint& pt = it->second;
	    if( (pt.x0>=0) && (pt.y0>=0) ){
	        canvas.set_source_rgb(0,1,float(it->first)/mTouchPoints.size());
		canvas.move_to(pt.x0,pt.y0);
	        canvas.line_to(pt.x1,pt.y1);
	        canvas.stroke();
	    }
	    pt.x0 = pt.x1;
	    pt.y0 = pt.y1;
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
