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
#ifndef __CONTEXT_H__
#define __CONTEXT_H__
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <core/callbackbase.h>
#include <core/attributeset.h>
#include <core/displaymetrics.h>
#include <content/resourcesimpl.h>   // cdroid::ResourcesImpl (+ Theme)
#include <content/resources.h> // cdroid::Resources (full def — getResources() returns it)

#define USE(FEATURE) (defined(USE_##FEATURE) && USE_##FEATURE)
#define ENABLE(FEATURE) (defined(ENABLE_##FEATURE) && ENABLE_##FEATURE)

// AOSP-aligned resource types live in namespace cdroid (androidfw sub-library,
// compiled into cdroid.so). Their full definitions come via <core/resourcesimpl.h>
// included above (ResourcesImpl / AssetManager / Asset / TypedValue).
namespace cdroid{
class Drawable;
class ColorStateList;
class Typeface;
class Intent;
class ActivityOptions;
class TypedArray;
class Resources;       // cdroid::Resources (resources.h) — the GUI subclass
class SharedPreferences;
class Looper;

// Porting scope: Context's system-service face — permissions (check/enforce),
// broadcasts, services, ContentResolver, getSystemService, PackageManager,
// wallpaper, databases, attribution — is intentionally NOT ported. Those are
// Binder round-trips to system_server and CDROID has no process boundary; only
// the in-process faces (resources, themes, app-data dirs, activity launches,
// prefs) are modeled, with case-by-case seams documented at each method.
class Context{
public:
    virtual ~Context() = default;
    virtual std::string getPackageName() const = 0;
    // CDROID seam: androidx ActivityNavigator ends in context.startActivity(intent). CDROID has no
    // framework "start Activity by Intent" (Activity == Window, instantiated by `new`, not by name),
    // so the default is a no-op. Wiring (className -> Window factory + show) is deferred; override
    // (e.g. on App) to actually launch. Kept as Context* so Navigator's mContext->startActivity compiles.
    virtual void startActivity(const Intent& /*intent*/) = 0;
    // AOSP Context.startActivity(Intent, Bundle): the options form (ActivityOptions scene
    // transitions). The default forwards to the plain launch — AOSP's no-options
    // equivalence; App overrides to consume the scene-transition options.
    virtual void startActivity(const Intent& intent, ActivityOptions* /*options*/) { startActivity(intent); }
    // AOSP-aligned Theme access. getTheme() returns the live Resources::Theme
    // (engine = cdroid::Theme, the AM2 Theme). setTheme(@StyleRes int) applies a
    // style resource (AOSP Context.setTheme).
    virtual Resources::Theme getTheme() = 0;
    virtual void setTheme(int resid) = 0;
    virtual const DisplayMetrics&getDisplayMetrics() const = 0;
    // AOSP Context.getMainLooper() (abstract there): CDROID's default forwards
    // to the process main looper (Looper::getMainLooper) — the single looper
    // every Context in this process serves.
    virtual Looper* getMainLooper();
    // AOSP Context.getApplicationContext() (abstract there): the application
    // scope Context — for CDROID the App singleton (an Activity's application
    // context is the application, never itself, matching AOSP semantics).
    virtual Context* getApplicationContext();
    // AOSP concrete Context.getNextAutofillId(): a process-wide counter.
    virtual int getNextAutofillId();
    // String-key raw access (the former getInputStream istream face): resolves
    // "@[package:]type/name" refs and pak entry paths into a buffer-backed
    // Asset (file-backed for on-disk paths). Caller owns the returned Asset;
    // AssetInputStream (core/iostreams.h) is the owning istream wrapper.
    virtual Asset* openAsset(const std::string&resname) = 0;

    virtual Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname,int width,int height) = 0;
    // Int face (binary paks): opens the resource by id via openRawResource —
    // string "pkg:type/key" refs cannot be opened from a binary pak.
    virtual Cairo::RefPtr<Cairo::ImageSurface> loadImage(int id,int width,int height) = 0;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(int id) { return loadImage(id,-1,-1); }
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname) {
        return loadImage(resname,-1,-1);
    }
    virtual Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream&,int width,int height) = 0;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream&stream) {
        return loadImage(stream,-1,-1);
    }

    // AOSP Resources.Theme.obtainStyledAttributes with AttributeSet == null.
    // Default impls delegate to getTheme() (see context.cc); the 4-arg
    // AttributeSet form is obtainStyledAttributes on Assets (binary AXML).
    // `attrs` is a sentinel-terminated attr-id array (trailing 0), matching
    // internal::R::styleable::X — the C++ analog of AOSP's int[] (no COUNT param).
    virtual std::unique_ptr<TypedArray> obtainStyledAttributes(const uint32_t* attrs);
    virtual std::unique_ptr<TypedArray> obtainStyledAttributes(int resid, const uint32_t* attrs);
    // AOSP Context.obtainStyledAttributes(AttributeSet, int[] attrs, int defStyleAttr, int defStyleRes).
    // attrs is nullable (AOSP new View(ctx, null, defStyleAttr)); styleable is a
    // sentinel-terminated attr-id array (internal::R::styleable::X, the C++ analog of
    // AOSP's int[]). The default routes through getTheme() (context.cc), so a
    // subclass that changes the theme only overrides getTheme(); App overrides
    // with AOSP's actual (final) routing through getResources().
    virtual std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet* attrs,
        const uint32_t* styleable, int32_t defStyleAttr=0, int32_t defStyleRes=0);
    // Resolve a resource id to its "@type/key" (or "@pkg:type/key") reference name —
    // the CDROID counterpart of AOSP Resources.getResourceName(resId). Default
    // empty; Assets overrides with arsc resolution.
    virtual std::string getResourceName(uint32_t resId) const { return std::string(); }

    // --- AOSP app-data directories (Context.java:1404-2040; paths, no File type). ---
    // Default impls (context.cc) resolve Environment + getPackageName(): the
    // data root is $ANDROID_DATA when set (AOSP contract), else /data when
    // writable, else ~/.cdroid (the host fallback). Wrappers need no extra
    // forwarding — the impls only call the virtual getPackageName().
    // getDataDir(): /data/data/<pkg>; getFilesDir(): <dataDir>/files;
    // getCacheDir(): <dataDir>/cache; getDir(): <dataDir>/app_<name> (AOSP
    // prefixes "app_"); getSharedPreferencesPath(): <dataDir>/shared_prefs/<n>.xml;
    // getFileStreamPath(): <filesDir>/<name>; external: <storage>/…/Android/data/<pkg>/… .
    virtual std::string getDataDir() const;
    virtual std::string getFilesDir() const;
    virtual std::string getCacheDir() const;
    virtual std::string getDir(const std::string& name, int mode) const;
    virtual std::string getExternalFilesDir(const std::string& type = std::string()) const;
    virtual std::string getExternalCacheDir() const;
    virtual std::string getFileStreamPath(const std::string& name) const;
    virtual std::string getSharedPreferencesPath(const std::string& name) const;
    // Convenience overload for a non-null AttributeSet& (delegates to the pointer
    // form). Defined out-of-line (context.cc) since it returns unique_ptr<TypedArray>.
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet& attrs,
        const uint32_t* styleable, int32_t defStyleAttr=0, int32_t defStyleRes=0);

    // --- AOSP-aligned ID-based resource face (android.content.Context) ---
    // Coexists with the string-based legacy methods above (overloads differ by
    // int vs std::string). Default implementations live in core/context.cc and
    // delegate to getResources(); pure-virtual ones (getResources/getAssets/
    // getDrawable(int)/getColorStateList(int) default to the themed path above
    // (AOSP final methods) — no subclass override needed.
    virtual Resources&      getResources() = 0;
    virtual AssetManager&   getAssets() = 0;
    virtual std::string    getString(int id);
    virtual std::u16string getText(int id);
    virtual std::string    getQuantityString(int id, int quantity);
    // AOSP final getString(int, Object...) / getQuantityString(int, int, Object...):
    // the format-args forms. Non-virtual like their AOSP counterparts — they delegate
    // to the virtual getResources(), so wrappers and themed subclasses need no override
    // (the same routing AOSP's final methods use). Args carry pre-stringified values
    // (Java stringifies via String.valueOf at Resources' door).
    std::string getQuantityString(int id, int quantity, const std::vector<std::string>& formatArgs);
    std::string getString(int id, const std::vector<std::string>& formatArgs);
    // Variadic convenience over the vector form: each argument is converted like
    // String.valueOf — std::string/const char* pass through, arithmetic values via
    // std::to_string, bool as "true"/"false". std::u16string args are not accepted
    // here (the core layer has no TextUtils); convert with TextUtils::utf16_utf8 at
    // the call site.
    template<typename... Args>
    std::string getString(int id, Args&&... args) {
        return getString(id, std::vector<std::string>{fmtArg(std::forward<Args>(args))...});
    }
    template<typename... Args>
    std::string getQuantityString(int id, int quantity, Args&&... args) {
        return getQuantityString(id, quantity,
                std::vector<std::string>{fmtArg(std::forward<Args>(args))...});
    }
    virtual int            getColor(int id);
    virtual bool           getBoolean(int id);
    virtual int            getInteger(int id);
    virtual float          getDimension(int id);
    virtual int            getDimensionPixelOffset(int id);
    virtual int            getDimensionPixelSize(int id);
    virtual Asset* openRawResource(int id);
    // AOSP final: resource loads are themed through getTheme() (context.cc).
    virtual Drawable*       getDrawable(int id);
    virtual std::shared_ptr<ColorStateList> getColorStateList(int id);
    virtual Typeface*       getFont(int id);   // default nullptr (deferred)

    // File creation mode: the default (and only modeled) mode, from
    // android.content.Context (MODE_PRIVATE = 0x00000000).
    static constexpr int MODE_PRIVATE = 0;

    // AOSP Context.getSharedPreferences(String, int): retrieve and hold the
    // contents of the preferences file, returning a SharedPreferences through
    // which you can retrieve and modify its values. Only MODE_PRIVATE is
    // modeled. The default implementation (context.cc) caches instances by
    // name so the same file always yields the same object, like AOSP.
    virtual std::shared_ptr<SharedPreferences> getSharedPreferences(
            const std::string& name, int mode);

private:
    // String.valueOf-shaped argument converters for the variadic getString/
    // getQuantityString overloads above.
    static const std::string& fmtArg(const std::string& s) { return s; }
    static std::string fmtArg(const char* s) { return s; }
    static std::string fmtArg(bool b) { return b ? "true" : "false"; }
    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    static std::string fmtArg(T v) { return std::to_string(v); }
};

}
#endif
