#include <inputmethodmanager.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <chrono>
#include <fstream>
#include <keycharactermap.h>
#include <textutils.h>

namespace cdroid{

#define NOW std::chrono::steady_clock::now().time_since_epoch().count()
class IMEWindow:public Window{
protected:
   friend InputMethodManager;
   View*mBuddy;
   //ToolBar*candidateView;
   KeyboardView*kbdView;
   std::wstring text2IM;
public:
   IMEWindow(int w,int h);
   ~IMEWindow(){
      LOGD("delete IMEWindow %p",this);
      InputMethodManager::getInstance().imeWindow=nullptr;
   }
   void updatePredicts(std::vector<std::string>candidates){
       //candidateView->clear();
       //for(auto c:candidates)     candidateView->addButton(c);
   }
   void onSizeChanged(int w,int h,int ow,int oh)override{
       kbdView->onSizeChanged(w,h,ow,oh);
       Window::onSizeChanged(w,h,ow,oh);
   }
   bool onKeyUp(int keyCode,KeyEvent& evt)override{
       LOGV("...%d flags=%x",keyCode,evt.getFlags());
       switch(keyCode){
       case KEY_ESCAPE:setVisibility(View::INVISIBLE);return true;
       default: return Window::onKeyDown(keyCode,evt);
       }
   }
   void changeCapital(){
       /*Keyboard&keyboard=kbdView->getKeyboard();
       for(int i=0;i<keyboard.getRows();i++){
           Keyboard::KeyRow&row=keyboard.getKeyRow(i);
           for(int j=0;j<row.size();j++){
               Keyboard::Key&k=row[j];
               if((isalpha(k.codes[0])==false)||k.modifier||k.sticky)continue;
               k.codes[0]=islower(k.codes[0])?toupper(k.codes[0]):tolower(k.codes[0]);
           }
       }*/
   }
};

IMEWindow::IMEWindow(int w,int h):Window(0,0,w,h,TYPE_SYSTEM_WINDOW){
    InputMethodManager&imm=InputMethodManager::getInstance();
    KeyboardView::OnKeyboardActionListener listener;
    //candidateView=new ToolBar(w,28);
    kbdView=new KeyboardView(w,h-30);
    setText("IME");
    //addView(candidateView).setPos(0,1);
    addView(kbdView).setPos(0,30);
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
             keyEvent.initialize(0,0,KeyEvent::ACTION_UP/*action*/,0,
             KEY_BACK,0/*scancode*/,0/*metaState*/,1/*repeatCount*/,NOW,NOW/*eventtime*/);
             /*sendKeyEvent(keyEvent);*/break;
        case -101:break;
        default:
            if(mBuddy){
                const wchar_t text[2]={primaryCode,0};
                mBuddy->commitText(text);
            }break;
        }
    };
    listener.onRelease=[](int primaryCode){
        LOGD("primaryCode=%x",primaryCode);
    };
    listener.onText=[](std::string&text){
        LOGD("onText(%s)",text.c_str());
    };
    listener.onKey=[](int primaryCode,const std::vector<int>&keyCodes){
        LOGV("primaryCode=%x %d keys",primaryCode,keyCodes.size());
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
                im->get_predicts(imm.predictsource,candidates);
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
        InputMethod*im=InputMethodManager::getInstance().im;
        imm.predictsource=itm.getText();
        wtext=TextUtils::utf8tounicode(imm.predictsource);
        imm.commitText(wtext,1);
        text2IM=std::wstring();
        im->close_search();
        im->get_predicts(imm.predictsource,candidates);
        updatePredicts(candidates);;
    });*/
}

InputMethodManager*InputMethodManager::mInst=nullptr;
std::map<std::string,InputMethod*>InputMethodManager::imemethods;

int InputMethodManager::registeMethod(const std::string&name,InputMethod*method){
    imemethods.insert(std::pair<std::string,InputMethod*>(name,method));
    LOGD("registeInputMethod(%s)%p",name.c_str(),method);
    return 0;
}

std::vector<std::string>InputMethodManager::getInputMethods(std::vector<InputMethod*>*methods){
    std::vector<std::string>ms;
    if(methods)methods->clear();
    for(auto m:imemethods){
        ms.push_back(m.first);
        if(methods)methods->push_back(m.second);
    }
    return ms;
}

InputMethodManager::InputMethodManager(){
    im=nullptr;
    kcm=nullptr;
    imeWindow=nullptr;
}

InputMethodManager::~InputMethodManager(){
    mInst=nullptr;
    delete kcm;
    LOGD("InputMethodManager Destroied!");
    if(imeWindow)WindowManager::getInstance().removeWindow(imeWindow);
    for_each(imemethods.begin(),imemethods.end(),[](std::map<std::string,InputMethod*>::reference it){
        delete it.second;
    });
    imemethods.erase(imemethods.begin(),imemethods.end());
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

InputMethodManager&InputMethodManager::getInstance(){
    if(mInst==nullptr){
        mInst=new InputMethodManager();
        if(mInst->setKeyCharacterMap("Generic.kcm"))
            mInst->setKeyCharacterMap("qwerty.kcm");
    }
    if(imemethods.size()==0){
        InputMethod*m=new InputMethod("cdroid:values/qwerty.xml");
        m->load_dicts("dict_pinyin.dat","userdict.dat");
        registeMethod("ChinesePinyin26",m);
    }
    return *mInst;
}

int InputMethodManager::getCharacter(int keycode,int metaState)const{
    LOGE_IF(kcm==nullptr,"KeyCharacterMap(kcm) not setted,cant map keycode to character!!!");
    if(kcm==nullptr)return keycode;
    return kcm->getCharacter(keycode,metaState);
}

void InputMethodManager::focusIn(View*view){
    if(imeWindow)imeWindow->mBuddy=view;
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
    LOGD("%s",k.getLabel());
}

void InputMethodManager::setInputType(int inputType){
    LOGV("type=%d",inputType);
    if( mInst->imeWindow==nullptr){
        mInst->imeWindow=new IMEWindow(1280,300);
        mInst->imeWindow->setPos(0,420);
    }
    if(mInputType!=inputType){
        mInputType=inputType;
    }
    switch(mInputType){
    case EditText::TYPE_NONE:
        if(imeWindow)imeWindow->setVisibility(View::INVISIBLE);
        break;
    default:
        if(imemethods.size()){
            auto it=imemethods.begin();
            setInputMethod(it->second,it->first);
        }break;
    }
    if(mInst->imeWindow){
        //imeWindow->kbdView->setKeyboard(kbd);
        imeWindow->mBuddy=nullptr;
    }
}

int InputMethodManager::setInputMethod(const std::string&name){
    auto it=imemethods.find(name);
    LOGE_IF(it==imemethods.end(),"Inputmethod \"%s\" not found!",name.c_str());
    if(it!=imemethods.end())
        return setInputMethod(it->second,it->first);
    return -1;
}

void InputMethodManager::onViewDetachedFromWindow(View*view){
    if(imeWindow)imeWindow->mBuddy=nullptr;
    LOGV("view=%p  %d",view,view->getId());
}

void InputMethodManager::commitText(const std::wstring&text,int newCursorPos){
    if(imeWindow&&imeWindow->mBuddy){
        imeWindow->mBuddy->commitText(text);
    }
}

int InputMethodManager::setInputMethod(InputMethod*method,const std::string&name){
    im=method;
    std::string layout =method->getKeyboardLayout(mInputType);
    Keyboard*kbd=new Keyboard(imeWindow->getContext(),layout,1280,240);
    imeWindow->kbdView->setKeyboard(kbd);
    LOGD("inputmethod '%s':%p keyboardlayout:'%s' %p",name.c_str(),im,layout.c_str(),kbd);
    return 0;
}

}
