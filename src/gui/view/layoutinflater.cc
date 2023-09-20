#include <view/layoutinflater.h>
#include <view/viewgroup.h>
#include <cdroid.h>
#include <expat.h>
#include <cdlog.h>
#include <string.h>
#include <fstream>
#include <iomanip>

namespace cdroid {

LayoutInflater::LayoutInflater(Context*context) {
    mContext=context;
}

LayoutInflater*LayoutInflater::from(Context*context) {
    static std::map<Context*,LayoutInflater*>mMaps;
    auto it = mMaps.find(context);
    if(it == mMaps.end()) {
        LayoutInflater*flater = new LayoutInflater(context);
        it = mMaps.insert(std::pair<Context*,LayoutInflater*>(context,flater)).first;
        if( mMaps.size() ==1) {
            atexit([]() {
                INFLATERMAPPER& fmap = LayoutInflater::getInflaterMap();
                STYLEMAPPER& smap = getStyleMap();
                fmap.clear();
                smap.clear();
                for(auto m:mMaps)
                    delete m.second;
                mMaps.clear();
                std::cout<<"LayoutInflater::Destroied!"<<std::endl;
            });
        }
    }
    return it->second;
}

LayoutInflater::INFLATERMAPPER& LayoutInflater::getInflaterMap() {
    static LayoutInflater::INFLATERMAPPER mFlateMapper;
    return mFlateMapper;
}

LayoutInflater::STYLEMAPPER& LayoutInflater::getStyleMap() {
    static LayoutInflater::STYLEMAPPER mDefaultStyle;
    return mDefaultStyle;
}

const std::string LayoutInflater::getDefaultStyle(const std::string&name)const {
    LayoutInflater::STYLEMAPPER& maps = getStyleMap();
    auto it = maps.find(name);
    return it==maps.end()?std::string():it->second;
}

LayoutInflater::ViewInflater LayoutInflater::getInflater(const std::string&name) {
    const size_t  pt = name.rfind('.');
    LayoutInflater::INFLATERMAPPER &maps =getInflaterMap();
    const std::string sname = (pt!=std::string::npos)?name.substr(pt+1):name;
    auto it = maps.find(sname);
    return (it!=maps.end())?it->second:nullptr;
}

bool LayoutInflater::registe(const std::string&name,const std::string&defstyle,LayoutInflater::ViewInflater inflater) {
    LayoutInflater::INFLATERMAPPER& maps = getInflaterMap();
    if(maps.find(name) != maps.end() )
        return false;
    maps.insert(INFLATERMAPPER::value_type(name,inflater));
    getStyleMap().insert(std::pair<const std::string,const std::string>(name,defstyle));
    return true;
}

View* LayoutInflater::inflate(const std::string&resource,ViewGroup*root,bool attachToRoot,AttributeSet*atts) {
    View*v=nullptr;
    if(mContext) {
        std::string package;
        std::unique_ptr<std::istream>stream = mContext->getInputStream(resource,&package);
        LOGV("inflate from %s",resource.c_str());
        if(stream && stream->good()) {
            v = inflate(package,*stream,root,attachToRoot && (root!=nullptr),atts);
        } else {
            LOGE("faild to load resource %s [cdroid.pak %s.pak] must be copied to your work directory",resource.c_str(),package.c_str());
        }
    } else {
        std::ifstream fin(resource);
        v=inflate(resource,fin,root,root!=nullptr,nullptr);
    }
    return v;
}

typedef struct {
    Context*ctx;
    std::string package;
    XML_Parser parser;
    bool attachToRoot;
    ViewGroup* root;
    std::vector<View*>views;//the first element is rootview setted by inflate
    std::vector<int>flags;
    AttributeSet* atts;
    View*returnedView;
    int parsedView;
} WindowParserData;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts) {
    WindowParserData*pd = (WindowParserData*)userData;
    AttributeSet atts(pd->ctx,pd->package);
    LayoutInflater::ViewInflater inflater = LayoutInflater::getInflater(name);
    ViewGroup*parent = pd->root;
    atts.set(satts);
    if(pd->views.size())
        parent = dynamic_cast<ViewGroup*>(pd->views.back());
    else if(pd->atts) {
        pd->atts->inherit(atts);
    }
    if(strcmp(name,"merge")==0) {
        pd->views.push_back(parent);
        pd->flags.push_back(1);
        if(pd->root == nullptr|| !pd->attachToRoot)
            FATAL("<merge /> can be used only with a valid ViewGroup root(%p) and attachToRoot=true",pd->root);
        return ;
    }

    if(strcmp(name,"include")==0) {
        /**the included layout's root node maybe merge,so we must use attachToRoot =true*/
        const std::string layout = atts.getString("layout");
        View* includedView = LayoutInflater::from(pd->ctx)->inflate(layout,parent,true,&atts);
        if(includedView) { //for merge as rootnode,the includedView will be null.
            LayoutParams*lp = parent->generateLayoutParams(atts);
            includedView->setLayoutParams(lp);
        }
        return;
    }

    if( inflater == nullptr ) {
        pd->views.push_back(nullptr);
        LOGE("Unknown Parser for %s",name);
        return;
    }

    std::string stname = atts.getString("style");
    if(!stname.empty()) {
        AttributeSet style = pd->ctx->obtainStyledAttributes(stname);
        atts.inherit(style);
    }
    stname = LayoutInflater::from(pd->ctx)->getDefaultStyle(name);
    if(!stname.empty()) {
        AttributeSet defstyle = pd->ctx->obtainStyledAttributes(stname);
        atts.inherit(defstyle);
    }

    View*v = inflater(pd->ctx,atts);
    pd->parsedView++;
    pd->flags.push_back(0);
    pd->views.push_back(v);
    LOG(VERBOSE)<<std::setw(pd->views.size()*8)<<v<<":"<<v->getId()<<"["<<name<<"]"<<stname;
    if( ( parent && ( parent != pd->root )) || ( pd->attachToRoot && pd->root ) ) {
        LayoutParams*lp = parent->generateLayoutParams(atts);
        parent->addView(v,lp);
    } else if (dynamic_cast<ViewGroup*>(v)) {
        LayoutParams*lp = ((ViewGroup*)v)->generateLayoutParams(atts);
        ((ViewGroup*)v)->setLayoutParams(lp);
    }
}

static void endElement(void *userData, const XML_Char *name) {
    WindowParserData*pd = (WindowParserData*)userData;
    if(strcmp(name,"include")==0) return;

    if((pd->views.size()==1) && (pd->flags.back()==0))
        pd->returnedView = pd->views.back();
    pd->flags.pop_back();
    pd->views.pop_back();
}

View* LayoutInflater::inflate(const std::string&package,std::istream&stream,ViewGroup*root,bool attachToRoot,AttributeSet*atts) {
    int len = 0;
    char buf[256];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    WindowParserData pd;
    ULONGLONG tstart = SystemClock::uptimeMillis();

    pd.ctx  = mContext;
    pd.root = root;
    pd.package=package;
    pd.parsedView  = 0;
    pd.returnedView= nullptr;
    pd.atts = atts;
    pd.attachToRoot= attachToRoot;
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, startElement, endElement);
    do {
        stream.read(buf,sizeof(buf));
        len = stream.gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es = XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while( len != 0 );
    XML_ParserFree(parser);
    if(root && attachToRoot) {
        //if(pd.returnedView)root->addView(pd.returnedView);already added in startElementd
        root->requestLayout();
        root->startLayoutAnimation();
    }
    LOGV("usedtime %dms parsed %d views",SystemClock::uptimeMillis() - tstart, pd.parsedView);
    return pd.returnedView;
}

}//endof namespace
