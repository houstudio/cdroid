#include <cdroid.h>
#include <cdlog.h>
#include <core/pathmeasure.h>
#include <view/gesturedetector.h>
#include <view/scalegesturedetector.h>
class MyWindow:public Window {
private:
    GestureDetector*mDetector;
    ScaleGestureDetector*mScaler;
    std::shared_ptr<cdroid::Path>mPath;
    PathMeasure mPathMeasure;
    std::shared_ptr<cdroid::Path>mTrimedPath;
    Runnable mRunnable;
public:
    MyWindow(int x,int y,int w,int h):Window(x,y,w,h) {
        GestureDetector::OnGestureListener gl;
        gl.onDown=std::bind(&MyWindow::onDown,this,std::placeholders::_1);
        gl.onSingleTapUp=std::bind(&MyWindow::onSingleTapUp,this,std::placeholders::_1);
        gl.onShowPress=std::bind(&MyWindow::onShowPress,this,std::placeholders::_1);
        gl.onLongPress=std::bind(&MyWindow::onLongPress,this,std::placeholders::_1);
        mDetector=new GestureDetector(mContext,gl);
        GestureDetector::OnDoubleTapListener dl;
        ScaleGestureDetector::OnScaleGestureListener sl;
        sl.onScale=[](ScaleGestureDetector&scaler) {
            LOGD("scaled %f",scaler.getScaleFactor());
            return true;
        };
        mScaler=new ScaleGestureDetector(mContext,sl);
        dl.onSingleTapConfirmed=std::bind(&MyWindow::onSingleTapConfirmed,this,std::placeholders::_1);
        dl.onDoubleTap=std::bind(&MyWindow::onDoubleTap,this,std::placeholders::_1);
        dl.onDoubleTapEvent=std::bind(&MyWindow::onDoubleTapEvent,this,std::placeholders::_1);
        setOnTouchListener([this](View& v, MotionEvent& event) {
            mScaler->onTouchEvent(event);
            return mDetector->onTouchEvent(event);
        });
	mTrimedPath=std::make_shared<cdroid::Path>();
        mDetector->setOnDoubleTapListener(dl);
    }
    void onDraw(Canvas&canvas)override {
        if(mPath==nullptr) {
            canvas.set_font_size(64);
            canvas.set_source_rgb(1,0,0);
            canvas.move_to(80,100);
            canvas.text_path("@#$CDROID make UI easy!");
            canvas.fill_preserve();
            LOGD("textpath");
            mPath =std::make_shared<cdroid::Path>(&canvas);
	    mPathMeasure.setPath(mPath,false);
            return;
        }
        Rect rc= {0,0,getWidth(),getHeight()};
        canvas.set_source_rgb(1,0,0);

        canvas.save();
        mTrimedPath->append_to_context(&canvas);
	canvas.fill_preserve();
	canvas.set_source_rgb(0,1,0);
        canvas.stroke();
        canvas.set_source_rgba(1,1,1,0.2);
        canvas.rectangle(300,100,200,200);
        canvas.stroke();
        for(int i=0; i<10; i++) {
            canvas.rectangle(rc.left,rc.top,rc.width,rc.height);
            canvas.stroke();
            rc.inflate(-10,-10);
        }
        canvas.translate(50,200);
        canvas.set_source_rgba(0,1,0,0.5);
        canvas.move_to(510.00,200.00);
        mPath->append_to_context(&canvas);
        canvas.stroke();
        canvas.restore();
    }
    void onLongPress(MotionEvent&e) {
        LOGD("onLongPress");
    }
    bool onDown(MotionEvent& e) {
        if(mRunnable==nullptr) {
            mRunnable =[this]() {
                invalidate();
		mPathMeasure.nextContour();
                mPathMeasure.getSegment(0,INT_MAX,mTrimedPath,true);
                postDelayed(mRunnable,150);
            };
            postDelayed(mRunnable,100);
        }
        invalidate();
        return false;
    }

    bool onSingleTapUp(MotionEvent& e) {
        LOGD("");
        return false;
    }
    void onShowPress(MotionEvent&) {
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
int main(int argc,const char*argv[]) {
    App app(argc,argv);
    Window*w=new MyWindow(0,0,-1,-1);
    w->setBackgroundColor(0xFF112233);
    /*AnimatedImageDrawable*ad=new AnimatedImageDrawable(&app,"./test.webp");
    w->setBackground(ad);
    ad->start();*/
    return app.exec();
}
