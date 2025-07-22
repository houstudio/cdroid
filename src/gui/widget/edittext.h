#ifndef __CDROID_EDITVIEW_H__
#define __CDROID_EDITVIEW_H__
#include <widget/textview.h>

namespace cdroid{

class EditText:public TextView{
private:
    void initEditText();
    void checkMatch(const std::wstring&w);
public:
    DECLARE_UIEVENT(void,AfterTextChanged,EditText&);
protected:
    Runnable mRBLink;
    int mPasswordChar;
    int mInputType;
    std::wstring mInputPattern;
    AfterTextChanged afterChanged;
    bool match();
    void blinkCaret();
    void onDetachedFromWindow()override;
    virtual void onDrawCaret(Canvas&canvas,const Rect&r);
    virtual void onFocusChanged(bool,int,Rect*)override;
    virtual void onDraw(Canvas&ctx)override;
    int commitText(const std::wstring&ws)override;
public:
    typedef enum{
        TYPE_NONE,
        TYPE_ANY,
        TYPE_NUMBER,
        TYPE_TEXT,
        TYPE_PASSWORD,
        TYPE_IP
    }INPUTTYPE;
    EditText(int w,int h);
    EditText(const std::string&txt,int w,int h);
    EditText(Context*ctx,const AttributeSet&attrs);
    void setText(const std::string&txt)override;
    void setLabelColor(int color);
    void replace(size_t start,size_t len,const std::string&txt);
    void setPattern(const std::string&pattern);
    void setHint(const std::string&txt)override;
    int getPasswordChar()const;
    void setPasswordChar(int ch);
    void setInputType(INPUTTYPE tp);
    int getInputType();
    virtual void setTextWatcher(AfterTextChanged ls);
    virtual void setEditMode(EDITMODE mode);
    void setCaretBlink(bool blink=true);
    virtual bool onKeyDown(int,KeyEvent&evt)override;
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};

}//endof cdroid

#endif

