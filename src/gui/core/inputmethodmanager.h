/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __INPUT_METHOD_MANAGER_H__
#define __INPUT_METHOD_MANAGER_H__
#include <map>
#include <core/inputmethod.h>
#include <widget/keyboardview.h>
namespace cdroid{

class InputMethodManager{
public:
    /* A registered input method + the keyboard layout it uses. The layout lives
     * here (a UI concern), not on InputMethod (an engine), so the engine stays
     * decoupled from the keyboard UI. */
    struct ImMethod{
        std::string name;
        InputMethod* method;
        std::string layout;
    };
private:
    friend class IMEWindow;
    int mInputType;
    std::vector<ImMethod>imeMethods;
    int setInputMethod(InputMethod*,const std::string&name);
    void ensureIMEWindow();   // lazily create the on-screen IMEWindow
    void positionIMEWindow(); // place it docked to the bottom of the screen (undo any off-screen hide)
    /* Load a Keyboard from the given XML layout id and install it on the IME
     * window's KeyboardView. Shared by setInputType (class-driven layout) and
     * setInputMethod (method-switch layout). */
    void applyKeyboard(const std::string&layout);
    /* Resolve the letter (TYPE_CLASS_TEXT) layout: the active method's custom
     * LETTER layout if it supplies one, else its registered ImMethod layout,
     * else the first registered, else qwerty. */
    std::string activeTextLayout() const;
    /* 123/ABC toggle on the text keyboard: flip between the symbols page and the
     * active text method's layout. Independent of inputType (the field is still
     * text); reset by setInputType on every editor change. */
    void toggleSymbolMode();
protected:
    InputMethod*im;
    static class InputMethodManager*mInst;
    class IMEWindow*imeWindow;
    InputMethodManager();
    ~InputMethodManager();
public:
    static InputMethodManager&getInstance();
    static InputMethodManager*peekInstance();
    int registeMethod(const std::string&name,InputMethod*,const std::string&layout);
    std::vector<std::string>getInputMethods(std::vector<InputMethod*>*methods);
    int getInputMethodCount()const;
    InputMethod*getInputMethod(int idx);
    InputMethod*getInputMethod(const std::string&name);
    void shutDown(){delete mInst;}
    void viewClicked(View*v);
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
    /* ------------------------------------------------------------------
     *  Android-compatible adapter surface.
     *
     *  CDROID's IMM is an *in-process* on-screen-keyboard router: the IMEWindow
     *  is the single input sink and a "buddy" view is the commit target. There
     *  is no InputConnection / EditorInfo / ExtractedText protocol (intentionally
     *  deferred). These methods therefore map Android's IMM vocabulary onto the
     *  buddy+window model rather than reproducing the IME<->editor text sync:
     *    show/hide  -> real behaviour (toggle the keyboard window)
     *    isActive   -> real (this view is the current commit target)
     *    restartInput/updateSelection -> no-op-by-design here (documented)
     * ------------------------------------------------------------------ */
    bool isActive(View*v);                                   // imm.isActive(view)
    void showSoftInput(View*v,int flags);                    // imm.showSoftInput(view,0)
    void hideSoftInputFromView(View*v,int flags);            // imm.hideSoftInputFromView(view,0)
    void hideSoftInputFromWindow(View*v,int flags);          // no window token in CDROID -> view-keyed alias
    void restartInput(View*v);                               // imm.restartInput(view)
    void updateSelection(View*v,int selStart,int selEnd,int candidatesStart,int candidatesEnd);
};

}
#endif
