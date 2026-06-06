#include <text/textpaint.h>
#include <core/app.h>
#include <core/layout.h>
#include <widget/cdwindow.h>
#include <text/staticlayout.h>

class MyWindow:public cdroid::Window {
private:
public:
    MyWindow(int x,int y,int w,int h):Window(x,y,w,h) {
    }
    void onDraw(Canvas&canvas)override {
        cdroid::TextPaint pt;
        pt.setTextSize(32);
        std::string u8str=
            "Hello World! السلام عليكم (Peace be upon you) مرحبا "
            "This is a test with multiple languages including "
            "Arabic: مرحبا بالعالم and Persian: سلام دنیا "
            "شكراً for testing. Thank you! متشکرم "
            "Line breaking should work properly with complex scripts.";

        cdroid::SpannedString*span= new cdroid::SpannedString(u8str);
        std::cout<<" spansize="<<span->length()<<std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        cdroid::StaticLayout::Builder *bdr=cdroid::StaticLayout::Builder::obtain(
                span,0,span->length(),&pt,800);
                bdr->setMaxLines(8).setLineSpacing(0,1.5);
        cdroid::StaticLayout*staticlayout=bdr->build();

        auto endLayout = std::chrono::high_resolution_clock::now();

        canvas.set_source_rgba(1,1,1,1);
        //canvas.rectangle(0,0,1280,480);
        canvas.paint();
        canvas.set_line_width(3);
        canvas.set_color(0xFF00FF00);
        canvas.rectangle(0,0,staticlayout->getWidth(),staticlayout->getHeight(false));
        canvas.stroke();
        canvas.set_color(0xFFFF0000);
        staticlayout->draw(canvas);
        auto endTime = std::chrono::high_resolution_clock::now();

        auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
        auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);

        std::cout << "StaticLayout layout:" << durlayout.count()
            <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
        
        cdroid::Layout layout(32,800);
        layout.setMultiline(true);
        layout.setText(u8str);
        startTime = std::chrono::high_resolution_clock::now();
        layout.relayout(true);
        endLayout = std::chrono::high_resolution_clock::now();

        canvas.translate(0,staticlayout->getHeight(false)+10);
        canvas.set_color(0xFF000000);
        layout.draw(canvas);
        endTime = std::chrono::high_resolution_clock::now();
        durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
        durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
        std::cout << "CdroidLayout layout:" << durlayout.count()
            <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
        canvas.dump2png("text.png");
    }
};
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
    cdroid::TextPaint pt;
    pt.setTextSize(32);
    std::vector<float> advances(32);
    float advance1=pt.measureText((const char32_t*)L"Hello World!Haha!.",0,18);
    std::cout<<"advance="<<advance1<<std::endl;
    MyWindow mw(0,0,-1,-1);
    app.exec();
}
