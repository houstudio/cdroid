#include <text/textpaint.h>
#include <core/app.h>
#include <widget/cdwindow.h>
#include <text/boringlayout.h>
#include <text/dynamiclayout.h>
class MyWindow:public cdroid::Window {
private:
public:
    MyWindow(int x,int y,int w,int h):Window(x,y,w,h) {
    }
    void onDraw(Canvas&canvas)override {
        cdroid::TextPaint pt;
        pt.setTextSize(24);
        std::u16string u16str=
            u"Hello World! السلام عليكم (Peace be upon you) مرحبا "
            "This is a test with multiple languages including "
            "Arabic: مرحبا بالعالم and Persian: سلام دنیا "
            "شكراً for testing. Thank you! متشکرم "
            "Line breaking should work properly with complex scripts.";

        cdroid::SpannedString*span= new cdroid::SpannedString(u16str);
        //MyString mystr(u8str);
        std::cout<<" spansize="<<span->length()<<std::endl<<std::endl;
        
        
        int staticLayoutHeight=0;
        int dynamicLayoutHeight=0;
        int layoutWidth=0;
        canvas.set_source_rgba(1,1,1,1);
        canvas.paint();
        int32_t avgLayoutTime=0,avgDrawTime=0;
        for(int i=0;i<10;i++){
            cdroid::StaticLayout::Builder *bdr=cdroid::StaticLayout::Builder::obtain(
                span,0,span->length(),&pt,800);
                //&mystr,0,mystr.length(),&pt,800);
                bdr->setMaxLines(8).setLineSpacing(0,1.0f);
            auto startTime = std::chrono::high_resolution_clock::now();
            cdroid::StaticLayout*staticLayout=bdr->build();

            auto endLayout = std::chrono::high_resolution_clock::now();

            canvas.set_color(0xFFFF0000);
            staticLayout->draw(canvas);
            layoutWidth= staticLayout->getWidth();
            staticLayoutHeight=staticLayout->getHeight(false);
            auto endTime = std::chrono::high_resolution_clock::now();
            auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
            auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
            avgLayoutTime+=durlayout.count();
            avgDrawTime+=durDraw.count();
            std::cout << "StaticLayout layout:" << durlayout.count()
              <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
            auto dirs = staticLayout->getLineDirections(1);
            delete staticLayout;
        }
        avgLayoutTime/=1000;
        avgDrawTime/=1000;
        std::cout<<"StaticLayout avgLayoutTime="<<avgLayoutTime<<" avgDrawTime="<<avgDrawTime<<std::endl;
        canvas.set_line_width(3);
        canvas.set_color(0xFF00FF00);
        canvas.rectangle(0,0,layoutWidth,staticLayoutHeight);
        canvas.stroke();
        std::cout<<std::endl;
        canvas.translate(0,staticLayoutHeight);
        canvas.set_color(0x80112233);
        avgLayoutTime=0,avgDrawTime=0;
        for(int i=0;i<10;i++){
            cdroid::DynamicLayout::Builder*bdr=cdroid::DynamicLayout::Builder::obtain(span,&pt,800);
            auto startTime = std::chrono::high_resolution_clock::now();
            auto dynamicLayout = bdr->setLineSpacing(0,1.0f).build();
            auto endLayout = std::chrono::high_resolution_clock::now();
            
            dynamicLayout->draw(canvas);
            auto endTime = std::chrono::high_resolution_clock::now();
            auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
            auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
            avgLayoutTime+=durlayout.count();
            avgDrawTime+=durDraw.count();
            std::cout << "DynamicLayout layout:" << durlayout.count()
              <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
            layoutWidth=dynamicLayout->getWidth();
            dynamicLayoutHeight=dynamicLayout->getHeight(false);
            delete dynamicLayout;
        }
        avgLayoutTime/=1000;
        avgDrawTime/=1000;
        std::cout<<"DynamicLayout avgLayoutTime="<<avgLayoutTime<<" avgDrawTime="<<avgDrawTime<<std::endl;
        canvas.rectangle(0,0,layoutWidth,dynamicLayoutHeight);
        canvas.stroke();
        std::cout<<std::endl;
        BoringLayout::Metrics metrics;//={0,20,-7,28,0,0};
        metrics.ascent=-7;metrics.descent=20;
        metrics.bottom=32;metrics.top=0;
        int boringWidth,boringHeight;
        auto hintBoring = BoringLayout::isBoring(span, &pt,TextDirectionHeuristics::LTR,nullptr);//mHintBoring);
        canvas.translate(0,dynamicLayoutHeight+20);
        pt.setTextSize(24);
        for(int i=0;i<10;i++){
            auto startTime = std::chrono::high_resolution_clock::now();
            auto boringLayout=BoringLayout::make(span,&pt,getWidth(),Layout::Alignment::ALIGN_NORMAL, 1.f/*mSpacingMult*/, 0/*mSpacingAdd*/,metrics, false);
            auto endLayout = std::chrono::high_resolution_clock::now();
            boringLayout->draw(canvas,nullptr,nullptr,0);
            auto endTime = std::chrono::high_resolution_clock::now();
             auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
            auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
            boringWidth=boringLayout->getWidth();
            boringHeight=boringLayout->getHeight();
            avgLayoutTime+=durlayout.count();
            avgDrawTime+=durDraw.count();
            std::cout << "BoringLayout layout:" << durlayout.count()
              <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
            delete boringLayout;
        }
        avgLayoutTime/=100;
        avgDrawTime/=100;
        std::cout<<"avgLayoutTime="<<avgLayoutTime<<std::endl;
        std::cout<<"avgDrawTime="<<avgDrawTime<<std::endl;

        canvas.set_color(0xFF0000FF);
        canvas.rectangle(0,-7,boringWidth,boringHeight);
        canvas.stroke();
        canvas.translate(0,boringHeight+20);
        std::string u8str=TextUtils::utf16_utf8(u16str);
        /*for(int j=0;j<10;j++){
            cdroid::Layout layout(24,getWidth());
            //layout.setMultiline(true);
            layout.setText(u8str);
            auto startTime = std::chrono::high_resolution_clock::now();
            layout.relayout(true);
            auto endLayout = std::chrono::high_resolution_clock::now();

            canvas.set_color(0xFF000000);
            layout.draw(canvas);
            auto endTime = std::chrono::high_resolution_clock::now();
            auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
            auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
            std::cout << "CdroidLayout layout:" << durlayout.count()
                <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
        }*/
        delete span;
        cdroid::Path path;
        float startX = 100, startY = 200;
        float w = 300, h = 100;
        
        path.move_to(startX, startY);
        path.curve_to(startX + w/3, startY - h, startX + w*2/3, startY + h, startX + w, startY);
        path.curve_to(startX + w + w/3, startY - h, startX + w + w*2/3, startY + h, startX + w*2, startY);
        path.curve_to(startX + w*2 + w/3, startY - h, startX + w*2 + w*2/3, startY + h, startX + w*3, startY);
        path.append_to_context(&canvas);
        canvas.stroke();
        canvas.set_color(0xFFFF0000);
        pt.setTextSize(48);
        pt.drawTextOnPath(canvas,"Hello world!,Perfect CDROID.",path,30,10);
        canvas.dump2png("text.png");
    }
};
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
    cdroid::TextPaint pt;
    pt.setTextSize(32);
    std::vector<float> advances(32);
    //float advance1=pt.measureText((const char32_t*)L"Hello World!Haha!.",0,18);
    //std::cout<<"advance="<<advance1<<std::endl;
    MyWindow mw(0,0,-1,-1);
    app.exec();
}
