#ifndef __CLICKABLE_SPAN_H__
#define __CLICKABLE_SPAN_H__
namespace cdroid{
class View;
class TextPaint;
class ClickableSpan:public CharacterStyle{
private:
    int mID;
public:
    ClickableSpan(){
        static int sIdCounter;
        mID = sIdCounter++;
    }
    virtual void onClick(View& widget)=0;
    void updateDrawState(TextPaint& paint)const override{
        paint.setUnderlineText(true);
        paint.setColor(paint.linkColor);
    }
    int getId()const{
        return mID;
    }
};

class URLSpan:public ClickableSpan{
private:
    std::string mURL;
public:
    URLSpan(const std::string&url):mURL(url){
    }
    std::string getURL()const{return mURL;}
};
}/*endof namespace*/
#endif/*__CLICKABLE_SPAN_H__*/
