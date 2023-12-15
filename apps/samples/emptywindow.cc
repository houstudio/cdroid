#include <cdroid.h>
#include <cdlog.h>
#include <widget/candidateview.h>
#include <cairomm/cairomm.h>

class MyWindow:public Window{
public:
    MyWindow(int x,int y,int w,int h):Window(x,y,w,h){};
    void onDraw(Canvas&canvas)override{
        LOGD("%p onDraw",this);
        Rect rc={0,0,getWidth(),getHeight()};
        canvas.set_source_rgb(1,0,0);
	canvas.rotate(30);
	//canvas.arc(100,100,50,0,M_PI*2);
	canvas.rectangle(0,0,100,100);
	canvas.clip();
	double x,y,w,h;
	canvas.get_clip_extents(x,y,w,h);
	LOGD("clip=%f,%f,%f,%f",x,y,w,h);
        for(int i=0;i<10;i++){
            canvas.rectangle(rc.left,rc.top,rc.width,rc.height);
            canvas.stroke();
            rc.inflate(-10,-10);
        }
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new MyWindow(0,0,-1,-1);

    // 加载彩色图像
    Cairo::RefPtr<Cairo::ImageSurface> colorImage = Cairo::ImageSurface::create_from_png("bg_1.png");
    // 创建一个灰度图像表面
    Cairo::RefPtr<Cairo::ImageSurface> grayImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, colorImage->get_width(), colorImage->get_height());
    // 创建一个绘图上下文
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(grayImage);

    // 创建一个表面填充对象
    Cairo::RefPtr<Cairo::SurfacePattern> pattern = Cairo::SurfacePattern::create(colorImage);
    // 创建一个变换矩阵来实现灰度滤镜效果
    const long start=SystemClock::uptimeMillis();
    Cairo::Matrix matrix=Cairo::rotation_matrix(M_PI/10.f);//{0.3, 0.59, 0.11, 0, 0, 0}; // 灰度转换矩阵
    // 应用变换矩阵
    pattern->set_matrix(matrix);
    // 使用填充对象填充一个矩形，以实现灰度滤镜效果
    cr->set_source(pattern);
    cr->paint();
    LOGD("used time=%ld",SystemClock::uptimeMillis()-start);
    grayImage->write_to_png("gray.png");

    return app.exec();
}
