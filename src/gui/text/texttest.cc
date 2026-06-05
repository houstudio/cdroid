#include <text/textpaint.h>
#include <core/app.h>
#include <text/staticlayout.h>
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
    cdroid::TextPaint pt;
    pt.setTextSize(32);
    cdroid::Canvas  canvas(1280,480);
    std::vector<float> advances(32);
    canvas.set_source_rgb(1,1,1);
    canvas.fill();
    float advance1=pt.measureText((const char32_t*)L"Hello World!Haha!.",0,18);
    std::cout<<"advance="<<advance1<<std::endl;

    cdroid::SpannedString*span= new cdroid::SpannedString(
        "Hello World! السلام عليكم (Peace be upon you) مرحبا "
        "This is a test with multiple languages including "
        "Arabic: مرحبا بالعالم and Persian: سلام دنیا "
        "شكراً for testing. Thank you! متشکرم "
        "Line breaking should work properly with complex scripts.");

    auto startTime = std::chrono::high_resolution_clock::now();    
    cdroid::StaticLayout::Builder *bdr=cdroid::StaticLayout::Builder::obtain(
            span,0,200,&pt,800);
            bdr->setMaxLines(8);
    cdroid::StaticLayout*staticlayout=bdr->build();
    canvas.set_source_rgba(1,1,1,1);
    canvas.paint();
    canvas.set_line_width(3);
    canvas.set_color(0xFF00FF00);
    canvas.rectangle(0,0,800,480);
    canvas.stroke();
    canvas.set_color(0xFFFF0000);
    auto endLayout = std::chrono::high_resolution_clock::now();
    staticlayout->draw(canvas);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
    auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
    std::cout << "StaticLayout layout:" << durlayout.count() 
        <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
    canvas.dump2png("text.png");
    std::cout<<"staticlayout="<<staticlayout<<std::endl;
    app.exec();
}
