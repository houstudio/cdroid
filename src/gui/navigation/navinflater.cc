#if 10
#include <expat.h>
#include <navigation/navaction.h>
#include <navigation/navgraph.h>
#include <navigation/navinflater.h>
namespace cdroid{
//frameworks/support/navigation/runtime/src/main/java/androidx/navigation/NavInflater.java
//private static final String APPLICATION_ID_PLACEHOLDER = "${applicationId}";

NavInflater::NavInflater(Context* context, NavigatorProvider* navigatorProvider){
    mContext = context;
    mNavigatorProvider = navigatorProvider;
}

NavGraph* NavInflater::inflateMetadataGraph() {
    /*Bundle metaData = mContext->getApplicationInfo().metaData;
    if (metaData != nullptr) {
        const int resid = metaData.getInt(METADATA_KEY_GRAPH);
        if (resid != 0) {
            return inflate(resid);
        }
    }*/
    return nullptr;
}

class NavInflater::NaviParser{
public:
    NavInflater*mInflater;
    std::string package;
    Context*mContext;
    Navigator*navigator;
    NavDestination*navdst;
public:
    NaviParser(Context*ctx,NavInflater*flater):mContext(ctx),mInflater(flater),navdst(nullptr){
    }
    static void startElement(void *userData, const XML_Char *name, const XML_Char **satts);
    static void endElement(void *userData, const XML_Char *name);
};

void  NavInflater::NaviParser::startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    NaviParser*nvdat=(NaviParser*)userData;
    AttributeSet atts(nvdat->mContext,nvdat->package);
    atts.set(satts);
    if(strcmp(name,"argument")==0){
    }else if(strcmp(name,"deepLink")==0){
    }else if(strcmp(name,"action")==0){
    }else if(strcmp(name,"include")==0){
    }else if(strcmp(name,"fragment")==0){
    }else if(strcmp(name,"navigation")==0){
        nvdat->navdst = nvdat->mInflater->inflate(name,atts);
    }
}

void  NavInflater::NaviParser::endElement(void *userData, const XML_Char *name){
}

NavGraph* NavInflater::inflate(const std::string& graphResId) {
    int len = 0;
    std::string package;
    NaviParser pd(mContext,this);
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, NaviParser::startElement, NaviParser::endElement);
    std::unique_ptr<std::istream>stream = mContext->getInputStream(graphResId,&package);
    do {
        char buf[256];
        stream->read(buf,sizeof(buf));
        len = stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es = XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while( len != 0 );
    return nullptr;
}

NavDestination* NavInflater::inflate(const std::string&name,const AttributeSet& attrs){
    Navigator* navigator = mNavigatorProvider->getNavigator(name);//parser.getName());
    NavDestination* dest = navigator->createDestination();

    dest->onInflate(mContext, attrs);

    const int innerDepth =0;// parser.getDepth() + 1;
    int type;
    int depth;
    /*while ((type = parser.next()) != XmlPullParser.END_DOCUMENT
            && ((depth = parser.getDepth()) >= innerDepth
            || type != XmlPullParser.END_TAG)) {
        if (type != XmlPullParser.START_TAG) {
            continue;
        }

        if (depth > innerDepth) {
            continue;
        }

        const std::string name = parser.getName();
        if (TAG_ARGUMENT.equals(name)) {
            inflateArgument(res, dest, attrs);
        } else if (TAG_DEEP_LINK.equals(name)) {
            inflateDeepLink(res, dest, attrs);
        } else if (TAG_ACTION.equals(name)) {
            inflateAction(res, dest, attrs);
        } else if (TAG_INCLUDE.equals(name) && dynamic_cast<NavGraph*>(dest)) {
            TypedArray a = res.obtainAttributes(attrs, R.styleable.NavInclude);
            const int id = a.getResourceId(R.styleable.NavInclude_graph, 0);
            ((NavGraph*) dest)->addDestination(inflate(id));
            a.recycle();
        } else if (dynamic_cast<NavGraph*>(dest)) {
            ((NavGraph*)dest)->addDestination(inflate(res, parser, attrs));
        }
    }*/

    return dest;
}

void NavInflater::inflateArgument(NavDestination& dest,const AttributeSet& attrs){
    /*TypedArray a = res.obtainAttributes(attrs, R.styleable.NavArgument);
    std::string name = attrs.getString("name");//a.getString(R.styleable.NavArgument_android_name);

    TypedValue value = sTmpValue.get();
    if (value == nullptr) {
        value = new TypedValue();
        sTmpValue.set(value);
    }
    if (a.getValue(R.styleable.NavArgument_android_defaultValue, value)) {
        switch (value.type) {
        case TypedValue.TYPE_STRING:
            dest.getDefaultArguments().putString(name, value.string.toString());
            break;
        case TypedValue.TYPE_DIMENSION:
            dest.getDefaultArguments().putInt(name,
                    (int) value.getDimension(res.getDisplayMetrics()));
            break;
        case TypedValue.TYPE_FLOAT:
            dest.getDefaultArguments().putFloat(name, value.getFloat());
            break;
        case TypedValue.TYPE_REFERENCE:
            dest.getDefaultArguments().putInt(name, value.data);
            break;
        default:
            if (value.type >= TypedValue.TYPE_FIRST_INT
                    && value.type <= TypedValue.TYPE_LAST_INT) {
                dest.getDefaultArguments().putInt(name, value.data);
            } else {
                //throw ("unsupported argument type " + value.type);
            }
        }
    }*/
}

void NavInflater::inflateDeepLink(NavDestination& dest, const AttributeSet& attrs) {
    //TypedArray a = res.obtainAttributes(attrs, R.styleable.NavDeepLink);
    std::string uri = attrs.getString("uri");//R.styleable.NavDeepLink_uri);
    if (uri.empty()){//
        //throw new IllegalArgumentException("Every <" + TAG_DEEP_LINK
        //        + "> must include an app:uri");
    }
    //uri = uri.replace(APPLICATION_ID_PLACEHOLDER, mContext.getPackageName());
    dest.addDeepLink(uri);
}

void NavInflater::inflateAction(NavDestination& dest,const AttributeSet& attrs) {
    //TypedArray a = res.obtainAttributes(attrs, R.styleable.NavAction);
    const int id = attrs.getResourceId("id");//R.styleable.NavAction_android_id, 0);
    const int destId = attrs.getResourceId("destination");//R.styleable.NavAction_destination, 0);
    NavAction* action = new NavAction(destId);
#if 0
    NavOptions::Builder builder;// = new NavOptions.Builder();
    builder.setLaunchSingleTop(a.getBoolean(R.styleable.NavAction_launchSingleTop, false));
    builder.setLaunchDocument(a.getBoolean(R.styleable.NavAction_launchDocument, false));
    builder.setClearTask(a.getBoolean(R.styleable.NavAction_clearTask, false));
    builder.setPopUpTo(a.getResourceId(R.styleable.NavAction_popUpTo, 0),
            a.getBoolean(R.styleable.NavAction_popUpToInclusive, false));
    builder.setEnterAnim(a.getResourceId(R.styleable.NavAction_enterAnim, -1));
    builder.setExitAnim(a.getResourceId(R.styleable.NavAction_exitAnim, -1));
    builder.setPopEnterAnim(a.getResourceId(R.styleable.NavAction_popEnterAnim, -1));
    builder.setPopExitAnim(a.getResourceId(R.styleable.NavAction_popExitAnim, -1));
    action->setNavOptions(builder.build());
#endif
    dest.putAction(id, action);
}
}/*endof namespace*/
#endif
