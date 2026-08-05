#ifndef __ACTIVITYNAVIGATOR_H__
#define __ACTIVITYNAVIGATOR_H__
/*********************************************************************************
 * Port of androidx.navigation.ActivityNavigator — cross-Activity navigation.
 *
 * CDROID maps Activity to Window (typedef Window Activity). navigate() builds an Intent from the
 * destination (component / action / data / args) and calls mContext->startActivity(intent), which is
 * a no-op Context seam until the className->Window-factory + launch wiring is added (deferred — see
 * plan). popBackStack() finishes the host Window. Activity destinations do NOT go on the NavController
 * back stack (androidx returns null), so navigate here performs the side-effect only.
 *********************************************************************************/
#include <navigation/navigator.h>
#include <navigation/navdestination.h>
#include <core/intent.h>
#include <string>
namespace cdroid{
class Context;
class Window;
class AttributeSet;
class NavOptions;
class Bundle;

class ActivityNavigator : public Navigator{
public:
    class Destination;
    class Extras;

    explicit ActivityNavigator(Context* context, Window* hostActivity = nullptr);
    NavDestination* createDestination() override;
    void navigate(NavDestination* destination, Bundle* args, NavOptions* navOptions) override;
    // androidx ActivityNavigator.navigate returns null — activity destinations do NOT go on the
    // NavController back stack (you can't pop back to the caller of a new Activity). Override the
    // modern entry-based navigate to run the per-destination startActivity WITHOUT pushing entries
    // onto this navigator's state (the base default pushes, leaving a phantom Back press).
    void navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Navigator::Extras* extras) override;
    bool popBackStack() override;

    Context* getContext() const { return mContext; }
    // androidx hostActivity: the Activity walking the ContextWrapper chain. CDROID has no
    // ContextWrapper; the host Window is resolved when the startActivity wiring lands — nullptr for
    // now (popBackStack returns false; navigate adds FLAG_ACTIVITY_NEW_TASK like a non-Activity ctx).
    Window* getHostActivity() const { return mHostActivity; }
    void setHostActivity(Window* host) { mHostActivity = host; }

    // androidx applyPopAnimationsToPendingTransition(activity) — CDROID has no activity pending
    // transition override yet; no-op stub.
    static void applyPopAnimationsToPendingTransition(Window* activity);

    static constexpr const char* EXTRA_NAV_SOURCE     = "android-support-navigation:ActivityNavigator:source";
    static constexpr const char* EXTRA_NAV_CURRENT    = "android-support-navigation:ActivityNavigator:current";
    static constexpr const char* EXTRA_POP_ENTER_ANIM = "android-support-navigation:ActivityNavigator:popEnterAnim";
    static constexpr const char* EXTRA_POP_EXIT_ANIM  = "android-support-navigation:ActivityNavigator:popExitAnim";
private:
    Context* mContext;
    Window* mHostActivity = nullptr;
};

// A navigation destination that starts an Activity (Window) via an Intent. Mirrors
// androidx ActivityNavigator.Destination.
class ActivityNavigator::Destination : public NavDestination{
public:
    explicit Destination(ActivityNavigator* activityNavigator) : NavDestination((Navigator*)activityNavigator) {}
    ~Destination() override { delete mIntent; }
    void onInflate(Context* context, const AttributeSet& attrs) override;
    // androidx ActivityNavigator.Destination.supportsActions: activity destinations don't support
    // <action> elements (return false).
    bool supportsActions() const override { return false; }
    bool equals(const Destination& other) const;
    int hashCode() const;

    Intent* getIntent() const { return mIntent; }
    Destination* setIntent(Intent* intent){ delete mIntent; mIntent = intent; return this; }
    const std::string& getDataPattern() const { return mDataPattern; }
    Destination* setDataPattern(const std::string& dataPattern){ mDataPattern = dataPattern; return this; }

    std::string getTargetPackage() const { return mIntent ? mIntent->getPackage() : std::string(); }
    Destination* setTargetPackage(const std::string& packageName){ requireIntent()->setPackage(packageName); return this; }
    ComponentName getComponent() const { return mIntent ? mIntent->getComponent() : ComponentName(); }
    Destination* setComponentName(const ComponentName& name){ requireIntent()->setComponent(name); return this; }
    std::string getAction() const { return mIntent ? mIntent->getAction() : std::string(); }
    Destination* setAction(const std::string& action){ requireIntent()->setAction(action); return this; }
    Uri* getData() const { return mIntent ? mIntent->getData() : nullptr; }
    Destination* setData(Uri* data){ requireIntent()->setData(data); return this; }
private:
    Intent* mIntent = nullptr; // owned
    std::string mDataPattern;
    Intent* requireIntent(){ if(mIntent == nullptr) mIntent = new Intent(); return mIntent; }
};

// androidx ActivityNavigator.Extras: Intent flags to add (+ ActivityOptionsCompat, which CDROID
// stubs — no shared-element activity transitions). Built via Builder.
class ActivityNavigator::Extras : public Navigator::Extras{
public:
    explicit Extras(int flags) : mFlags(flags) {}
    int getFlags() const { return mFlags; }
    class Builder{
    public:
        Builder& addFlags(int flags){ mFlags |= flags; return *this; }
        // androidx setActivityOptions(ActivityOptionsCompat) — CDROID has no activity options; no-op.
        Builder& setActivityOptions(void* /*activityOptions*/){ return *this; }
        Extras* build(){ return new Extras(mFlags); }
    private:
        int mFlags = 0;
    };
private:
    int mFlags;
};

}//namespace
#endif
