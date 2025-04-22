#include <core/xmlpullparser.h>
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

NavGraph* NavInflater::inflate(const std::string& graphResId) {
    XmlPullParser parser(mContext,graphResId);
    AttributeSet& attrs = parser;
    int type;
    while ((type = parser.next()) != XmlPullParser::START_TAG
            && type != XmlPullParser::END_DOCUMENT) {
        // Empty loop
    }
    if (type != XmlPullParser::START_TAG) {
        throw std::runtime_error("No start tag found");
    }

    std::string rootElement = parser.getName();
    NavDestination* destination = inflate(parser, attrs);
    if (dynamic_cast<NavGraph*>(destination)==nullptr) {
        throw ("Root element <" + rootElement + ">" + " did not inflate into a NavGraph");
    }
    return (NavGraph*) destination;
}

NavDestination* NavInflater::inflate(XmlPullParser&parser,const AttributeSet& attrs){
    Navigator* navigator = mNavigatorProvider->getNavigator(parser.getName());
    NavDestination* dest = navigator->createDestination();

    dest->onInflate(mContext, attrs);

    const int innerDepth =parser.getDepth() + 1;
    int type, depth;
    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
            && ((depth = parser.getDepth()) >= innerDepth
            || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if (depth > innerDepth) {
            continue;
        }

        const std::string name = parser.getName();
        if (name.compare("argument")==0) {
            inflateArgument(*dest, attrs);
        } else if (name.compare("deepLink")==0) {
            inflateDeepLink(*dest, attrs);
        } else if (name.compare("action")==0) {
            inflateAction(*dest, attrs);
        } else if ((name.compare("include")==0) && dynamic_cast<NavGraph*>(dest)) {
            const std::string id = attrs.getString("graph");
            ((NavGraph*) dest)->addDestination(inflate(id));
        } else if (dynamic_cast<NavGraph*>(dest)) {
            ((NavGraph*)dest)->addDestination(inflate(parser, attrs));
        }
    }

    return dest;
}

void NavInflater::inflateArgument(NavDestination& dest,const AttributeSet& attrs){
    const std::string name = attrs.getString("name");
    const std::string argType = attrs.getString("argType");
    const std::string defValue= attrs.getString("defaultValue");

    /*if (a.getValue(R.styleable.NavArgument_android_defaultValue, value)) {
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
        throw std::runtime_error("Every <deepLink> must include an app:uri");
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
