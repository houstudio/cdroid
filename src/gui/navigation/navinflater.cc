#include <core/xmlpullparser.h>
#include <navigation/navaction.h>
#include <navigation/navgraph.h>
#include <navigation/navinflater.h>
#include <navigation/navargument.h>
#include <navigation/navoptions.h>
#include <navigation/navtype.h>
#include <porting/cdlog.h>
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
    LOGD("NavInflater.inflate: tag='%s'", parser.getName().c_str());
    Navigator* navigator = mNavigatorProvider->getNavigator(parser.getName());
    if(!navigator){ LOGD("NavInflater: no navigator for tag '%s'", parser.getName().c_str()); return nullptr; }
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
        LOGD("NavInflater.inflate: child tag='%s'", name.c_str());
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
    const std::string defValue = attrs.getString("defaultValue");
    const bool nullable = (attrs.getString("nullable") == "true");
    NavTypeKind kind = argType.empty() ? NavTypeKind::STRING : navTypeKindFromName(argType);
    NavArgument::Builder builder;
    builder.setType(kind);
    builder.setIsNullable(nullable);
    if(!defValue.empty()){
        try{
            switch(kind){
                case NavTypeKind::INT:    builder.setDefaultValue(IntType().parseValue(defValue)); break;
                case NavTypeKind::LONG:   builder.setDefaultValue(LongType().parseValue(defValue)); break;
                case NavTypeKind::FLOAT:  builder.setDefaultValue(std::stof(defValue)); break;
                case NavTypeKind::BOOL:   builder.setDefaultValue(defValue == "true"); break;
                // reference default is an @-resource ref / 0x / literal -> int. attrs.getInt uses
                // the raw attribute value and resolves @-refs (via Context) + 0x + decimals, which
                // is the closest CDROID analogue to androidx resolving @ refs in inflate (CDROID
                // has no unified int-resId system, so @string resId is a known limitation).
                case NavTypeKind::REFERENCE: builder.setDefaultValue(attrs.getInt("defaultValue", 0)); break;
                default:                  builder.setDefaultValue(defValue); break;
            }
        }catch(...){
            builder.setDefaultValue(defValue);
        }
    }
    dest.addArgument(name, builder.build());
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
    // Mirrors androidx NavInflater.inflateAction: action + destination are int ids; popUpTo is
    // an int destination id (-1 = none). Anim is kept as a resource name here (androidx uses an
    // int res id) because CDROID's animation pipeline resolves by name.
    const int id = attrs.getResourceId("id", 0);
    const int destId = attrs.getResourceId("destination", 0);
    NavAction* action = new NavAction(destId);
    NavOptions::Builder builder;
    builder.setLaunchSingleTop(attrs.getBoolean("launchSingleTop", false));
    builder.setRestoreState(attrs.getBoolean("restoreState", false));
    builder.setPopUpTo(attrs.getResourceId("popUpTo", -1),
            attrs.getBoolean("popUpToInclusive", false),
            attrs.getBoolean("popUpToSaveState", false));
    builder.setEnterAnim(attrs.getString("enterAnim"));
    builder.setExitAnim(attrs.getString("exitAnim"));
    builder.setPopEnterAnim(attrs.getString("popEnterAnim"));
    builder.setPopExitAnim(attrs.getString("popExitAnim"));
    action->setNavOptions(builder.build());
    // TODO: nested <argument> children should populate action defaultArguments (needs SavedState
    // merge); not required for popUpTo/singleTop, deferred.
    dest.putAction(id, action);
}
}/*endof namespace*/
