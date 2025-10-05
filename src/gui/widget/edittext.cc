#include <widget/edittext.h>
#include <core/inputmethodmanager.h>
#include <cdlog.h>
#include <regex>
#include <math.h>
#include <utils/textutils.h>
#include <core/layout.h>

namespace cdroid{

DECLARE_WIDGET2(EditText,"cdroid:attr/editTextStyle")

EditText::EditText(Context*ctx,const AttributeSet& attrs)
  :TextView(ctx,attrs){
    initEditText();
    mHint = ctx->getString(attrs.getString("hint"));
    mInputType = attrs.getInt("inputType",std::unordered_map<std::string,int>{
		    {"none",TYPE_NONE}    , {"any", TYPE_ANY},
		    {"number",TYPE_NUMBER}, {"text",TYPE_TEXT},
		    {"textPassword",TYPE_PASSWORD},{"ip",TYPE_IP},
		    {"textVisiblePassword",TYPE_PASSWORD}
	  },TYPE_NONE);
    setEditable(true);
}

EditText::EditText(int w,int h):EditText(std::string(),w,h){
}

EditText::EditText(const std::string&txt,int w,int h):TextView(txt,w,h){
    initEditText();
    setFocusable(true);
    setFocusableInTouchMode(true);
    setEditable(true);
}

void EditText::initEditText(){
    mPasswordChar='*';
    mBlinkOn=true;
    mEditMode=INSERT;
    mInputType=TYPE_NONE;
    afterChanged=nullptr;
    mCaretRect.set(0,0,1,1);
    mBlinkOn=false;
    mRBLink=std::bind(&EditText::blinkCaret,this);
}

void EditText::onDetachedFromWindow(){
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
        if((mMaxLength>0)&&(wText.size()>=mMaxLength))
            break;
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
    invalidate(true);
    return ws.length();
}

void EditText::setInputType(INPUTTYPE tp){
    if(tp!=mInputType){
       if(tp==TYPE_PASSWORD||mInputType==TYPE_PASSWORD)
           invalidate(true);
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
        std::wstring& wText = getEditable();
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
    invalidate(true);
}

void EditText::replace(size_t start,size_t len,const std::string&txt){
    std::wstring wtxt=TextUtils::utf8tounicode(txt);
    getEditable().replace(start,len,wtxt);
    mCaretPos=start+wtxt.size()-len;
    invalidate(true);
}

void EditText::setEditMode(EDITMODE mode){
    mEditMode=mode;
}

void EditText::setHint(const std::string&txt){
    View::setHint(txt);
    if(getText().empty())invalidate(true);
}

void EditText::onFocusChanged(bool focus,int direction,Rect*prevfocusrect){
    InputMethodManager & imm = InputMethodManager::getInstance();
    if(focus){
	imm.setInputType(mInputType);
        imm.focusIn((View*)this);
    }else{
        imm.focusOut((View*)this);
    }
    TextView::onFocusChanged(focus,direction,prevfocusrect);
    mBlinkOn = focus;
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
    case KeyEvent::KEYCODE_DPAD_LEFT:
        if(mCaretPos>0){
            setCaretPos(mCaretPos-1);
            return true;
        }break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        if(mCaretPos<(int)wText.size()){
            setCaretPos(mCaretPos+1);
            return true;
        }break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        return (!isSingleLine())&&moveCaret2Line(line+1);
    case KeyEvent::KEYCODE_DPAD_UP:
        return (!isSingleLine())&&moveCaret2Line(line-1);
    case KeyEvent::KEYCODE_BACKSPACE:
        if(wText.size() && (mCaretPos>0) && (mCaretPos<=wText.size()) ){
            wchar_t wc0 = wText[mCaretPos-1];
            wText.erase(mCaretPos-1,1);
            changed = match();
            if(changed){
                setCaretPos(mCaretPos-1);
                mLayout->relayout(true);
            }else
                wText.insert(mCaretPos-1,1,wc0);
            ret=true;
        }else setCaretPos(wText.size()-1);
        break;
    case KeyEvent::KEYCODE_DEL:
        if(mCaretPos<wText.size()){
            wchar_t wc0=wText[mCaretPos];
            wText.erase(mCaretPos,1);
            changed=match();
            if(!changed) wText.insert(mCaretPos,1,wc0);
            else mLayout->relayout(true);
            ret=true; 
        }break;
    case KeyEvent::KEYCODE_INSERT:
         mEditMode=!mEditMode;
         invalidate(true);
         break;
#if 0
    case KeyEvent::KEYCODE_OK:
         if(nullptr!=afterChanged)
             afterChanged(*this);
         return false;
#endif
    case KeyEvent::KEYCODE_MENU:
        return true;
    case KeyEvent::KEYCODE_ENTER:
        if(!isSingleLine()){
            if(mCaretPos<wText.length()) wText.insert(mCaretPos,1,'\n');
            else wText.append(1,'\n');
            mLayout->relayout(true);
            invalidate(true);
            return true;
        }
        return false;
    default:
        ch = InputMethodManager::getInstance().getCharacter(keyCode,event.getMetaState());
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
        invalidate(true);
        if(nullptr!=afterChanged)
            afterChanged(*this);
    }
    return ret;
}

void EditText::blinkCaret(){
    if(isFocused()){
        invalidate((const Rect*)&mCaretRect);
        postDelayed(mRBLink,500);
    }
}

void EditText::onDrawCaret(Canvas&canvas,const Rect&r){
    canvas.rectangle(r);
    canvas.fill();
}

int EditText::getPasswordChar()const{
    return mPasswordChar;
}

void EditText::setPasswordChar(int ch){
    mPasswordChar = ch;
    if(mInputType==TYPE_PASSWORD)
        invalidate(true);
}

void EditText::onDraw(Canvas&canvas){
    const std::wstring& wText=getEditable();
    mLayout->relayout();
    canvas.set_font_size(getFontSize());
    if(wText.empty()||(mInputType==TYPE_PASSWORD) ){
        Layout hpl(*mLayout);
        Layout* tmp=mLayout;
        if((mInputType==TYPE_PASSWORD)&&wText.length())
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

std::string EditText::getAccessibilityClassName()const{
    return "EditText";
}

void EditText::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    TextView::onInitializeAccessibilityNodeInfoInternal(info);
    if (isEnabled()) {
        info.addAction(AccessibilityNodeInfo::ACTION_SET_TEXT);//AccessibilityAction::ACTION_SET_TEXT);
    }
}
}//end namespace


