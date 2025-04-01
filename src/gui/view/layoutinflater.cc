#include <core/atexit.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <porting/cdlog.h>
#include <expat.h>
#include <string.h>
#include <fstream>
#include <iomanip>

#if defined(__linux__)||defined(__unix__)
  #include <unistd.h>
#elif defined(_WIN32)||defined(_WIN64)
  #include <direct.h>
  #ifndef PATH_MAX
  #define PATH_MAX _MAX_PATH
  #endif
#endif

namespace cdroid {

LayoutInflater::LayoutInflater(Context*context) {
    mContext = context;
}

LayoutInflater*LayoutInflater::mInst = nullptr;
LayoutInflater*LayoutInflater::from(Context*context) {
    if(mInst==nullptr){
        mInst = new LayoutInflater(context);
        AtExit::registerCallback([](){
            LOGD("delete LayoutInflater %p",mInst);
            delete mInst;
        });
    }
    return mInst;
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

bool LayoutInflater::registerInflater(const std::string&name,const std::string&defstyle,LayoutInflater::ViewInflater inflater) {
    LayoutInflater::INFLATERMAPPER& maps = getInflaterMap();
    LayoutInflater::STYLEMAPPER& smap = getStyleMap();
    auto flaterIter = maps.find(name);
    auto styleIter = smap.find(name);
#if 1
    /*disable widget inflater's hack*/
    if(flaterIter!=maps.end() ){
        LOGW("%s is registed to %p",name.c_str(),flaterIter->second);
        return false;
    }
    maps.insert(INFLATERMAPPER::value_type(name,inflater));
    smap.insert(std::pair<const std::string,const std::string>(name,defstyle));
#else
    if(flaterIter!=maps.end() ){
        flaterIter->second = inflater;
        LOGI("%s is hacked to %p",name.c_str(),flaterIter->second);
    }else{
        maps.insert(INFLATERMAPPER::value_type(name,inflater));
    }
    if(styleIter!=smap.end())
        styleIter->second = defstyle;
    else
        smap.insert(std::pair<const std::string,const std::string>(name,defstyle));
#endif
    return true;
}
#ifndef NEW_LAYOUT_INFLATER
View* LayoutInflater::inflate(const std::string&resource,ViewGroup*root,bool attachToRoot,AttributeSet*atts) {
    View*v = nullptr;
    const int64_t tstart = SystemClock::uptimeMillis();
    if(mContext) {
        std::string package;
        std::unique_ptr<std::istream>stream = mContext->getInputStream(resource,&package);
        LOGV("inflate from %s",resource.c_str());
        if(stream && stream->good()) {
            v = inflate(package,*stream,root,attachToRoot && (root!=nullptr),atts);
        } else {
            char cwdpath[PATH_MAX]="your work directory";
            char* result = getcwd(cwdpath,PATH_MAX);
            LOGE("faild to load resource %s [%s.pak] must be copied to [%s]",resource.c_str(),package.c_str(),result);
        }
    } else {
        std::ifstream fin(resource);
        v=inflate(resource,fin,root,root!=nullptr,nullptr);
    }
    const long tmused = long(SystemClock::uptimeMillis() - tstart);
    LOGI_IF(tmused>=8,"%s inflater used %ldms",resource.c_str(),tmused);
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
            includedView->setId(pd->ctx->getId(atts.getString("id")));
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
        v->setLayoutParams(lp);
    } else if(pd->root){
        LayoutParams*lp = pd->root->generateLayoutParams(atts);
        v->setLayoutParams(lp);
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
    size_t len = 0;
    char buf[256];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    WindowParserData pd;

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
    return pd.returnedView;
}
#else
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View* LayoutInflater::inflate(const std::string&resource,ViewGroup* root, bool attachToRoot,AttributeSet*atts){
    int type,depth;
    XmlPullParser parser(mContext,resource);
    View*result = root;
    XmlPullParser::XmlEvent event;
    const int innerDepth = parser.getDepth();
    while(((type=parser.next(event))!=XmlPullParser::START_TAG)
            &&(type!=XmlPullParser::END_DOCUMENT)){
        //Empty
    }

    if(type!=XmlPullParser::START_TAG)throw std::logic_error("No start tag found");
    const std::string name = parser.getName();
    if(name.compare(TAG_MERGE)==0){
        rInflate(parser,root,mContext,event.attributes,false);
    }else{
        View*temp = createViewFromTag(root,name,mContext,event.attributes,false);
        ViewGroup::LayoutParams*params = nullptr;
        if(root!=nullptr){
            params =root->generateLayoutParams(event.attributes);
            if(!attachToRoot)temp->setLayoutParams(params);
        }
        rInflateChildren(parser,temp,event.attributes,true);
        if((root!=nullptr)&&attachToRoot) root->addView(temp,params);
        if((root==nullptr)||(attachToRoot==false)) result = temp;
    }
    return result;
}
#endif

View* LayoutInflater::createView(const std::string& name, const std::string& prefix,const AttributeSet& attrs){
    return nullptr;
}

View* LayoutInflater::createViewFromTag(View* parent,const std::string& name, Context* context,const AttributeSet& attrs,bool ignoreThemeAttr) {
#if 0
    if (name.compare("view")==0) {
        //name = attrs.getAttributeValue(nullptr, "class");
    }

    // Apply a theme wrapper, if allowed and one is specified.

    View* view;
    if (mFactory2 != null) {
        view = mFactory2.onCreateView(parent, name, context, attrs);
    } else if (mFactory != null) {
        view = mFactory.onCreateView(name, context, attrs);
    } else {
        view = null;
    }

    if (view == null && mPrivateFactory != null) {
        view = mPrivateFactory.onCreateView(parent, name, context, attrs);
    }

    if (view == null) {
        final Object lastContext = mConstructorArgs[0];
        mConstructorArgs[0] = context;
        try {
            if (-1 == name.indexOf('.')) {
                view = onCreateView(parent, name, attrs);
            } else {
                view = createView(name, null, attrs);
            }
        } finally {
            mConstructorArgs[0] = lastContext;
        }
    }
    return view;
#else
   std::string styleName = attrs.getString("style");
   AttributeSet temp(attrs);
   LayoutInflater::ViewInflater inflater = LayoutInflater::getInflater(name);
    if(!styleName.empty()) {
        AttributeSet style = context->obtainStyledAttributes(styleName);
        temp.inherit(style);
    }
    styleName = LayoutInflater::from(context)->getDefaultStyle(name);
    if(!styleName.empty()) {
        AttributeSet defstyle = context->obtainStyledAttributes(styleName);
        temp.inherit(defstyle);
    }
    View*view = inflater(context,temp);
    return view;
#endif
}

void LayoutInflater::rInflateChildren(XmlPullParser& parser, View* parent,const AttributeSet& attrs,bool finishInflate){
    rInflate(parser, parent, parent->getContext(), attrs, finishInflate);
}

void LayoutInflater::rInflate(XmlPullParser& parser, View* parent, Context* context,const AttributeSet& attrs, bool finishInflate){
    const int depth = parser.getDepth();
    int type;
    XmlPullParser::XmlEvent event;
    bool pendingRequestFocus = false;

    while (((type = parser.next(event)) != XmlPullParser::END_TAG ||
            parser.getDepth() > depth) && (type != XmlPullParser::END_DOCUMENT)) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        AttributeSet& a = event.attributes;
        if (name.compare(TAG_REQUEST_FOCUS)==0) {
            pendingRequestFocus = true;
            consumeChildElements(parser);
        } else if (name.compare(TAG_TAG)==0) {
            parseViewTag(parser, parent, a);
        } else if (name.compare(TAG_INCLUDE)==0) {
            if (parser.getDepth() == 0) {
                throw std::logic_error("<include /> cannot be the root element");
            }
            parseInclude(parser, context, parent, a);
        } else if (name.compare(TAG_MERGE)==0) {
            throw std::logic_error("<merge /> must be the root element");
        } else {
            View* view = createViewFromTag(parent, name, context, a,false);
            ViewGroup* viewGroup = (ViewGroup*) parent;
            ViewGroup::LayoutParams* params = viewGroup->generateLayoutParams(a);
            rInflateChildren(parser, view, a, true);
            viewGroup->addView(view, params);
        }
    }

    if (pendingRequestFocus) {
        parent->restoreDefaultFocus();
    }

    if (finishInflate) {
        //parent->onFinishInflate();
    }
}

void LayoutInflater::consumeChildElements(XmlPullParser& parser){
    int type;
    XmlPullParser::XmlEvent event;
    const int currentDepth = parser.getDepth();
    while (((type = parser.next(event)) != XmlPullParser::END_TAG ||
            parser.getDepth() > currentDepth) && type != XmlPullParser::END_DOCUMENT) {
        // Empty
    }
}

void LayoutInflater::parseViewTag(XmlPullParser& parser, View* view,const AttributeSet& attrs){
#if 0
    Context* context = view.getContext();
    final TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.ViewTag);
    final int key = ta.getResourceId(R.styleable.ViewTag_id, 0);
    final CharSequence value = ta.getText(R.styleable.ViewTag_value);
    view.setTag(key, value);
    consumeChildElements(parser);
#endif
}

void LayoutInflater::parseInclude(XmlPullParser& parser, Context* context, View* parent,const AttributeSet& attrs){
    int type;
    if (dynamic_cast<ViewGroup*>(parent)) {
        // Apply a theme wrapper, if requested. This is sort of a weird
        // edge case, since developers think the <include> overwrites
        // values in the AttributeSet of the included View. So, if the
        // included View has a theme attribute, we'll need to ignore it.

        // If the layout is pointing to a theme attribute, we have to
        // massage the value to get a resource identifier out of it.
        const bool hasThemeOverride = false;
        std::string layout = attrs.getString("layout");
        if (layout.empty()) {
            throw std::logic_error("You must specify a layout in the include tag: <include layout=\"@layout/layoutID\" />");
            // Attempt to resolve the "?attr/name" string to an attribute
            // within the default (e.g. application) package.
            //layout = context.getResources().getIdentifier(value.substring(1), "attr", context.getPackageName());
        }

        XmlPullParser childParser(context,layout);
        XmlPullParser::XmlEvent event;

        while ((type = childParser.next(event)) != XmlPullParser::START_TAG &&
                type != XmlPullParser::END_DOCUMENT) {
            // Empty.
        }

        if (type != XmlPullParser::START_TAG) {
            throw std::logic_error(childParser.getPositionDescription()+": No start tag found!");
        }

        const std::string childName = childParser.getName();
        AttributeSet& childAttrs = event.attributes;

        if (childName.compare(TAG_MERGE)==0){
            // The <merge> tag doesn't support android:theme, so nothing special to do here.
            rInflate(childParser, parent, context, childAttrs, false);
        } else {
            View* view = createViewFromTag(parent, childName,context, childAttrs, hasThemeOverride);
            ViewGroup* group = (ViewGroup*) parent;

            const int id = attrs.getResourceId("id", View::NO_ID);
            const int visibility = attrs.getInt("visibility", -1);

            // We try to load the layout params set in the <include /> tag.
            // If the parent can't generate layout params (ex. missing width
            // or height for the framework ViewGroups, though this is not
            // necessarily true of all ViewGroups) then we expect it to throw
            // a runtime exception.
            // We catch this exception and set localParams accordingly: true
            // means we successfully loaded layout params from the <include>
            // tag, false means we need to rely on the included layout params.
            ViewGroup::LayoutParams* params = group->generateLayoutParams(attrs);
            if (params == nullptr) {
                params = group->generateLayoutParams(childAttrs);
            }
            view->setLayoutParams(params);

            // Inflate all children.
            rInflateChildren(childParser, view, childAttrs, true);

            if (id != View::NO_ID) {
                view->setId(id);
            }

            switch (visibility) {
            case 0: view->setVisibility(View::VISIBLE);   break;
            case 1: view->setVisibility(View::INVISIBLE); break;
            case 2: view->setVisibility(View::GONE);      break;
            }
            group->addView(view);
        }
    } else {
        throw std::logic_error("<include /> can only be used inside of a ViewGroup");
    }
    LayoutInflater::consumeChildElements(parser);
}

}//endof namespace
