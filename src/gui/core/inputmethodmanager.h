#ifndef __INPUT_METHOD_MANAGER_H__
#define __INPUT_METHOD_MANAGER_H__
#include <windows.h>
#include <widget/keyboardview.h>
#include <inputmethod.h>
#include <map>
namespace cdroid{

class InputMethodManager{
private:
    friend class IMEWindow;
    int mInputType;
    std::wstring text2IM;
    std::shared_ptr<Keyboard>kbd;
    static std::map<std::string,InputMethod*>imemethods;
    int setInputMethod(InputMethod*,const std::string&name);
protected:
    class KeyCharacterMap*kcm;
    std::string predictsource;
    InputMethod*im;
    static class InputMethodManager*mInst;
    class IMEWindow*imeWindow;
    InputMethodManager();
    ~InputMethodManager();
    void commitText(const std::wstring&text,int newcursorPos);
public:
    static InputMethodManager&getInstance();
    static int registeMethod(const std::string&name,InputMethod*);
    static std::vector<std::string>getInputMethods(std::vector<InputMethod*>*methods);
    void shutDown(){delete mInst;}
    int setKeyCharacterMap(const std::string&kcm);
    int getCharacter(int keycode,int metaState)const;
    void focusIn(View*);
    void focusOut(View*);
    void sendKeyEvent(KeyEvent&k);
    void onViewDetachedFromWindow(View*);
    void setInputType(int inputType);
    int setInputMethod(const std::string&name);
    /**
     @param   newCursorPosition The new cursor position around the text,
     *        in Java characters. If > 0, this is relative to the end
     *        of the text - 1; if <= 0, this is relative to the start
     *        of the text. So a value of 1 will always advance the cursor
     *        to the position after the full text being inserted. Note that
     *        this means you can't position the cursor within the text,
     *        because the editor can make modifications to the text
     *        you are providing so it is not possible to correctly specify
     *        locations there.
    */
    void showIme();
};

}
#endif
