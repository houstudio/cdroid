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
       LOGV("...%d flags=%x",keyCode,evt.getFlags());
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
    LayoutInflater::from(mContext)->inflate("@cdroid:layout/ime_pinyin_keyboard",this);
    kbdView = (KeyboardView*)findViewById(cdroid::R::id::keyboardview);
    candidateView = (CandidateView*)findViewById(cdroid::R::id::predict2);
    candidateView->setPredictListener(std::bind(&IMEWindow::onPredict,this,std::placeholders::_1,
	   std::placeholders::_2,std::placeholders::_3));
    View* closeKbd = findViewById(cdroid::R::id::closekeyboard);
    closeKbd->setOnClickListener(std::bind(&IMEWindow::onCloseKeyboard,this,std::placeholders::_1));
    requestLayout();
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
	        updatePredicts(candidates);
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
        LOGV("primaryCode=%x %d keys",primaryCode,keyCodes.size());
        switch(primaryCode){
        case Keyboard::KEYCODE_MODE_CHANGE://changeMode();break;
        case Keyboard::KEYCODE_SHIFT    :  changeCapital();break;
        case Keyboard::KEYCODE_DONE     :  break;
        case Keyboard::KEYCODE_DELETE:
        case Keyboard::KEYCODE_BACKSPACE:
             keyEvent.initialize(0,InputDevice::SOURCE_KEYBOARD,0,KeyEvent::ACTION_UP/*action*/,0,KeyEvent::KEYCODE_BACK,
		        0/*scancode*/,0/*metaState*/,1/*repeatCount*/,NOW,NOW/*eventtime*/);
             imm.sendKeyEvent(keyEvent);break;
	}
    };
    kbdView->setOnKeyboardActionListener(listener);
#if 0
    kbdView->setButtonListener([&](const Keyboard::Key&k){
        std::vector<std::string>candidates;
        InputMethod*im=InputMethodManager::getInstance().im;
        LOGV("key %d modifer=%d sticky=%d im=%p",k.codes[0],k.modifier,k.sticky,im);
        if((k.modifier|k.sticky)==0){
            text2IM.append(1,k.codes[0]);
            std::string u8txt=TextUtils::unicode2utf8(text2IM);
            int rc=im->search(u8txt,candidates);
            updatePredicts(candidates);
            LOGV("key[%s]CHAR:%c u8txt=%s predicts=%d ",k.label.c_str(),k.codes[0],u8txt.c_str(),rc);
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
            LOGV("key[%s]code:%d keylabel=%s ",k.label.c_str(),k.codes[0],KeyEvent::getLabel(k.codes[0]));
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
    kcm= nullptr;
    mInputType = 0;
    imeWindow = nullptr;
}

InputMethodManager::~InputMethodManager(){
    mInst = nullptr;
    delete kcm;
    LOGD("InputMethodManager Destroied!");
    if(imeWindow)WindowManager::getInstance().removeWindow(imeWindow);
    for(auto ime:imeMethods){
        delete ime.second;
    }
    imeMethods.clear();
}

int InputMethodManager::setKeyCharacterMap(const std::string&filename){
    std::ifstream fs(filename);
    delete kcm;
    kcm = nullptr;
    if(fs.good()){
        KeyCharacterMap::load(filename,fs,KeyCharacterMap::FORMAT_ANY,kcm);
    }else{
        std::shared_ptr<std::istream>in=App::getInstance().getInputStream(filename);
        if(in)KeyCharacterMap::load(filename,*in,KeyCharacterMap::FORMAT_ANY,kcm);
    }
    LOGI("load Keyboard Character map from %s  kcm=%p",filename.c_str(),kcm);
    return kcm?0:-1;
}

InputMethodManager& InputMethodManager::getInstance(){
    if(mInst == nullptr){
        mInst = new InputMethodManager();
        if(mInst->setKeyCharacterMap("Generic.kcm"))
            mInst->setKeyCharacterMap("qwerty.kcm");
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

int InputMethodManager::getCharacter(int keycode,int metaState)const{
    LOGE_IF(kcm==nullptr,"KeyCharacterMap(kcm) not setted,cant map keycode to character!!!");
    if(kcm == nullptr)return keycode;
    return kcm->getCharacter(keycode,metaState);
}

void InputMethodManager::viewClicked(View*view){
    if(imeWindow)imeWindow->mBuddy = view;
}

void InputMethodManager::focusIn(View*view){
    if(imeWindow)imeWindow->mBuddy = view;
    LOGV("imeWindow=%d buddy=%p %d",imeWindow,view,view->getId());
}

void InputMethodManager::focusOut(View*view){
    if(imeWindow){
        imeWindow->setVisibility(View::INVISIBLE);
    }
    LOGV("view=%p %d",view,view->getId());
}

void InputMethodManager::showIme(){
    LOGV("imeWindow=%p",imeWindow);
    if(imeWindow)imeWindow->setVisibility(View::VISIBLE);
}

void InputMethodManager::sendKeyEvent(KeyEvent&k){
    if(imeWindow&&imeWindow->mBuddy)imeWindow->mBuddy->onKeyDown(k.getKeyCode(),k);
    LOGD("%s",KeyEvent::keyCodeToString(k.getKeyCode()).c_str());
}

void InputMethodManager::setInputType(int inputType){
    LOGV("type=%d",inputType);
    if(mInputType==inputType)
        return;
    mInputType = inputType;
    if( mInst->imeWindow == nullptr){
        Point dspSize;
        mInst->imeWindow = new IMEWindow(-1,300);
        Display& dp = WindowManager::getInstance().getDefaultDisplay();
        const int rotation = dp.getRotation();
        dp.getRealSize(dspSize);
        const int screenHeight=((rotation==Display::ROTATION_90)||(rotation==Display::ROTATION_270))?dspSize.x:dspSize.y;
        mInst->imeWindow->setPos(0,screenHeight-mInst->imeWindow->getHeight());
        LOGD("screenHeight=%d imewin.height=%d",screenHeight,mInst->imeWindow->getHeight());
    }
    switch(mInputType){
    case EditText::TYPE_NONE:
        if(imeWindow)imeWindow->setVisibility(View::INVISIBLE);
        break;
    default:
        if(imeMethods.size()){
            auto it = imeMethods.begin();
            setInputMethod(it->second,it->first);
        }break;
    }
    if(mInst->imeWindow){
        //imeWindow->kbdView->setKeyboard(kbd);
        imeWindow->mBuddy = nullptr;
    }
}

int InputMethodManager::setInputMethod(const std::string&name){
    InputMethod*im = getInputMethod(name);
    LOGE_IF(im==nullptr,"Inputmethod \"%s\" not found!",name.c_str());
    if(im) return setInputMethod(im,name);
    return -1;
}

void InputMethodManager::onViewDetachedFromWindow(View*view){
    if(imeWindow)imeWindow->mBuddy = nullptr;
    LOGV("view=%p  %d",view,view->getId());
}

void InputMethodManager::commitText(const std::wstring&text,int newCursorPos){
    if(imeWindow&&imeWindow->mBuddy){
        imeWindow->mBuddy->commitText(text);
    }
}

int InputMethodManager::setInputMethod(InputMethod*method,const std::string&name){
    im = method;
    std::string layout = method->getKeyboardLayout(mInputType);
    Keyboard*kbd = new Keyboard(imeWindow->getContext(),layout,imeWindow->getWidth(),240);
    imeWindow->kbdView->setKeyboard(kbd);
    LOGD("inputmethod '%s':%p keyboardlayout:'%s' %p",name.c_str(),im,layout.c_str(),kbd);
    return 0;
}

}
