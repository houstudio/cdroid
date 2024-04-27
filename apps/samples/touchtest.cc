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

    bool onTouchEvent(MotionEvent&event)override{
        switch(event.getActionMasked()){
        case MotionEvent::ACTION_UP:
            mClear = event.getActionMasked()==MotionEvent::ACTION_UP&&(event.getX()<20)&&(event.getY()<20);
            invalidate();
	    break;
        case MotionEvent::ACTION_DOWN:
	case MotionEvent::ACTION_POINTER_DOWN:{
	        int pointer = event.getActionIndex();
		auto it = mTouchPoints.find(pointer);
		if(it==mTouchPoints.end()){
		    Point pt;
		    pt.x=event.getX(pointer);
		    pt.y=event.getY(pointer);
                    it=mTouchPoints.insert({pointer,pt}).first;
		}else{
		    Point& npt=it->second;
                    npt.x=event.getX(pointer);
		    npt.y=event.getY(pointer);
		}
		invalidate();
	    }break;
	case MotionEvent::ACTION_MOVE:
            for(int i=0;i<event.getPointerCount();i++){
	        auto it = mTouchPoints.find(i);
		if(it!=mTouchPoints.end()){
		    it->second.x=event.getX(i);
                    it->second.y=event.getY(i);
		}
            }invalidate();
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
	    canvas.arc(pt.x,pt.y,6,0,M_PI*2.f);
	    LOGD("(%d,%d)clear=%d",pt.x,pt.y,mClear);
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
