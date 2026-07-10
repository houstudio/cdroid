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
#include <core/windowmanager.h>

namespace cdroid{

#define NOW std::chrono::steady_clock::now().time_since_epoch().count()
class IMEWindow:public Window{
protected:
   friend InputMethodManager;
   View* mBuddy;
   KeyboardView* kbdView;
   CandidateView* candidateView;
   std::wstring mText2IM;
public:
   IMEWindow(int w,int h);
   ~IMEWindow(){
       LOGD("delete IMEWindow %p",this);
       InputMethodManager::getInstance().imeWindow=nullptr;
   }
   void updatePredicts(std::vector<std::string>candidates){
       candidateView->setSuggestions(candidates,true,true);
       LOGD("%d sugguestions",candidates.size());
       candidateView->invalidate(true);
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
   void onPredict(CandidateView&v,const std::string&s,int id){
       std::vector<std::string> predicts;
       LOGD("predict '%s' selected",s.c_str(),id);
       v.clear();
       InputMethod* im = InputMethodManager::getInstance().im;
       im->get_predicts(s,predicts);
       commitText(s);
       mText2IM.clear();
       candidateView->setSuggestions(predicts,true,true);
   }
   void onCloseKeyboard(View&v){
       LOGD("close IME'sKeyboard");
       Point sz;
       WindowManager::getInstance().getDefaultDisplay().getSize(sz);
       setPos(0,sz.y);
   }
};

IMEWindow::IMEWindow(int w,int h):Window(0,0,w,h,TYPE_SYSTEM_WINDOW){
    KeyboardView::OnKeyboardActionListener listener;
    InputMethodManager&imm = InputMethodManager::getInstance();
    View*vg=LayoutInflater::from(mContext)->inflate("@cdroid:layout/ime_pinyin_keyboard",this,false);
    kbdView = (KeyboardView*)vg->findViewById(cdroid::R::id::keyboardview);
    candidateView = (CandidateView*)vg->findViewById(cdroid::R::id::predict2);
    candidateView->setPredictListener(std::bind(&IMEWindow::onPredict,this,std::placeholders::_1,
           std::placeholders::_2,std::placeholders::_3));
    View* closeKbd = vg->findViewById(cdroid::R::id::closekeyboard);
    closeKbd->setOnClickListener(std::bind(&IMEWindow::onCloseKeyboard,this,std::placeholders::_1));
    addView(vg);//layout(0,0,getWidth(),h);
    vg->requestLayout();
    setId(123);
    setVisibility(INVISIBLE);
    listener.onPress=[this](int primaryCode){
        LOGD("primaryCode=%d %x",primaryCode,primaryCode);
        KeyEvent keyEvent;
        switch(primaryCode){
        case Keyboard::KEYCODE_SHIFT:
        case Keyboard::KEYCODE_ALT:
        case Keyboard::KEYCODE_DONE:
        case Keyboard::KEYCODE_CANCEL:
        case Keyboard::KEYCODE_MODE_CHANGE:break;
        case Keyboard::KEYCODE_BACKSPACE:
        case Keyboard::KEYCODE_DELETE:
             keyEvent.initialize(0,InputDevice::SOURCE_KEYBOARD,0,KeyEvent::ACTION_UP/*action*/,0,
             KeyEvent::KEYCODE_BACK,0/*scancode*/,0/*metaState*/,1/*repeatCount*/,NOW,NOW/*eventtime*/);
             /*sendKeyEvent(keyEvent);*/break;
        case -101:break;
        default:
            if(primaryCode>0){
                int rc;
                std::string u8txt;
                std::vector<std::string> candidates;
                InputMethod* im = InputMethodManager::getInstance().im;
                mText2IM.append(1,primaryCode);
                u8txt = TextUtils::unicode2utf8(mText2IM);
                rc = im->search(u8txt,candidates);
                if(rc < 0){
                    // InputMethod has no candidate search (e.g. the English/qwerty
                    // method) -> commit the typed character straight into the editor
                    // via EditText->Editor::commitText (the in-process equivalent of
                    // Android's InputConnection.commitText -> Editable).
                    commitText(u8txt);
                    mText2IM.clear();
                }else{
                    updatePredicts(candidates);
                }
                LOGD("txt=%s primaryCode=%x/%c",u8txt.c_str(),primaryCode,primaryCode);
            }break;
        }
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
        case Keyboard::KEYCODE_MODE_CHANGE://changeMode();break;
        case Keyboard::KEYCODE_SHIFT    :  changeCapital();break;
        case Keyboard::KEYCODE_DONE     :  break;
        case Keyboard::KEYCODE_DELETE:
        case Keyboard::KEYCODE_BACKSPACE:
             // Send Android's KEYCODE_DEL (backspace, deletes LEFT). sendKeyEvent()
             // dispatches onKeyDown, so use ACTION_DOWN. (KEYCODE_BACK is the Back
             // button and is not a delete.)
             keyEvent.initialize(0,InputDevice::SOURCE_KEYBOARD,0,KeyEvent::ACTION_DOWN/*action*/,0,KeyEvent::KEYCODE_DEL,
                        0/*scancode*/,0/*metaState*/,1/*repeatCount*/,NOW,NOW/*eventtime*/);
             imm.sendKeyEvent(keyEvent);break;
        }
    };
    kbdView->setOnKeyboardActionListener(listener);
#if 0
    kbdView->setButtonListener([&](const Keyboard::Key&k){
        std::vector<std::string>candidates;
        InputMethod*im=InputMethodManager::getInstance().im;
        LOGD("key %d modifer=%d sticky=%d im=%p",k.codes[0],k.modifier,k.sticky,im);
        if((k.modifier|k.sticky)==0){
            text2IM.append(1,k.codes[0]);
            std::string u8txt=TextUtils::unicode2utf8(text2IM);
            int rc=im->search(u8txt,candidates);
            updatePredicts(candidates);
            LOGD("key[%s]CHAR:%c u8txt=%s predicts=%d ",k.label.c_str(),k.codes[0],u8txt.c_str(),rc);
            if(rc<0){
                const wchar_t text[2]={k.codes[0],0};
                imm.commitText(text,1);
                text2IM.erase();
                return;
            }else if(k.codes[0]==' '){
                /*int idx=0;candidateView->getIndex();
                const std::string&txt=candidateView->getItem(idx)->getText();
                const std::wstring wtext=TextUtils::utf8tounicode(txt);
                imm.commitText(wtext,1);
                im->get_predicts(imm.predictSource,candidates);
                updatePredicts(candidates);
                im->close_search();*/
            }
        }else if(k.modifier|k.sticky){
            KeyEvent keyEvent;
            keyEvent.initialize(0,0,KeyEvent::ACTION_UP/*action*/,0,
                KEY_BACK,0/*scancode*/,0/*metaState*/,1/*repeatCount*/,NOW,NOW/*eventtime*/);
            LOGD("key[%s]code:%d keylabel=%s ",k.label.c_str(),k.codes[0],KeyEvent::getLabel(k.codes[0]));
            switch(k.codes[0]){
            case Keyboard::KEYCODE_MODE_CHANGE://changeMode();break;
            case Keyboard::KEYCODE_SHIFT    :  changeCapital();break;
            case Keyboard::KEYCODE_DONE     :  break;
            case Keyboard::KEYCODE_DELETE:
            case Keyboard::KEYCODE_BACKSPACE:  imm.sendKeyEvent(keyEvent);break;
            }
        }
    });
#endif
    /*candidateView->setItemClickListener([&](AbsListView&lv,const ListView::ListItem&itm,int index){
        std::wstring wtext;
        std::vector<std::string>candidates;
        InputMethod*im = InputMethodManager::getInstance().im;
        imm.predictSource = itm.getText();
        wtext = TextUtils::utf8tounicode(imm.predictSource);
        imm.commitText(wtext,1);
        text2IM = std::wstring();
        im->close_search();
        im->get_predicts(imm.predictSource,candidates);
        updatePredicts(candidates);
    });*/
}

InputMethodManager*InputMethodManager::mInst=nullptr;

int InputMethodManager::registeMethod(const std::string&name,InputMethod*method){
    imeMethods.push_back({name,method});
    LOGD("registeInputMethod(%s)%p",name.c_str(),method);
    return 0;
}

int InputMethodManager::getInputMethodCount()const{
    return (int)imeMethods.size();
}

InputMethod*InputMethodManager::getInputMethod(int idx){
    return imeMethods.at(idx).second;
}

std::vector<std::string>InputMethodManager::getInputMethods(std::vector<InputMethod*>*methods){
    std::vector<std::string>ms;
    if(methods)methods->clear();
    for(auto m:imeMethods){
        ms.push_back(m.first);
        if(methods)methods->push_back(m.second);
    }
    return ms;
}

InputMethod*InputMethodManager::getInputMethod(const std::string&name){
    for(auto m:imeMethods){
        if(m.first.compare(name)==0)
            return m.second;
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
    if(imeWindow)WindowManager::getInstance().removeWindow(imeWindow);
    for(auto ime:imeMethods){
        delete ime.second;
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
        InputMethod*m = new InputMethod("@cdroid:xml/qwerty.xml");
        mInst->registeMethod("English",m);
#ifdef ENABLE_PINYIN2HZ
        m = new GooglePinyin("@cdroid:xml/qwerty.xml");
        m->load_dicts("dict_pinyin.dat","userdict.dat");
        mInst->registeMethod("GooglePinyin26",m);
#endif
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
    switch(mInputType){
    case InputType::TYPE_NULL:
        if(imeWindow)imeWindow->setVisibility(View::INVISIBLE);
        break;
    default:
        if(imeMethods.size()){
            auto it = imeMethods.begin();
            setInputMethod(it->second,it->first);
        }break;
    }
    // Note: do NOT clear imeWindow->mBuddy here. Switching the keyboard layout is
    // independent of the commit target, and nulling the buddy would detach an
    // editor that is focused while its input type changes. The buddy is owned by
    // the focus/touch flow (viewClicked/focusIn/showSoftInput).
}

int InputMethodManager::setInputMethod(const std::string&name){
    InputMethod*im = getInputMethod(name);
    LOGE_IF(im==nullptr,"Inputmethod \"%s\" not found!",name.c_str());
    if(im) return setInputMethod(im,name);
    return -1;
}

void InputMethodManager::onViewDetachedFromWindow(View*view){
    if(imeWindow)imeWindow->mBuddy = nullptr;
    LOGD("view=%p  %d",view,view->getId());
}


int InputMethodManager::setInputMethod(InputMethod*method,const std::string&name){
    im = method;
    std::string layout = method->getKeyboardLayout(mInputType);
    Keyboard*kbd = new Keyboard(imeWindow->getContext(),layout,imeWindow->getWidth(),240);
    imeWindow->kbdView->setKeyboard(kbd);
    LOGD("inputmethod '%s':%p keyboardlayout:'%s' %p %d keys loaded",name.c_str(),im,layout.c_str(),kbd,kbd->getKeys().size());
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
