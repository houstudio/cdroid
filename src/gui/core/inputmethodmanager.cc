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
#include <inputmethodmanager.h>
#include <text/inputtype.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <chrono>
#include <fstream>
#include <keycharactermap.h>
#include <utils/textutils.h>
#include <widget/candidateview.h>
#include <widget/R.h>
#include <core/app.h>
#include <core/englishinputmethod.h>
#include <core/googlepinyin.h>
#include <core/imeselectioncontroller.h>
#include <core/windowmanager.h>

namespace cdroid{

#define NOW std::chrono::steady_clock::now().time_since_epoch().count()
class IMEWindow:public Window{
protected:
   friend InputMethodManager;
   View* mBuddy;
   KeyboardView* kbdView;
   CandidateView* candidateView;
   /* Owns all the 1/2-level selection logic (search/choose/predict/backspace),
    * decoupled from this window so other keyboards can reuse it. */
   ImeSelectionController* mController = nullptr;
   /* Numeric/phone/datetime keyboards commit each key straight to the editor,
    * bypassing the composition/candidate engine (a digit fed to onChar would
    * be held as composing text and never committed). True while such a layout
    * is shown. */
   bool mDirectCommit = false;
   /* User pressed the 123 key on the text keyboard -> the symbols layout is
    * shown temporarily; ABC switches back. Independent of inputType (the field
    * is still text). Reset whenever setInputType loads a fresh layout. */
   bool mSymbolMode = false;
public:
   IMEWindow(int w,int h);
   ~IMEWindow(){
       LOGD("delete IMEWindow %p",this);
       delete mController;
       InputMethodManager::getInstance().imeWindow=nullptr;
   }
   /* The active engine changed (method switch); hand it to the controller. */
   void setActiveMethod(InputMethod* im){ if(mController) mController->setInputMethod(im); }
   /* Toggle direct-commit mode for the numeric/phone/datetime layouts. Entering
    * it drops any half-composed text so a stale composition does not leak back
    * when the user returns to a text field. */
   void setDirectCommit(bool d){
       if(d && mController) mController->reset();
       mDirectCommit = d;
   }
   void onSizeChanged(int w,int h,int ow,int oh)override{
       kbdView->onSizeChanged(w,h,ow,oh);
       Window::onSizeChanged(w,h,ow,oh);
   }
   bool onKeyUp(int keyCode,KeyEvent& evt)override{
       LOGD("...%d flags=%x",keyCode,evt.getFlags());
       switch(keyCode){
       case KeyEvent::KEYCODE_ESCAPE:setVisibility(View::INVISIBLE);return true;
       default: return Window::onKeyDown(keyCode,evt);
       }
   }
   /* Deliver a committed UTF-8 string to the editor (the controller's committer
    * target). */
   void commitText(const std::string&txt){
       const std::wstring uniTxt = TextUtils::utf8tounicode(txt);
       if(mBuddy)mBuddy->commitText(uniTxt);
   }
   void changeCapital(){
       Keyboard*keyboard = kbdView->getKeyboard();
       std::vector<Keyboard::Key*>&keys = keyboard->getKeys();
       for(int i=0;i<keys.size();i++){
           Keyboard::Key& k = *keys[i];
           if((isalpha(k.codes[0])==false)||k.modifier||k.sticky)continue;
           k.codes[0]=islower(k.codes[0])?toupper(k.codes[0]):tolower(k.codes[0]);
       }
   }
   void onCloseKeyboard(View&v){
       LOGD("close IME'sKeyboard");
       // Just hide. setVisibility(INVISIBLE) triggers Window::onVisibilityChanged,
       // which calls WindowManager::hideWindow to repaint the uncovered area; and
       // INVISIBLE itself takes the window out of touch routing + key dispatch so
       // a dismissed keyboard stops consuming input. showSoftInput brings it back.
       setVisibility(View::INVISIBLE);
   }
};

IMEWindow::IMEWindow(int w,int h):Window(0,0,w,h,TYPE_SYSTEM_WINDOW){
    // The IME window stays focusable (default). It only consumes input events
    // while VISIBLE; hiding it via setVisibility(INVISIBLE) removes it from both
    // touch routing and key dispatch in WindowManager, so a dismissed keyboard
    // does not keep eating events.
    KeyboardView::OnKeyboardActionListener listener;
    InputMethodManager&imm = InputMethodManager::getInstance();
    View*vg=LayoutInflater::from(mContext)->inflate("@cdroid:layout/ime_pinyin_keyboard",this,false);
    kbdView = (KeyboardView*)vg->findViewById(cdroid::R::id::keyboardview);
    candidateView = (CandidateView*)vg->findViewById(cdroid::R::id::predict2);
    // The controller owns the 1/2-level selection logic; the committer delivers
    // each committed phrase/word to the focused editor via this->commitText.
    mController = new ImeSelectionController(candidateView,
        [this](const std::string& s){ commitText(s); });
    candidateView->setPredictListener([this](CandidateView&,const std::string&s,int id){
        mController->onCandidateSelected(s,id);
    });
    View* closeKbd = vg->findViewById(cdroid::R::id::closekeyboard);
    closeKbd->setOnClickListener(std::bind(&IMEWindow::onCloseKeyboard,this,std::placeholders::_1));
    addView(vg);//layout(0,0,getWidth(),h);
    vg->requestLayout();
    setId(123);
    setVisibility(INVISIBLE);
    listener.onPress=[this](int primaryCode){
        LOGD("primaryCode=%d %x",primaryCode,primaryCode);
        // Chars are committed on RELEASE via onKey (not here on press), so that
        // a long-press popup can abort the base char through mAbortKey (AOSP:
        // detectAndSendKey/onKey runs on ACTION_UP and is skipped when aborted).
        (void)primaryCode;
    };
    listener.onRelease = [this](int primaryCode){
    };
    listener.onText= [](std::string&text){
        LOGD("onText(%s)",text.c_str());
    };
    listener.onKey = [this](int primaryCode,const std::vector<int>&keyCodes){
        KeyEvent keyEvent;
        InputMethodManager&imm = InputMethodManager::getInstance();
        LOGD("primaryCode=%x %d keys",primaryCode,keyCodes.size());
        switch(primaryCode){
        case Keyboard::KEYCODE_MODE_CHANGE: imm.toggleSymbolMode(); break;
        case Keyboard::KEYCODE_SHIFT    :  changeCapital();break;
        case Keyboard::KEYCODE_DONE     :  break;
        case Keyboard::KEYCODE_DELETE:
        case Keyboard::KEYCODE_BACKSPACE:
             // Composing-aware: while a pinyin is in progress the backspace edits
             // the composing buffer (undo last fixed word, or drop the last typed
             // char); only when nothing is composing do we forward a real
             // KEYCODE_DEL to delete in the editor.
             if(!mController->onBackspace()){
                 keyEvent.initialize(0,InputDevice::SOURCE_KEYBOARD,0,KeyEvent::ACTION_DOWN/*action*/,0,KeyEvent::KEYCODE_DEL,
                            0/*scancode*/,0/*metaState*/,1/*repeatCount*/,NOW,NOW/*eventtime*/);
                 imm.sendKeyEvent(keyEvent);
             }break;
        default:
             // Printable key delivered on RELEASE (AOSP onKey model). A long-
             // press popup aborts the base char via mAbortKey, and the chosen
             // accent arrives here while the popup is still showing -- commit
             // it directly (pickChar) instead of composing (onChar).
             if(primaryCode>0){
                 if(mDirectCommit)
                     // Numeric/phone/datetime: commit straight to the editor,
                     // no composition / candidate strip (a digit through onChar
                     // would be held as composing and never committed).
                     commitText(std::string(1,(char)primaryCode));
                 else if(kbdView && kbdView->isMiniKeyboardOnScreen())
                     mController->pickChar(primaryCode);
                 else
                     mController->onChar(primaryCode);
             }break;
        }
    };
    kbdView->setOnKeyboardActionListener(listener);
}

InputMethodManager*InputMethodManager::mInst=nullptr;

int InputMethodManager::registeMethod(const std::string&name,InputMethod*method,const std::string&layout){
    imeMethods.push_back({name,method,layout});
    LOGD("registeInputMethod(%s)%p layout=%s",name.c_str(),method,layout.c_str());
    return 0;
}

int InputMethodManager::getInputMethodCount()const{
    return (int)imeMethods.size();
}

InputMethod*InputMethodManager::getInputMethod(int idx){
    return imeMethods.at(idx).method;
}

std::vector<std::string>InputMethodManager::getInputMethods(std::vector<InputMethod*>*methods){
    std::vector<std::string>ms;
    if(methods)methods->clear();
    for(auto m:imeMethods){
        ms.push_back(m.name);
        if(methods)methods->push_back(m.method);
    }
    return ms;
}

InputMethod*InputMethodManager::getInputMethod(const std::string&name){
    for(auto m:imeMethods){
        if(m.name.compare(name)==0)
            return m.method;
    }
    return nullptr;
}

InputMethodManager::InputMethodManager(){
    im = nullptr;
    mInputType = 0;
    imeWindow = nullptr;
}

InputMethodManager::~InputMethodManager(){
    mInst = nullptr;
    LOGD("InputMethodManager Destroied!");
    if(imeWindow){
        imeWindow->close();
        imeWindow = nullptr;
    }
    for(auto ime:imeMethods){
        delete ime.method;
    }
    imeMethods.clear();
}

InputMethodManager& InputMethodManager::getInstance(){
    if(mInst == nullptr){
        mInst = new InputMethodManager();
        // The device KeyCharacterMap is no longer loaded here — it is per-device
        // now (InputDevice loads its .kcm; KeyEvent resolves via its deviceId).
    }
    if(mInst->imeMethods.size() == 0){
        // English uses a word-completion method (built-in baseline word list,
        // optionally overridden by english_words.txt in the data path). If the
        // override file is absent it simply keeps the built-in list.
        InputMethod*m = new EnglishInputMethod();
        m->loadDicts(App::getInstance().getDataPath() + "english_words.txt", "");
        mInst->registeMethod("English",m,"@cdroid:xml/qwerty.xml");
#ifdef ENABLE_PINYIN2HZ
        m = new GooglePinyin();
        m->loadDicts("dict_pinyin.dat","userdict.dat");
        mInst->registeMethod("GooglePinyin26",m,"@cdroid:xml/qwerty.xml");
#endif
        // Default the active method to the first registered one so the
        // composition controller always has an engine before any explicit
        // setInputMethod() (otherwise onChar() no-ops on mIm==null).
        mInst->im = mInst->imeMethods.begin()->method;
    }
    return *mInst;
}

InputMethodManager* InputMethodManager::peekInstance(){
    return mInst;
}

void InputMethodManager::viewClicked(View*view){
    if(imeWindow)imeWindow->mBuddy = view;
}

void InputMethodManager::focusIn(View*view){
    if(imeWindow)imeWindow->mBuddy = view;
    LOGD("imeWindow=%d buddy=%p %d",imeWindow,view,view->getId());
}

void InputMethodManager::focusOut(View*view){
    // setVisibility(INVISIBLE) -> onVisibilityChanged -> WindowManager::hideWindow
    // repaints the uncovered area.
    if(imeWindow){
        imeWindow->setVisibility(View::INVISIBLE);
    }
    LOGD("view=%p %d",view,view->getId());
}

void InputMethodManager::showIme(){
    LOGD("imeWindow=%p",imeWindow);
    if(imeWindow)imeWindow->setVisibility(View::VISIBLE);
}

void InputMethodManager::sendKeyEvent(KeyEvent&k){
    if(imeWindow&&imeWindow->mBuddy)imeWindow->mBuddy->onKeyDown(k.getKeyCode(),k);
    LOGD("%s",KeyEvent::keyCodeToString(k.getKeyCode()).c_str());
}

void InputMethodManager::ensureIMEWindow(){
    if(mInst->imeWindow != nullptr) return;
    mInst->imeWindow = new IMEWindow(-1,300);
    positionIMEWindow();
    LOGD("IMEWindow created: height=%d",mInst->imeWindow->getHeight());
}

void InputMethodManager::positionIMEWindow(){
    // Dock the keyboard to the bottom of the screen. The close button hides the
    // window by setPos()-ing it off-screen, so every (re)show must move it back,
    // otherwise setVisibility(VISIBLE) alone leaves it parked below the screen.
    if(imeWindow == nullptr) return;
    Point dspSize;
    Display& dp = WindowManager::getInstance().getDefaultDisplay();
    const int rotation = dp.getRotation();
    dp.getRealSize(dspSize);
    const int screenHeight=((rotation==Display::ROTATION_90)||(rotation==Display::ROTATION_270))?dspSize.x:dspSize.y;
    imeWindow->setPos(0,screenHeight-imeWindow->getHeight());
}

void InputMethodManager::setInputType(int inputType){
    LOGD("setInputType type=%d (prev=%d)",inputType,mInputType);
    ensureIMEWindow(); // the window must exist before we can show/hide or switch keyboards
    if(mInputType==inputType)
        return;
    mInputType = inputType;
    if(inputType==InputType::TYPE_NULL){
        if(imeWindow)imeWindow->setVisibility(View::INVISIBLE);
        return;
    }
    // A new editor resets the 123/ABC toggle -- always start in the layout the
    // inputType asks for, never inherit a stale symbol-mode layout.
    if(imeWindow) imeWindow->mSymbolMode = false;
    // Decode the inputType class and pick a matching soft-keyboard layout, plus
    // whether keys commit straight to the editor (numeric/phone/datetime, no
    // composition). The active InputMethod is asked FIRST for a custom layout
    // for THIS inputType (getKeyboardLayout(int) -- it may vary by class AND
    // variation, e.g. password); empty -> built-in default. This is the
    // customization hook: a product subclass overrides getKeyboardLayout to ship
    // its own keyboards. Android's full design routes this through EditorInfo +
    // onCreateInputConnection; this is the in-process equivalent (path A).
    const int cls = inputType & InputType::TYPE_MASK_CLASS;
    if(im==nullptr && !imeMethods.empty()) im = imeMethods.begin()->method;
    const std::string custom = im ? im->getKeyboardLayout(inputType) : std::string();
    std::string layout;
    bool directCommit = false;
    switch(cls){
    case InputType::TYPE_CLASS_NUMBER:
        layout = !custom.empty()?custom:"@cdroid:xml/keyboard_number.xml";
        directCommit = true; break;
    case InputType::TYPE_CLASS_PHONE:
        layout = !custom.empty()?custom:"@cdroid:xml/keyboard_phone.xml";
        directCommit = true; break;
    case InputType::TYPE_CLASS_DATETIME:
        layout = !custom.empty()?custom:"@cdroid:xml/keyboard_datetime.xml";
        directCommit = true; break;
    default:
        // TYPE_CLASS_TEXT (and anything else): the method's custom layout, else
        // its registered layout, else qwerty.
        layout = !custom.empty()?custom:activeTextLayout();
        directCommit = false;
        // Ensure the composition controller has an engine (see the null-guard
        // above); without it onChar() bails and text keys never reach the editor.
        if(imeWindow) imeWindow->setActiveMethod(im);
        break;
    }
    applyKeyboard(layout);
    imeWindow->setDirectCommit(directCommit);
    // Note: do NOT clear imeWindow->mBuddy here. Switching the keyboard layout is
    // independent of the commit target, and nulling the buddy would detach an
    // editor that is focused while its input type changes. The buddy is owned by
    // the focus/touch flow (viewClicked/focusIn/showSoftInput).
}

void InputMethodManager::applyKeyboard(const std::string&layout){
    if(imeWindow==nullptr) return;
    // AOSP sizes a Keyboard from the display metrics (the %p base). CDROID's
    // Keyboard ctor takes that width explicitly; imeWindow->getWidth() can be 0
    // or stale before the window is laid out, which parses every %p gap/width
    // to 0 and the keys then render flush with no gaps (resize() only reflows
    // widths on overflow, never gaps). Use the screen width so the geometry is
    // stable across every (re)show.
    Point dspSize;
    Display& dp = WindowManager::getInstance().getDefaultDisplay();
    const int rot = dp.getRotation();
    dp.getRealSize(dspSize);
    const int screenW = (rot==Display::ROTATION_90||rot==Display::ROTATION_270) ? dspSize.y : dspSize.x;
    Keyboard*kbd = new Keyboard(imeWindow->getContext(),layout,screenW,240);
    imeWindow->kbdView->setKeyboard(kbd);
    // A product's InputMethod may supply a custom long-press popup container
    // (getKeyboardLayout(POPUP)); apply it as the KeyboardView's popup layout so
    // the accent mini-keyboard window is customizable. Empty -> keep the
    // popupLayout declared in the IME layout (keyboard_popup_keyboard.xml).
    if(im){
        const std::string popup = im->getKeyboardLayout(InputMethod::POPUP);
        if(!popup.empty()) imeWindow->kbdView->setPopupLayout(popup);
    }
    LOGD("applyKeyboard layout='%s' w=%d %p %d keys",layout.c_str(),screenW,kbd,kbd->getKeys().size());
}

std::string InputMethodManager::activeTextLayout() const {
    // The method's custom layout for the field is resolved by the caller (via
    // getKeyboardLayout(inputType)); here we only fall back to the method's
    // registered ImMethod layout, then the first registered, then qwerty.
    if(im){
        for(const auto&mthd:imeMethods){ if(mthd.method==im) return mthd.layout; }
    }
    if(!imeMethods.empty()) return imeMethods.begin()->layout;
    return "@cdroid:xml/qwerty.xml";
}

void InputMethodManager::toggleSymbolMode(){
    if(imeWindow==nullptr) return;
    imeWindow->mSymbolMode = !imeWindow->mSymbolMode;
    std::string layout;
    if(imeWindow->mSymbolMode){
        // 123 pressed on the text keyboard: show the built-in symbols page
        // (digits + punctuation, with an ABC key to switch back). The symbol
        // page is IME-internal (not inputType-driven), so it is not routed
        // through getKeyboardLayout. The field is still text, so keys still go
        // through the composition engine (directCommit stays off); a digit/
        // symbol commits via the engine's word-boundary flush.
        layout = "@cdroid:xml/symbols.xml";
    } else {
        // ABC pressed: restore the field's text layout (custom if the method
        // supplies one for the current inputType, else its registered layout).
        const std::string c = im?im->getKeyboardLayout(mInputType):std::string();
        layout = !c.empty()?c:activeTextLayout();
    }
    applyKeyboard(layout);
    LOGD("toggleSymbolMode -> %d", (int)imeWindow->mSymbolMode);
}

int InputMethodManager::setInputMethod(const std::string&name){
    InputMethod*im = getInputMethod(name);
    LOGE_IF(im==nullptr,"Inputmethod \"%s\" not found!",name.c_str());
    if(im) return setInputMethod(im,name);
    return -1;
}

void InputMethodManager::onViewDetachedFromWindow(View*view){
    /* AOSP's IMM only drops its served-view state when the served view itself
     * (or an ancestor) detaches. CDROID collapses that to the single mBuddy
     * pointer, so mirror the guard: null mBuddy ONLY when this view is the
     * buddy. Otherwise a transient popup dismissing (its contentView subtree
     * detaches) would wrongly clear the editor target, and the next popup
     * accent pick would commit to nothing until the editor is tapped again. */
    if(imeWindow && imeWindow->mBuddy == view)
        imeWindow->mBuddy = nullptr;
    LOGV("view=%p  %d",view,view?view->getId():-1);
}


int InputMethodManager::setInputMethod(InputMethod*method,const std::string&name){
    im = method;
    // The controller keeps its own pointer to the active engine; update it on
    // every method switch (and reset any half-composed pinyin).
    if(imeWindow) imeWindow->setActiveMethod(method);
    // The keyboard layout is a UI concern owned by the registered ImMethod, not
    // by the InputMethod engine (the base engine has no layout of its own); look
    // it up by the method name here.
    std::string layout;
    for(const auto&mthd:imeMethods){
        if(mthd.name==name){ layout = mthd.layout; break; }
    }
    // Switching to a named text method always leaves direct-commit off -- the
    // engine drives composition/candidates.
    if(imeWindow) imeWindow->setDirectCommit(false);
    applyKeyboard(layout);
    LOGD("inputmethod '%s':%p keyboardlayout:'%s'",name.c_str(),im,layout.c_str());
    return 0;
}

bool InputMethodManager::isActive(View*v){
    return imeWindow != nullptr && imeWindow->mBuddy == v;
}

void InputMethodManager::showSoftInput(View*v,int /*flags*/){
    // Android: request the IME to show for this view. In the in-process model
    // that is: designate v as the commit target and make the keyboard visible.
    // Never silently no-op: ensure the keyboard window exists first (it used to
    // be created lazily only inside setInputType, so a TYPE_NULL/untyped editor
    // would leave it null and the keyboard would never appear).
    ensureIMEWindow();
    if(imeWindow == nullptr) return; // only if creation failed
    imeWindow->mBuddy = v;
    positionIMEWindow();             // undo any off-screen hide from the close button
    imeWindow->setVisibility(View::VISIBLE);
    imeWindow->requestLayout(); // make sure the freshly-shown subtree is measured/laid out
    LOGD("showSoftInput view=%p type=%d imeWindow=%p",v,mInputType,imeWindow);
}

void InputMethodManager::hideSoftInputFromView(View*v,int /*flags*/){
    // Hide the keyboard only when it is currently attached to v (matching
    // Android's "from this view" semantics).
    // setVisibility(INVISIBLE) -> onVisibilityChanged -> WindowManager::hideWindow
    // repaints the uncovered area.
    if(imeWindow && imeWindow->mBuddy == v){
        imeWindow->setVisibility(View::INVISIBLE);
    }
    LOGD("hideSoftInputFromView view=%p",v);
}

void InputMethodManager::hideSoftInputFromWindow(View*v,int flags){
    // CDROID has no IBinder window token; key the hide by the view instead.
    hideSoftInputFromView(v,flags);
}

void InputMethodManager::restartInput(View*v){
    // Android: notify the IME that the editor's content/type changed so it
    // re-queries the editor through its InputConnection. CDROID's in-process IME
    // caches no editor text or composing state (it commits keystrokes directly),
    // so there is nothing to re-push here: the keyboard layout is driven by
    // setInputType(), which TextView pushes explicitly when the type changes or
    // when an editor is shown. Intentional no-op, kept so call sites read like
    // Android.
    LOGV("restartInput view=%p (in-process IME: no-op)",v);
}

void InputMethodManager::updateSelection(View* /*v*/,int /*selStart*/,int /*selEnd*/,
                                          int /*candidatesStart*/,int /*candidatesEnd*/){
    // In-process IME: selection and composing state are not tracked here. The
    // composing buffer lives inside IMEWindow (independent of the editor's
    // Selection spans), and committed text flows directly into the buddy view.
    // Intentional no-op, kept as a real call so TextView/Editor read like Android.
}

}
