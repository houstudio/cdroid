#include <widget/edittext.h>
#include <inputmethodmanager.h>
#include <cdlog.h>
#include <regex>
#include <math.h>
#include <textutils.h>
#include <layout.h>


namespace cdroid{

EditText::EditText(Context*ctx,const AttributeSet& attrs):TextView(ctx,attrs){
    mHint=ctx->getString(attrs.getString("hint"));
    setEditable(true);
}

EditText::EditText(int w,int h):EditText(std::string(),w,h){
}

EditText::EditText(const std::string&txt,int w,int h):TextView(txt,w,h){
    setFocusable(true);
    setFocusableInTouchMode(true);
    setEditable(true);
    mPasswordChar='*';
    mBlinkOn=true;
    mEditMode=INSERT;
    mInputType=TYPE_ANY;
    afterChanged=nullptr;
    mCaretRect.set(0,0,1,1);
    mBlinkOn=false;
    mRBLink=std::bind(&EditText::blinkCaret,this);
}

EditText::~EditText(){
    mBlinkOn=false;
    removeCallbacks(mRBLink);
}

void EditText::setTextWatcher(AfterTextChanged ls){
    afterChanged=ls;
}

void EditText::setText(const std::string&txt){
    TextView::setText(txt);
    setCaretPos(0);
}

int EditText::commitText(const std::wstring&ws){
    std::wstring& wText=getEditable();
    switch(mEditMode){
    case READONLY:return 0;
    case INSERT:
        if(mCaretPos<wText.size())
            wText.insert(mCaretPos,ws);
        else 
            wText.append(ws);
        break;
    case REPLACE:
        if(mCaretPos<wText.size())
            wText.replace(mCaretPos,ws.length(),ws);
        else
            wText.append(ws);
        break;
    }
    mLayout->relayout(true);
    setCaretPos(mCaretPos+ws.length());
    invalidate(nullptr);
    return ws.length();
}

void EditText::setInputType(INPUTTYPE tp){
    if(tp!=mInputType){
       if(tp==TYPE_PASSWORD||mInputType==TYPE_PASSWORD)
           invalidate(nullptr);
       switch(tp){
       case TYPE_IP:
           setPattern("^((25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))$");
           break;
       case TYPE_PASSWORD:
           setSingleLine(true);break;
       case TYPE_NUMBER:setPattern("^[1-9]\\d*$");break;
       default:setPattern("");break;
       }mInputType=tp;
    }
}

int EditText::getInputType(){
    return mInputType;
}

void EditText::setPattern(const std::string&pattern){
    mInputPattern=TextUtils::utf8tounicode(pattern);
}

bool EditText::match(){
    if(mInputPattern.empty())
       return true;
    try{
        std::wstring& wText=getEditable();
        std::wregex reg(mInputPattern);
        return std::regex_match(wText,reg);
    }catch(std::exception &ex){
        LOGE("Error:%s",ex.what());
        return false;
    }
}

void EditText::checkMatch(const std::wstring&ws){
    if(!match()){
        getEditable()=ws;
    }
    mLayout->relayout();
    invalidate(nullptr);
}

void EditText::replace(size_t start,size_t len,const std::string&txt){
    std::wstring wtxt=TextUtils::utf8tounicode(txt);
    getEditable().replace(start,len,wtxt);
    mCaretPos=start+wtxt.size()-len;
    invalidate(nullptr);
}

void EditText::setEditMode(EDITMODE mode){
    mEditMode=mode;
}

View& EditText::setHint(const std::string&txt){
    View::setHint(txt);
    if(getText().empty())invalidate(nullptr);
    return *this;
}

void EditText::onFocusChanged(bool focus,int direction,const RECT*prevfocusrect){
    InputMethodManager&imm=InputMethodManager::getInstance();
    if(focus){
	imm.setInputType(mInputType);
        InputMethodManager::getInstance().focusIn((View*)this);
    }else{
        InputMethodManager::getInstance().focusOut((View*)this);
    }

    TextView::onFocusChanged(focus,direction,prevfocusrect);
    mBlinkOn=focus;
    if(mBlinkOn){
        if(mCaretRect.empty())
            mCaretRect=getClientRect();
        blinkCaret();
    }
    if(mInputType!=TYPE_NONE)imm.showIme();
}

bool EditText::onKeyDown(int keyCode,KeyEvent & event){
    wchar_t ch;
    bool ret=false;
    int changed=0;

    std::wstring& wText=getEditable();
    int line=mLayout->getLineForOffset(mCaretPos);
    switch(keyCode){
    case KEY_LEFT:
        if(mCaretPos>0){
            setCaretPos(mCaretPos-1);
            return true;
        }break;
    case KEY_RIGHT:
        if(mCaretPos<(int)wText.size()){
            setCaretPos(mCaretPos+1);
            return true;
        }break;
    case KEY_DOWN:
        return (!isSingleLine())&&moveCaret2Line(line+1);
    case KEY_UP:
        return (!isSingleLine())&&moveCaret2Line(line-1);
    case KEY_BACKSPACE:
        if( (mCaretPos>0) && (mCaretPos<wText.size()) ){
            wchar_t wc0=wText[mCaretPos];
            wText.erase(mCaretPos-1,1);
            changed=match();
            if(changed){
                setCaretPos(mCaretPos-1);
                mLayout->relayout(true);
            }else
                wText.insert(mCaretPos-1,1,wc0);
            ret=true;
        }else setCaretPos(wText.size()-1);
        break;
    case KEY_DELETE:
        if(mCaretPos<wText.size()){
            wchar_t wc0=wText[mCaretPos];
            wText.erase(mCaretPos,1);
            changed=match();
            if(!changed) wText.insert(mCaretPos,1,wc0);
            else mLayout->relayout(true);
            ret=true; 
        }break;
    case KEY_INSERT:
         mEditMode=!mEditMode;
         invalidate(nullptr);
         break;
    case KEY_OK:
         if(nullptr!=afterChanged)
             afterChanged(*this);
         return false;
    case KEY_MENU:
        return true;
    case KEY_ENTER:
        if(!isSingleLine()){
            if(mCaretPos<wText.length()) wText.insert(mCaretPos,1,'\n');
            else wText.append(1,'\n');
            mLayout->relayout(true);
            invalidate(nullptr);
            return true;
        }
        return false;
    default:
        ch=InputMethodManager::getInstance().getCharacter(keyCode,event.getMetaState());
        if(ch!=0){
            std::wstring ws;
            ws.append(1,ch); 
            commitText(ws);
            return true; 
        }
        return TextView::onKeyDown(keyCode,event);
    }
    if(changed){
        mLayout->relayout();  
        invalidate(nullptr);
        if(nullptr!=afterChanged)
            afterChanged(*this);
    }
    return ret;
}

void EditText::blinkCaret(){
    if(isFocused()){
        invalidate(&mCaretRect);
        postDelayed(mRBLink,500);
    }
}

void EditText::onDrawCaret(Canvas&canvas,const RECT&r){
    canvas.rectangle(r);
    canvas.fill();
}

int EditText::getPasswordChar()const{
    return mPasswordChar;
}

void EditText::setPasswordChar(int ch){
    mPasswordChar = ch;
    if(mInputType==TYPE_PASSWORD)
        invalidate(nullptr);
}

void EditText::onDraw(Canvas&canvas){
    const std::wstring& wText=getEditable();
    mLayout->relayout();
    canvas.set_font_size(getFontSize());
    if(wText.empty()||(mInputType==TYPE_PASSWORD) ){
        Layout hpl(*mLayout);
        Layout* tmp=mLayout;
        if((mInputType==TYPE_PASSWORD))
            hpl.setText(std::string(wText.length(),'*'));
        else
            hpl.setText(mHint);
        hpl.relayout();
        mLayout = &hpl;
        TextView::onDraw(canvas);
        mLayout = tmp;
    }else{
        TextView::onDraw(canvas);
    }
    
    if(isFocused()){
        canvas.set_color(getCurrentTextColor()&0x80FFFFFF);
        if(!mCaretRect.empty()&&mBlinkOn)
            onDrawCaret(canvas,mCaretRect);
        mBlinkOn=!mBlinkOn;
    }
}

}//end namespace


