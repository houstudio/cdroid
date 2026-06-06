#include <text/textpaint.h>
#include <core/app.h>
#include <core/layout.h>
#include <widget/cdwindow.h>
#include <text/staticlayout.h>
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
    virtual std::wstring toWString() const {return mStr;}
    // Copies characters from [start, end) into dest starting at destPos.
    // If dest is shorter than destPos, it will be resized.
    virtual void getChars(int start, int end, std::vector<char32_t>& dest, int destPos) const {
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
        pt.setTextSize(32);
        std::string u8str=
            "Hello World! السلام عليكم (Peace be upon you) مرحبا "
            "This is a test with multiple languages including "
            "Arabic: مرحبا بالعالم and Persian: سلام دنیا "
            "شكراً for testing. Thank you! متشکرم "
            "Line breaking should work properly with complex scripts.";

        cdroid::SpannedString*span= new cdroid::SpannedString(u8str);
        MyString mystr(u8str);
        std::cout<<" spansize="<<span->length()<<std::endl;
        
        
        int staticLayoutHeight=0;

        for(int i=0;i<2;i++){
            cdroid::StaticLayout::Builder *bdr=cdroid::StaticLayout::Builder::obtain(
                //span,0,span->length(),&pt,800);
                &mystr,0,mystr.length(),&pt,800);
                bdr->setMaxLines(8).setLineSpacing(0,1.5);
            auto startTime = std::chrono::high_resolution_clock::now();
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
            staticLayoutHeight=staticlayout->getHeight(false);
            auto endTime = std::chrono::high_resolution_clock::now();
            auto durlayout = std::chrono::duration_cast<std::chrono::microseconds>(endLayout-startTime);
            auto durDraw=std::chrono::duration_cast<std::chrono::microseconds>(endTime-endLayout);
            std::cout << "StaticLayout layout:" << durlayout.count()
               <<" draw:"<< durDraw.count() <<" microseconds" << std::endl;
            delete staticlayout;
        }

        for(int j=0;j<2;j++){
            cdroid::Layout layout(32,800);
            layout.setMultiline(true);
            layout.setText(u8str);
            auto startTime = std::chrono::high_resolution_clock::now();
            layout.relayout(true);
            auto endLayout = std::chrono::high_resolution_clock::now();

            canvas.translate(0,staticLayoutHeight+10);
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
    float advance1=pt.measureText((const char32_t*)L"Hello World!Haha!.",0,18);
    std::cout<<"advance="<<advance1<<std::endl;
    MyWindow mw(0,0,-1,-1);
    app.exec();
}
