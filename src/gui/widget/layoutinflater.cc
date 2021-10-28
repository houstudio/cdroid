#include <widget/layoutinflater.h>
#include <widget/viewgroup.h>
#include <windows.h>
#include <expat.h>
#include <cdlog.h>
#include <string.h>
#include <fstream>

namespace cdroid{

#define DECLAREPARSER(component) { #component ,[](Context*ctx,const AttributeSet&atts)->View*{return new component(ctx,atts);}}
std::map<const std::string,LayoutInflater::ViewInflater>LayoutInflater::mViewInflaters={
    DECLAREPARSER(View),
    DECLAREPARSER(ViewGroup),
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

LayoutInflater::LayoutInflater(Context*context){
    mContext=context;
}

LayoutInflater*LayoutInflater::from(Context*context){
    return new LayoutInflater(context);
}

LayoutInflater::ViewInflater LayoutInflater::getViewInflater(const std::string&name){
    auto it=mViewInflaters.find(name);
    return (it!=mViewInflaters.end())?it->second:nullptr;
}

View* LayoutInflater::inflate(const std::string&resource,ViewGroup* root, bool attachToRoot){
    View*v=inflate(resource,root);
    if(root && attachToRoot) root->addView(v);
    return v;
}

View* LayoutInflater::inflate(const std::string&resource,ViewGroup*root){
    View*v=nullptr;
    if(mContext){
        std::unique_ptr<std::istream>stream=mContext->getInputStream(resource);
        if(stream && stream->good()) v=inflate(*stream,root);
    }else{
        std::ifstream fin(resource);
        v=inflate(fin,root);
    }
    return v;
}

typedef struct{
    Context*ctx;
    XML_Parser parser;
    std::vector<View*>views;//the first element is rootview setted by inflate
    AttributeSet rootAttrs;
}WindowParserData;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    WindowParserData*pd=(WindowParserData*)userData;
    AttributeSet atts(satts);
    LayoutInflater::ViewInflater inflater=LayoutInflater::getViewInflater(name);
    ViewGroup*parent=nullptr;
    if(pd->views.size())
        parent=dynamic_cast<ViewGroup*>(pd->views.back());
    if(strcmp(name,"merge")==0)return;
    if(inflater==nullptr){
        XML_StopParser(pd->parser,false);
        LOGE("Unknown Parser for %s",name);
        return;
    }

    View*v=inflater(pd->ctx,atts);
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
    if(strcmp(name,"merge")==0)return;
    pd->views.pop_back();
}

View* LayoutInflater::inflate(std::istream&stream,ViewGroup*root){
    int len=0;
    char buf[256];
    XML_Parser parser=XML_ParserCreate(nullptr);
    WindowParserData pd={mContext,parser};
    ULONGLONG tstart=SystemClock::uptimeMillis();
    if(root)
        pd.views.push_back(root);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, startElement, endElement);
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
    LOGD("usedtime %dms rootView=%p",SystemClock::uptimeMillis()-tstart,pd.views.at(0));
    return pd.views.at(0);
}

}//endof namespace
