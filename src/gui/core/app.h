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
#include <istream>
#include <cairomm/surface.h>
#include <core/looper.h>
#include <core/context.h>
#include <core/assets.h>

namespace cxxopts{
    class ParseResult;
}
namespace cdroid{

class Window;
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
};

class App:public Assets{
private:
    bool mQuitFlag;
    int mExitCode;
    std::vector<ActivityPendingResult> mPendingResults;
    // Manifest-derived app metadata (PackageManager role).
    int mApplicationTheme = 0;                 // <application android:theme>
    std::string mApplicationLabel;             // <application android:label>
    std::map<std::string, ActivityInfo> mActivityInfos;
    // Theme to apply to the NEXT instantiated activity window (set by
    // startActivity from the manifest, consumed by Window's Context ctor —
    // the theme must land on the themed context before the subclass ctor
    // inflates content; AOSP performs this in performLaunchActivity).
    int mPendingActivityTheme = 0;
    void parsePackageManifest(const std::string& pakPath);
    Window* mLastStartedWindow = nullptr;
protected:
    std::unique_ptr<cxxopts::ParseResult> mArgsResult;
    static std::atomic<App*>mInst;
    void onInit();
public:
     App(int argc=0,const char*argv[]=NULL);
     ~App()override;
     static App&getInstance();
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
     void startActivityForResultInternal(Window* caller, const Intent& intent, int requestCode);
     void dispatchPendingResult(Window* target);
     virtual void exit(int code=0);
     // androidx ActivityNavigator ends in context.startActivity(intent). Real impl: resolve the
     // Intent's ComponentName.className via ActivityFactory (REGISTER_ACTIVITY) and `new` the Window
     // (its ctor self-registers with WindowManager, so it shows), then stamp the Intent on it.
     void startActivity(const Intent& intent) override;
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
    const std::string& getApplicationLabel() const { return mApplicationLabel; }
    const ActivityInfo* getActivityInfo(const std::string& name) const;
    // The MAIN/LAUNCHER activity (empty when the manifest has none).
    std::string getLauncherActivity() const;
    friend class Window;   // consumes mPendingActivityTheme in its Context ctor
};

}/*end ofnamespace*/
#endif/*__APPLICATION_H__*/
