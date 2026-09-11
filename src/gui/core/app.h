/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include <string>
#include <map>
#include <atomic>
#include <functional>
#include <vector>
#include <unordered_map>
#include <istream>
#include <cairomm/surface.h>
#include <core/looper.h>
#include <content/contextimpl.h>
#include <content/typedarray.h>      // TypedArray: consumer-side typed attr view

namespace cxxopts{
    class ParseResult;
    class OptionAdder;
}
namespace cdroid{

class Window;
class ActivityOptions; // scene-transition (shared element) options, widget/activityoptions.h
class AssetManager2;   // androidfw (arscEngine return; defined in assetmanager2.h)
// AssetManager is forward-declared at global scope in context.h.

struct ActivityPendingResult { Window* caller; int requestCode; Window* target; };

// AOSP android.content.pm.ActivityInfo (micro): the fields CDROID consumes
// from the compiled AndroidManifest.xml in the app pak — the PackageManager
// role. Populated by App::parsePackageManifest (PackageParser micro-port).
struct ActivityInfo {
    std::string name;            // android:name (the REGISTER_ACTIVITY key)
    int theme = 0;               // android:theme (0 = inherit the application theme)
    int configChanges = 0;       // android:configChanges bits (Configuration::CONFIG_*)
    std::string label;           // android:label
    bool launchable = false;     // MAIN/LAUNCHER intent-filter present
    // android:screenOrientation in ActivityInfo numbering: landscape=0,
    // portrait=1, unspecified=-1 (differs from ResTable_config::ORIENTATION_*).
    int screenOrientation = -1;
};

// The Application AND the one ContextImpl: CDROID has no separate
// ActivityThread machinery, so App derives from ContextImpl (the resource
// stack owner) and merges the Application role into the process singleton,
// plus the main loop / window management.
class App:public ContextImpl{
private:
    bool mQuitFlag;
    int mExitCode;
    std::vector<ActivityPendingResult> mPendingResults;
    // Manifest-derived app metadata (PackageManager role).
    int mApplicationTheme = 0;                 // <application android:theme>
    std::string mApplicationLabel;             // <application android:label>
    std::string mPackageName;                  // <manifest package="..."> (stable id;
                                               // AOSP ContextImpl's package comes from
                                               // the same manifest attr via PackageManager)
    std::map<std::string, ActivityInfo> mActivityInfos;
    // Theme to apply to the NEXT instantiated activity window (set by
    // startActivity from the manifest, consumed by Window's Context ctor —
    // the theme must land on the themed context before the subclass ctor
    // inflates content; AOSP performs this in performLaunchActivity).
    int mPendingActivityTheme = 0;
    void parsePackageManifest(const std::string& pakPath);
    // performLaunchActivity's create path: instantiate the Window (its ctor self-registers
    // with WindowManager) and stamp the Intent. Returns nullptr on the reuse paths
    // (singleTop / CLEAR_TOP-singleTop / REORDER_TO_FRONT delivered the Intent to an
    // existing instance) and when no Window is registered for the class name.
    Window* performLaunch(const Intent& intent);

    // --- resource stack (the former Assets; the ContextImpl role) ---------
    // Lazy ID-based resource layer (AOSP Resources/AssetManager), built
    // on first getResources()/getAssets() from the pak paths recorded in
    // addResource().
    std::vector<std::string>        mPakPaths;
    mutable AssetManager*  mAssetManager = nullptr;
    mutable cdroid::Resources*      mCdroidResources = nullptr;
    void ensureCdroidResources() const;

    // i18n.dat contents loaded from cdroid.pak (App::onInit) — a binary blob, hence
    // vector<char>. Its heap buffer is the backing store for i18n::DataResource's static
    // pointer, so it is filled ONCE and must never be modified afterwards (a reallocating
    // write would dangle the static pointer). ~App detaches the pointer
    // (DataResource::SetData(nullptr, 0)) before this member frees.
    std::vector<char> mI18nData;
    // The pak registry (mResources) and the arsc theme engine (mArscTheme)
    // live on Context (the ContextImpl role); App inherits them.
    bool arscResolveHexRef(const std::string& s, TypedValue* out) const;
    void parseItem(const std::string&package,const std::string&resid,const std::vector<std::string>&tag,std::vector<AttributeSet>atts,const std::string&value,void*);
    // Rebuild the live arsc theme for `resid` (setTheme's engine side).
    void applyTheme(int resid);
    // Release the resource stack (called from ~App after the UI is down).
    void destroyResourceState();
protected:
    std::string mName;
    DisplayMetrics mDisplayMetrics;
    int addResource(const std::string&path,const std::string&name=std::string());
    std::unique_ptr<cxxopts::ParseResult> mArgsResult;
    static std::atomic<App*>mInst;
    void onInit();
public:
     App(int argc=0,const char*argv[]=NULL);
     ~App()override;
     static App&getInstance();
     // Register application-specific command line options at static-init time
     // (before App is constructed): App's --help then lists them in their own
     // `group` beside the framework's. The application still reads the values
     // from its own cxxopts pass — this only merges the help listing.
     static bool addAppOptions(const std::string& group,
             const std::function<void(cxxopts::OptionAdder&)>& adder);
     const std::string getDataPath()const;
     virtual void setOpacity(unsigned char alpha);
     virtual const std::string getName()const;
     bool hasArg(const std::string&key)const;
     bool hasSwitch(const std::string&key)const;
     const std::string getArg(const std::string&key,const std::string&fallback_value)const;
     int getArgAsInt(const std::string&key,int fallback_value)const;
     float getArgAsFloat(const std::string&key,float fallback_value)const;
     double getArgAsDouble(const std::string&key,double fallback_value)const;
     size_t getParamCount()const;
     std::string getParam(int idx,const std::string&def="")const;
     virtual void addEventHandler(const EventHandler* handler);
     virtual void removeEventHandler(const EventHandler*handler);
     virtual int exec();
     // Activity result mediation: Window.startActivityForResult → target.setResult → close →
     // caller.onActivityResult. App tracks the caller↔target mapping.
     void startActivityForResultInternal(Window* caller, const Intent& intent, int requestCode,
                                         ActivityOptions* options = nullptr);
     void dispatchPendingResult(Window* target);
     virtual void exit(int code=0);
     // androidx ActivityNavigator ends in context.startActivity(intent). Real impl: resolve the
     // Intent's ComponentName.className via ActivityFactory (REGISTER_ACTIVITY) and `new` the Window
     // (its ctor self-registers with WindowManager, so it shows), then stamp the Intent on it.
     void startActivity(const Intent& intent) override;
     // AOSP Context/Activity.startActivity(Intent, Bundle options): with an ActivityOptions scene
     // transition (makeSceneTransitionAnimation), the started Window's coordinator captures the
     // caller's shared elements and flies them (see Window::setSharedElementEnter). Takes
     // ownership of `options` (Android hands its Bundle to the system; the object dies here).
     void startActivity(const Intent& intent, ActivityOptions* options) override;
    // AOSP ActivityThread.handleConfigurationChanged(Configuration): the system
    // side of a configuration change — applies it to the resources (variant
    // reselection + cache invalidation) and then, per activity, either
    // dispatches onConfigurationChanged (when its configChanges covers every
    // changed bit) or relaunches it (recreate). CDROID's "system" entry point:
    // apps call this to switch uiMode (night), locale, density, ...
    void handleConfigurationChanged(const Configuration& newConfig);
    // AOSP Application.onConfigurationChanged (ComponentCallbacks2): the app-
    // level callback, invoked by the "system" side (ActivityThread role) before
    // the per-activity dispatch/recreate. Override to react app-wide.
    virtual void onConfigurationChanged(const Configuration& newConfig){(void)newConfig;}

    // --- PackageManager face (from the app pak's compiled manifest) ---
    // AOSP ApplicationInfo.theme / ActivityInfo lookups.
    int getApplicationTheme() const { return mApplicationTheme; }
    // App-wide setTheme (the "apply it app-wide" lever): besides rebuilding the
    // live theme (applyTheme), the runtime choice becomes the
    // manifest-equivalent default so windows launched afterwards — including
    // Window::recreate() relaunches — build their ContextThemeWrapper under it
    // (startActivity reads mApplicationTheme when the activity has no
    // android:theme of its own). Without this, recreate() keeps inflating
    // under the boot theme and themed views never re-color.
    void setTheme(int resid) override;
    const std::string& getApplicationLabel() const { return mApplicationLabel; }
    const ActivityInfo* getActivityInfo(const std::string& name) const;
    // The MAIN/LAUNCHER activity (empty when the manifest has none).
    std::string getLauncherActivity() const;
    // Orientation into the arsc request config so -land/-port resource
    // variants select (AOSP: WMS owns the effective orientation; CDROID has
    // no rotation, so it is resolved once at startup): --orientation switch >
    // launcher activity's android:screenOrientation > screen shape.
    void applyOrientationConfig(const std::string& forced = std::string());
    friend class Window;   // consumes mPendingActivityTheme in its Context ctor

    // --- Context implementation (the ContextImpl face) ---------------------
    // The AM2 table behind this App's AssetManager (ContextImpl::arscEngine).
    AssetManager2* arscEngine() const override;
    // Binary-AXML bridge (transitional): resolve a resource ID / fetch a string
    // from the loaded arsc so the parsers can render typed attribute values.
    bool arscResolveId(uint32_t resId, TypedValue* out) const;
    const char16_t* arscStringAt(uint32_t resId, size_t* outLen) const;
    // Render a resource ID as an "@type/key" reference string (e.g.
    // "@drawable/bg", "@string/hello") matching text-XML form, so CDROID's
    // existing string-based resolvers consume binary-AXML references unchanged.
    std::string getResourceName(uint32_t resId) const override;
    // Resolve a theme-attribute reference (?attr/<id>) through the arsc Theme:
    // getAttribute + resolveAttributeReference, so ?android:colorPrimary etc.
    // flatten to a concrete value. Returns true if the theme had the attr.
    // When outBlock != null, *outBlock receives the owning string-pool block of
    // the resolved value (needed to resolve TYPE_STRING values via stringAtBlock).
    bool arscThemeAttribute(uint32_t attrId, TypedValue* out, ssize_t* outBlock = nullptr) const;
    std::string getPackageName()const override;
    Resources::Theme getTheme() override;
    const DisplayMetrics&getDisplayMetrics()const override;
    // loadImage/openAsset: inherited from Context (impls in contextimpl.cc).
    // AOSP ID-based resource face.
    Resources&      getResources() override;
    AssetManager&   getAssets() override;
    // getDrawable/getColorStateList: inherit Context's AOSP-final themed
    // defaults (getResources().getDrawable(id, getTheme())).
    // Bring the ID-based obtainStyledAttributes(const uint32_t*) overloads from
    // Context into App scope; otherwise the string overload above hides them
    // (C++ name hiding).
    using Context::obtainStyledAttributes;
    // AOSP Context.obtainStyledAttributes(AttributeSet, int[], defStyleAttr, defStyleRes).
    // `attrs` is nullable (AOSP new View(ctx, null, defStyleAttr)); `styleable` is a
    // sentinel-terminated attr-id array (internal::R::styleable::X). Overrides Context's pure
    // virtual with arsc resolution (element > style= > defStyleAttr > defStyleRes).
    std::unique_ptr<TypedArray> obtainStyledAttributes(
        const AttributeSet* attrs, const uint32_t* styleable,
        int32_t defStyleAttr = 0, int32_t defStyleRes = 0) override;
};

}/*end ofnamespace*/
#endif/*__APPLICATION_H__*/
