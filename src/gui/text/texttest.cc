#include <text/textpaint.h>
#include <core/app.h>
#include <core/layout.h>
#include <widget/cdwindow.h>
#include <text/dynamiclayout.h>
class MyString:public cdroid::CharSequence{
private:
    std::wstring mStr;
public:
    MyString(const std::string& str){
        mStr=TextUtils::utf8tounicode(str);
    }
    MyString(const std::wstring& str):mStr(str){   
    }
    virtual ~MyString() = default;
    virtual size_t length()const{return mStr.size();}
    virtual int charAt(int idx)const{return mStr[idx];}
    virtual CharSequence*subSequence(int,int)const{return nullptr;}
    virtual std::string toString() const {
        return TextUtils::unicode2utf8(mStr);
    }
    // Copies characters from [start, end) into dest starting at destPos.
    // If dest is shorter than destPos, it will be resized.
    virtual void getChars(int start, int end, std::vector<char16_t>& dest, int destPos) const {
        dest.assign(mStr.begin()+start,mStr.begin()+end);
    }
};
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
            std::cout << "StaticLayout layout:" << durlayout.count()
               <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
            auto dirs = staticLayout->getLineDirections(1);
            delete staticLayout;
        }
        canvas.set_line_width(3);
        canvas.set_color(0xFF00FF00);
        canvas.rectangle(0,0,layoutWidth,staticLayoutHeight);
        canvas.stroke();
        std::cout<<std::endl;
        canvas.translate(0,staticLayoutHeight);
        canvas.set_color(0x80112233);
        for(int i=0;i<10;i++){
            cdroid::DynamicLayout::Builder*bdr=cdroid::DynamicLayout::Builder::obtain(span,&pt,800);
            auto startTime = std::chrono::high_resolution_clock::now();
            auto dynamicLayout = bdr->setLineSpacing(0,1.0f).build();
            auto endLayout = std::chrono::high_resolution_clock::now();
            
            dynamicLayout->draw(canvas);
            auto endTime = std::chrono::high_resolution_clock::now();
            auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
            auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
            std::cout << "DynamicLayout layout:" << durlayout.count()
               <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
            layoutWidth=dynamicLayout->getWidth();
            dynamicLayoutHeight=dynamicLayout->getHeight(false);
            delete dynamicLayout;
        }
        canvas.rectangle(0,0,layoutWidth,dynamicLayoutHeight);
        canvas.stroke();
        std::cout<<std::endl;
        canvas.translate(0,dynamicLayoutHeight+20);
        std::string u8str=TextUtils::utf16_utf8(u16str);
        for(int j=0;j<10;j++){
            cdroid::Layout layout(24,800);
            layout.setMultiline(true);
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
        }
        delete span;
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
