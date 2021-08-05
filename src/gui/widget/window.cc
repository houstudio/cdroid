/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <windows.h>
#include <widget/window.h>
#include <widget/textview.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <uieventsource.h>
#include <expat.h>
#include <systemclock.h>
#include <widget/measurespec.h>
#include <fstream>

namespace cdroid {
constexpr int FORWARD = 0;
constexpr int FINISH_HANDLED = 1;
constexpr int FINISH_NOT_HANDLED = 2;

Window::Window(Context*ctx,const AttributeSet&atts):ViewGroup(ctx,atts){
    canvas=nullptr;
    source=new UIEventSource(this);
    setFrame(0,0,1280,720);
    setFocusable(true);
    setKeyboardNavigationCluster(true);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    WindowManager::getInstance().addWindow(this);
}

Window::Window(int x,int y,int width,int height,int type)
  : ViewGroup(width,height),source(nullptr),canvas(nullptr),window_type(type){
    // Set the boundary
    // Do the resizing at first time in order to invoke the OnLayout
    source=new UIEventSource(this);
    mContext=&App::getInstance();
    canvas=nullptr;
    LOGV("%p source=%p visible=%d size=%dx%d",this,source,hasFlag(VISIBLE),width,height);
    setFrame(x, y, width, height);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setFocusable(true);
    setKeyboardNavigationCluster(true);
    WindowManager::getInstance().addWindow(this);
}

void Window::setText(const std::string&txt){
    mText=txt;
}

const std::string Window::getText()const{
    return mText;
}

void Window::invalidate(const RECT*r){
    RECT rc=r?*r:getClientRect();
    mPrivateFlags|=PFLAG_DIRTY;
    mInvalidRgn->do_union((const RectangleInt&)rc);
}

void Window::show(){
    if(getVisibility()!=VISIBLE){
        setVisibility(VISIBLE);
        if(canvas==nullptr)
            invalidate(nullptr);
        WindowManager::getInstance().resetVisibleRegion();
        if(canvas){
            GraphDevice::getInstance().add(canvas);
        }
    }
}

void Window::hide(){
    if(getVisibility()==VISIBLE){
        setVisibility(INVISIBLE);
        GraphDevice::getInstance().remove(canvas);
        WindowManager::getInstance().resetVisibleRegion();
    }
}

View& Window::setPos(int x,int y){
    if(x!=mLeft || y!=mTop){
        //WindowManager::getInstance().resetVisibleRegion();
        WindowManager::getInstance().moveWindow(this,x,y);
        mLeft=x;
        mTop=y;
        if(canvas){
           canvas->set_position(x,y);
        }
    }
    return *this;
}

View& Window::setSize(int cx,int cy){
    if(cx!=getWidth()||cy!=getHeight()){
        onSizeChanged(cx,cy,mWidth,mHeight);
        mWidth=cx;
        mHeight=cy;
        WindowManager::getInstance().resetVisibleRegion();
    }
    return *this;
}
void Window::onFinishInflate(){
    requestLayout(); 
}
Canvas*Window::getCanvas(){
//for children's canvas is allcated by it slef and delete by drawing thread(UIEventSource)
    if((canvas==nullptr)&&(getVisibility()==VISIBLE)){
        canvas=GraphDevice::getInstance().createContext(getBound());
    }
    int num=mInvalidRgn->get_num_rectangles();
    canvas->reset_clip();
    for(int i=0;i<num;i++){
        RectangleInt r=mInvalidRgn->get_rectangle(i);
        canvas->rectangle(r.x,r.y,r.width,r.height);
    }
    canvas->clip();
    return canvas;
}

Window::~Window() {
    LOGV("%p source=%p canvas=%p",this,source,canvas);
    if(canvas!=GraphDevice::getInstance().getPrimaryContext()){
        GraphDevice::getInstance().remove(canvas);
        delete canvas;
    }
}

void Window::onActive(){
    LOGV("%p[%s]",this,getText().c_str());
}

void Window::onDeactive(){
    LOGV("%p[%s]",this,getText().c_str());
}

int Window::processInputEvent(InputEvent&event){
    if(dynamic_cast<KeyEvent*>(&event))
        return processKeyEvent((KeyEvent&)event);
    else return processPointerEvent((MotionEvent&)event);
}

int Window::processPointerEvent(MotionEvent&event){
    return 0;
}

int Window::processKeyEvent(KeyEvent&event){
    int handled = FINISH_NOT_HANDLED;
    int groupNavigationDirection = 0;
    const int action=event.getAction();
    LOGD_IF(action==KeyEvent::ACTION_DOWN,"key:0x%x %s %x",event.getKeyCode(),KeyEvent::getLabel(event.getKeyCode()),KEY_DPAD_DOWN);
    if(dispatchKeyEvent(event))
        return FINISH_HANDLED;
    if (action == KeyEvent::ACTION_DOWN  && event.getKeyCode() == KEY_TAB) {
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(), KeyEvent::META_META_ON)) {
            groupNavigationDirection = View::FOCUS_FORWARD;
        } else if (KeyEvent::metaStateHasModifiers(event.getMetaState(),
              KeyEvent::META_META_ON | KeyEvent::META_SHIFT_ON)) {
            groupNavigationDirection = View::FOCUS_BACKWARD;
        }
    }
    if(action==KeyEvent::ACTION_DOWN){
        if(groupNavigationDirection!=0){
            if(performKeyboardGroupNavigation(groupNavigationDirection))
                return FINISH_HANDLED;
        }else {
            if(performFocusNavigation(event)){
                return FINISH_HANDLED;
            }
        }
    }
    return FORWARD;
}

bool Window::dispatchKeyEvent(KeyEvent&event){
    View* fv =getFocusedChild();
    bool handled=false;
    const int action=event.getAction();
    if(fv && fv->dispatchKeyEvent(event))
        return true;
    int groupNavigationDirection = 0;
    if (action == KeyEvent::ACTION_DOWN  && event.getKeyCode() == KEY_TAB) {
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(), KeyEvent::META_META_ON)) {
            groupNavigationDirection = View::FOCUS_FORWARD;
        } else if (KeyEvent::metaStateHasModifiers(event.getMetaState(),
              KeyEvent::META_META_ON | KeyEvent::META_SHIFT_ON)) {
            groupNavigationDirection = View::FOCUS_BACKWARD;
        }
    }
    if(action==KeyEvent::ACTION_DOWN){
        if(groupNavigationDirection!=0)
            return performKeyboardGroupNavigation(groupNavigationDirection);
        handled=performFocusNavigation(event);
    }
    if(!handled){
        switch(action){
        case KeyEvent::ACTION_UP  : handled = onKeyUp(event.getKeyCode(),event);  break;
        case KeyEvent::ACTION_DOWN: handled = onKeyDown(event.getKeyCode(),event);break;
        default:break;
        }
    }
    return handled;
}

bool Window::performFocusNavigation(KeyEvent& event){
    int direction = -1;
    //从下面代码可以看出，switch语句在此的主要作用是判断焦点的方向
    switch (event.getKeyCode()) {
    case KEY_DPAD_LEFT:
        direction = View::FOCUS_LEFT;
        break;
    case KEY_DPAD_RIGHT:
        direction = View::FOCUS_RIGHT;
        break;
    case KEY_DPAD_UP:
        direction = View::FOCUS_UP;
        break;
    case KEY_DPAD_DOWN:
        direction = View::FOCUS_DOWN;
        break;
    case KEY_TAB:
        if (event.hasNoModifiers()) {
            direction = View::FOCUS_FORWARD;
        } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
            direction = View::FOCUS_BACKWARD;
        }
        break;
    }

    if (direction != -1){
        ViewGroup*mView=(ViewGroup*)this;
        View* focused = mView->findFocus();
	RECT& mTempRect=mRectOfFocusedView;
        if (focused != nullptr) {
            View* v = mView->focusSearch(focused,direction);
            LOGV("mView=%p focused=%p:%d v=%p",mView,focused,focused->getId(),v);
            if (v != nullptr && v != focused) {
                focused->getFocusedRect(mTempRect);
                if (dynamic_cast<ViewGroup*>(mView)) {
                    mView->offsetDescendantRectToMyCoords(focused, mTempRect);
                    mView->offsetRectIntoDescendantCoords(v, mTempRect);
                }
		LOGV("request focus at rect(%d,%d-%d,%d)",mTempRect.x,mTempRect.y,mTempRect.width,mTempRect.height);
                if (v->requestFocus(direction, &mTempRect)) {
                    return true;
                }
            }

            if (mView->dispatchUnhandledMove(focused, direction)) {
                return true;
            }
        } else {
            if (mView->restoreDefaultFocus()) {
                return true;
            }
        }
    }
    return false;
}

bool Window::onKeyDown(int keyCode,KeyEvent& evt){
    Runnable run;
    switch(keyCode){
    case KEY_ESCAPE:
        LOGD("recv %d %s",keyCode,evt.getLabel());
        run=[this](){WindowManager::getInstance().removeWindow(this);};
        post(run);
        return true;
    default:
        //return performFocusNavigation(evt);
        LOGV("recv %d %s",keyCode,evt.getLabel());
        return ViewGroup::onKeyDown(keyCode,evt);
    } 
    return false;
}


void Window::postDelayed(Runnable& what,uint32_t delay){
    if(source)source->post(what,delay);
}

void Window::removeCallbacks(const Runnable& what){
    if(source)source->removeCallbacks(what);
}

void Window::requestLayout(){
    LOGV("requestLayout(%dx%d)child.count=%d HAS_BOUNDS=%x",getWidth(),getHeight(),
                getChildCount(),(mPrivateFlags&PFLAG_HAS_BOUNDS));
    //if( (getChildCount()==1) && dynamic_cast<ViewGroup*>(getChildAt(0)) ){
    int count=mChildren.size();
    for(auto c:mChildren){
	int x=0,y=0;
	int widthSpec,heightSpec;
        ViewGroup*vg=dynamic_cast<ViewGroup*>(c);
	if(vg==nullptr)continue;
	LayoutParams*lp=vg->getLayoutParams();
	LOGV("lp=%p  layoutsize=%d,%d",lp,lp->width,lp->height);

        if(vg->getWidth()>0) widthSpec =MeasureSpec::makeMeasureSpec(vg->getWidth(),MeasureSpec::EXACTLY);
	else if(vg->getWidth()<0)widthSpec=MeasureSpec::makeMeasureSpec(lp->width,MeasureSpec::AT_MOST);
        else widthSpec=MeasureSpec::makeMeasureSpec(mWidth,MeasureSpec::EXACTLY);

        if(vg->getHeight()>0) heightSpec =MeasureSpec::makeMeasureSpec(vg->getHeight(),MeasureSpec::EXACTLY);
	else if(vg->getHeight()<0)heightSpec=MeasureSpec::makeMeasureSpec(lp->height,MeasureSpec::AT_MOST);
        else heightSpec=MeasureSpec::makeMeasureSpec(mHeight,MeasureSpec::EXACTLY);

	if(vg->getHeight()>0&&vg->getWidth()>0){
	    x=vg->getLeft();
	    y=vg->getTop();
	}
        vg->measure(widthSpec, heightSpec);
        vg->layout(x,y,vg->getMeasuredWidth(),vg->getMeasuredHeight());
    }
}

void Window::broadcast(DWORD msgid,DWORD wParam,ULONG lParam){
    WindowManager::getInstance().broadcast(msgid,wParam,lParam);
}

void Window::close(){
    //sendMessage(View::WM_DESTROY,0,0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::function<View*(Context*ctx, const AttributeSet&attrs)>ViewParser;
#define DECLAREPARSER(component) { #component ,[](Context*ctx,const AttributeSet&atts)->View*{return new component(ctx,atts);}} 
static std::map<const std::string,ViewParser>viewParsers={
    DECLAREPARSER(View),
    DECLAREPARSER(TextView),
    DECLAREPARSER(ProgressBar),
    DECLAREPARSER(SeekBar),
    DECLAREPARSER(EditText),
    DECLAREPARSER(ToggleButton),
    DECLAREPARSER(RadioButton),
    DECLAREPARSER(CheckBox),
    DECLAREPARSER(Button),
    DECLAREPARSER(RadioGroup),
    DECLAREPARSER(ImageView),
    DECLAREPARSER(ImageButton),
    DECLAREPARSER(LinearLayout),
    DECLAREPARSER(AbsoluteLayout),
    DECLAREPARSER(FrameLayout),
    DECLAREPARSER(TableLayout),
    DECLAREPARSER(TableRow),
    DECLAREPARSER(Window),
};

typedef struct{
    Context*ctx;
    XML_Parser parser;
    std::string ns;
    std::string nsuri;
    std::vector<View*>views;//the first element is rootviewsetted by inflate
    View*rootView;
    AttributeSet rootAttrs;
}WindowParserData;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    WindowParserData*pd=(WindowParserData*)userData;
    AttributeSet atts;//(satts);
    auto it=viewParsers.find(name);
    for(int i=0;satts[i];i+=2){
        atts.add(satts[i]+pd->nsuri.length()+1,satts[i+1]);
    }
    ViewGroup*parent=nullptr;
    if(pd->views.size())
	parent=dynamic_cast<ViewGroup*>(pd->views.back());
    if(it==viewParsers.end()){
        XML_StopParser(pd->parser,false);
        LOGE("Unknown Parser for %s",name);
        return;
    }

    View*v=it->second(pd->ctx,atts);
    pd->views.push_back(v);
    if(parent){
        LayoutParams*lp=parent->generateLayoutParams(atts);
        LOGV("<%s> layoutSize=%dx%d",name,lp->width,lp->height);
        parent->addView(v,lp);
    }else{
        LayoutParams*lp=((ViewGroup*)v)->generateLayoutParams(atts);
	((ViewGroup*)v)->setLayoutParams(lp);
    }
}

static void endElement(void *userData, const XML_Char *name){
    WindowParserData*pd=(WindowParserData*)userData;
    ViewGroup*p=dynamic_cast<ViewGroup*>(pd->views.back());
    pd->views.pop_back();
    pd->rootView=p;
}

static void NamespaceStartHandler(void *userData,const XML_Char *prefix,const XML_Char *uri){
    LOGD("prefix=%s,uri=%s",prefix,uri);
    WindowParserData*pd=(WindowParserData*)userData;
    pd->ns=prefix;
    pd->nsuri=uri;
}

static void NamespaceiEndHandler (void *userData,const XML_Char *prefix){
    LOGD("prefix=%s",prefix);
}

int Window::inflate(const std::string&res){
    View*v=inflate(getContext(),res);
    LOGD("res=%s v=%p",res.c_str(),v);
    if(v){
        addView(v);
        onFinishInflate();
    }
    return 0;
}

View* Window::inflate(Context*ctx,const std::string&res){
    View*v=nullptr;
    if(ctx){
        std::unique_ptr<std::istream>stream=ctx->getInputStream(res);
        if(stream && stream->good()) v=inflate(ctx,*stream);
    }else{
        std::ifstream fin(res);
        v=inflate(nullptr,fin);
    }
    return v;
}

View*Window::inflate(Context*ctx,std::istream&stream){
    int len=0;
    char buf[256];
    XML_Parser parser=XML_ParserCreateNS(NULL,':');
    WindowParserData pd={ctx,parser};
    ULONGLONG tstart=SystemClock::uptimeMillis();
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetNamespaceDeclHandler(parser,NamespaceStartHandler,NamespaceiEndHandler);
    do {
        stream.read(buf,sizeof(buf));
        len=stream.gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    LOGD("usedtime %dms rootView=%p",SystemClock::uptimeMillis()-tstart,pd.rootView);
    return pd.rootView;
}

}  // namespace ui
