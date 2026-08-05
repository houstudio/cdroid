#include <navigation/activitynavigator.h>
#include <navigation/navoptions.h>
#include <navigation/navbackstackentry.h>
#include <navigation/navtype.h>
#include <navigation/navargument.h>
#include <core/bundle.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <core/uri.h>
#include <widget/cdwindow.h>
#include <porting/cdlog.h>
#include <regex>

namespace cdroid{

namespace {
// androidx NavInflater.APPLICATION_ID_PLACEHOLDER = "${applicationId}": replace with the package name.
constexpr const char* APPLICATION_ID_PLACEHOLDER = "${applicationId}";
std::string parseApplicationId(Context* context, const std::string& pattern){
    if(pattern.empty()) return pattern;
    std::string out = pattern;
    const std::string pkg = context->getPackageName();
    size_t pos = 0;
    while((pos = out.find(APPLICATION_ID_PLACEHOLDER, pos)) != std::string::npos){
        out.replace(pos, std::char_traits<char>::length(APPLICATION_ID_PLACEHOLDER), pkg);
        pos += pkg.size();
    }
    return out;
}
} // namespace

ActivityNavigator::ActivityNavigator(Context* context, Window* hostActivity)
  : mContext(context), mHostActivity(hostActivity){
    mName = "activity";
}

NavDestination* ActivityNavigator::createDestination(){
    return new Destination(this);
}

void ActivityNavigator::navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Navigator::Extras* extras){
    // androidx ActivityNavigator.navigate returns null: activity destinations do NOT go on the back
    // stack. Run the per-destination startActivity but DON'T push entries onto this navigator's
    // state (the base Navigator::navigate default pushes them — that left a phantom entry causing
    // an extra no-op Back press after returning from the started Window).
    for(NavBackStackEntry* entry : entries){
        if(entry) navigate(entry->getDestination(), entry->getArguments(), navOptions);
    }
    (void)extras;
}

bool ActivityNavigator::popBackStack(){
    // androidx: hostActivity.finish(). CDROID: Window::close(). No host resolved yet → false.
    if(mHostActivity != nullptr){
        mHostActivity->close();
        return true;
    }
    return false;
}

void ActivityNavigator::navigate(NavDestination* destination, Bundle* args, NavOptions* navOptions){
    auto* d = dynamic_cast<Destination*>(destination);
    if(d == nullptr || d->getIntent() == nullptr){
        LOGE("ActivityNavigator: destination %p has no Intent set", (void*)destination);
        return;
    }
    Intent intent(*d->getIntent()); // androidx: Intent(destination.intent)
    if(args != nullptr){
        intent.putExtras(args);
        const std::string& dataPattern = d->getDataPattern();
        if(!dataPattern.empty()){
            // androidx: fill "\\{(.+?)\\}" segments from args (NavType.serializeAsValue, else
            // Uri.encode(value.toString)). CDROID uses the string form + Uri::encode.
            std::regex re("\\{(.+?)\\}");
            std::string data;
            size_t last = 0;
            for(auto it = std::sregex_iterator(dataPattern.begin(), dataPattern.end(), re);
                it != std::sregex_iterator(); ++it){
                const auto& m = *it;
                const size_t matchStart = (size_t)(m[0].first  - dataPattern.begin());
                const size_t matchEnd   = (size_t)(m[0].second - dataPattern.begin());
                data.append(dataPattern, last, matchStart - last);
                const std::string argName = m[1].str();
                // androidx: NavType.serializeAsValue(navType[args, argName]) or Uri.encode fallback.
                std::string valueStr;
                auto ait = d->getArguments().find(argName);
                if(ait != d->getArguments().end() && ait->second){
                    switch(ait->second->getType()){
                    case NavTypeKind::INT: { int v = IntType().get(*args, argName); valueStr = IntType().serializeAsValue(v); break; }
                    case NavTypeKind::LONG: { long v = LongType().get(*args, argName); valueStr = LongType().serializeAsValue(v); break; }
                    case NavTypeKind::FLOAT: { float v = FloatType().get(*args, argName); valueStr = FloatType().serializeAsValue(v); break; }
                    case NavTypeKind::BOOL: { bool v = BoolType().get(*args, argName); valueStr = BoolType().serializeAsValue(v); break; }
                    case NavTypeKind::STRING: { std::string v = StringType().get(*args, argName); valueStr = StringType().serializeAsValue(v); break; }
                    case NavTypeKind::REFERENCE: { int v = ReferenceType().get(*args, argName); valueStr = ReferenceType().serializeAsValue(v); break; }
                    default: valueStr = Uri::encode(args->getString(argName)); break;
                    }
                } else {
                    valueStr = Uri::encode(args->getString(argName));
                }
                data.append(valueStr);
                last = matchEnd;
            }
            data.append(dataPattern, last, std::string::npos);
            intent.setData(new Uri(data)); // CDROID has no Uri::parse; Uri(string) ctor
        }
    }
    // navigatorExtras flags (androidx) would be added here; the legacy navigate signature carries no
    // extras in CDROID — deferred with the entry-based navigate + startActivity wiring.
    if(mHostActivity == nullptr){
        intent.addFlags(Intent::FLAG_ACTIVITY_NEW_TASK); // launching from a non-Activity context
    }
    if(navOptions != nullptr && navOptions->shouldLaunchSingleTop()){
        intent.addFlags(Intent::FLAG_ACTIVITY_SINGLE_TOP);
    }
    if(mHostActivity != nullptr){
        Intent hostIntent = mHostActivity->getIntent();
        const int hostCurrentId = hostIntent.getIntExtra(EXTRA_NAV_CURRENT, 0);
        if(hostCurrentId != 0) intent.putExtra(EXTRA_NAV_SOURCE, hostCurrentId);
    }
    intent.putExtra(EXTRA_NAV_CURRENT, d->getId());
    // androidx: carry popEnter/popExit anim (int resId) when not an animator resource + override the
    // host pending transition. CDROID anim resources are string-keyed and there is no
    // overridePendingTransition, so the animation branches are not ported.
    fprintf(stderr, "[ActivityNav] navigate: className='%s' hostActivity=%p → startActivity\n",
            intent.getComponent().getClassName().c_str(), (void*)mHostActivity);
    mContext->startActivity(intent); // CDROID seam — App::startActivity resolves via ActivityFactory
    LOGD("ActivityNavigator.navigate: component=%s dataPattern=%zu (startActivity is a no-op seam)",
         intent.getComponent().getClassName().c_str(), d->getDataPattern().size());
}

void ActivityNavigator::applyPopAnimationsToPendingTransition(Window* /*activity*/){
    // androidx: read EXTRA_POP_ENTER/EXIT_ANIM from activity.getIntent() and overridePendingTransition.
    // CDROID has no activity pending-transition override yet — no-op stub.
}

bool ActivityNavigator::Destination::equals(const Destination& other) const {
    // androidx: super.equals (identity) + intent.filterEquals + dataPattern.
    if(this == &other) return true;
    Intent* ti = getIntent();
    Intent* oi = other.getIntent();
    return ((ti && oi && ti->filterEquals(oi)) || (!ti && !oi))
        && getDataPattern() == other.getDataPattern();
}

int ActivityNavigator::Destination::hashCode() const {
    int h = 0;
    Intent* intent = getIntent();
    if(intent) h = 31 * h + intent->filterHashCode();
    std::hash<std::string> hasher;
    h = 31 * h + (int)hasher(getDataPattern());
    return h;
}

void ActivityNavigator::Destination::onInflate(Context* context, const AttributeSet& attrs){
    NavDestination::onInflate(context, attrs);
    LOGD("ActivityDest.onInflate name=%s targetPkg=%s action=%s", attrs.getString("name").c_str(), attrs.getString("targetPackage").c_str(), attrs.getString("action").c_str());
    // androidx R.styleable.ActivityNavigator: targetPackage / android:name / action / data / dataPattern.
    setTargetPackage(parseApplicationId(context, attrs.getString("targetPackage")));
    std::string className = attrs.getString("name");
    if(!className.empty()){
        if(className[0] == '.') className = context->getPackageName() + className;
        setComponentName(ComponentName(context->getPackageName(), className));
    }
    setAction(attrs.getString("action"));
    const std::string data = parseApplicationId(context, attrs.getString("data"));
    if(!data.empty()) setData(new Uri(data));
    setDataPattern(parseApplicationId(context, attrs.getString("dataPattern")));
}

}//namespace
