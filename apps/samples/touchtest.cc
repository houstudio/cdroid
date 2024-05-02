#include <cdroid.h>
#include <core/inputdevice.h>
#include <porting/cdinput.h>
#include <linux/input.h>
#include <core/inputeventsource.h>
#include <sstream>
class TouchWindow:public Window{
private:
    std::map<int,std::list<Point>> mTouchPoints;
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
        int minX=INT_MAX,minY=INT_MAX;
        int maxX=INT_MIN,maxY=INT_MIN;
        switch(event.getActionMasked()){
        case MotionEvent::ACTION_UP:
            mClear = event.getActionMasked()==MotionEvent::ACTION_UP&&(event.getX()<20)&&(event.getY()<20);
            if(mClear)invalidate();
            break;
        case MotionEvent::ACTION_DOWN:
        case MotionEvent::ACTION_POINTER_DOWN:
        case MotionEvent::ACTION_MOVE:
            for(int i=0;i<event.getPointerCount();i++){
                auto it = mTouchPoints.find(i);
                int x = event.getX(i);
                int y = event.getY(i);
                minX = std::min(x,minX);
                minY = std::min(y,minY);
                maxX = std::max(x,maxX);
                maxY = std::max(y,maxY);
                if(it == mTouchPoints.end()){
                   it = mTouchPoints.insert({i,std::list<Point>()}).first;
                }
                it->second.push_back({x,y});
                //LOGD("Pointer[%d] %d",i,event.getHistorySize());
                for(int j=0;j<event.getHistorySize();j++){
                    x = event.getHistoricalRawX(i,j);
                    y = event.getHistoricalRawY(i,j);
                    it->second.push_back({x,y});
                    minX = std::min(x,minX);
                    minY = std::min(y,minY);
                    maxX = std::max(x,maxX);
                    maxY = std::max(y,maxY);
                }
            }
            invalidate(minX-R,minY-R,maxX-minX+R+R,maxY-minY+R+R);
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
            std::list<Point>& pts = it->second;
            canvas.set_source_rgb(0,1,float(it->first)/mTouchPoints.size());
            for(auto p:pts){
                canvas.arc(p.x,p.y,R,0,M_PI*2.f);
                canvas.fill();
            }
            pts.clear();
        }
    }
};
void sendEvents();
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Point sz;
    WindowManager::getInstance().getDefaultDisplay().getRealSize(sz);
    LOGI("DisplaySize=%dx%d",sz.x,sz.y);
    Window*w=new TouchWindow(-1,-1);
    w->invalidate();
    Runnable run;
    run=[](){sendEvents();};
    //w->postDelayed(run,5000);
    return app.exec();
}

void sendEvents(){
   TouchDevice d(INJECTDEV_TOUCH);
   struct MTEvent{int type,code,value;};
   MTEvent mts[]={
     {EV_KEY,BTN_TOUCH,1},          //ACTION_DOWN
     {EV_ABS,ABS_MT_TRACKING_ID,45},
     {EV_ABS,ABS_MT_POSITION_X ,10},
     {EV_ABS,ABS_MT_POSITION_Y ,20},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_DOWN finger 0
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},//ACTION_POINTER_DOWN finger 1
     {EV_ABS,ABS_MT_POSITION_X ,40},
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,47},//ACTION_POINTER_DOWN finger 2
     {EV_ABS,ABS_MT_POSITION_X ,200},
     {EV_ABS,ABS_MT_POSITION_Y ,300},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 2
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,40},
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},
#if 10//2 moveevents
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 2
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,40},
     {EV_ABS,ABS_MT_POSITION_Y ,100},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_MOVE finger 1
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_ABS,ABS_MT_TRACKING_ID,46},
     {EV_ABS,ABS_MT_POSITION_X ,60},
     {EV_ABS,ABS_MT_POSITION_Y ,120},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_REPORT,0},
#endif
     {EV_ABS,ABS_MT_TRACKING_ID,45},//ACTION_POINTER_UP finger 1
     {EV_ABS,ABS_MT_POSITION_X ,20},
     {EV_ABS,ABS_MT_POSITION_Y ,30},
     {EV_SYN,SYN_MT_REPORT,0},
     //{EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},

     {EV_ABS,ABS_MT_TRACKING_ID,-1},
     {EV_SYN,SYN_MT_REPORT,0},
     {EV_SYN,SYN_REPORT,0},
   };
   for(int i=0;i<sizeof(mts)/sizeof(MTEvent);i++){
       int32_t tmEVT = i*20*100000;
       d.putRawEvent({0,tmEVT},mts[i].type,mts[i].code,mts[i].value);
   }
   for(int i=0;i<d.getEventCount();i++){
       MotionEvent*e=(MotionEvent*)d.popEvent();
       InputEventSource::getInstance().sendEvent(*e);
   }
}

