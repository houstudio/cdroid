// Default implementations of cdroid::Context's AOSP-aligned ID-based resource
// getters — delegate to getResources() (matching android.content.Context). The
// ID-based face coexists with the legacy string-based methods; concrete
// subclasses (Assets) supply getResources()/getAssets()/getDrawable(int)/
// getColorStateList(int). Header stays declaration-only.

#include "context.h"
#include "content/resourcesimpl.h"   // cdroid::ResourcesImpl (+ Theme)
#include "resources.h"      // cdroid::Resources (full def — getResources() returns it)
#include <content/typedarray.h>       // TypedArray (constructed below)
#include <content/androidfw/assetmanager2.h>   // AM2 engine + cdroid::Theme
#include <content/androidfw/attributeresolution.h>  // ResolveAttrs
#include <content/sharedpreferences.h>      // SharedPreferencesImpl (getSharedPreferences)
#include <core/environment.h>        // android.os.Environment port (data/storage roots)
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <map>

namespace cdroid {

std::string Context::getString(int id) {
    return getResources().getString(id);
}

std::u16string Context::getText(int id) {
    return getResources().getText(id);
}

std::string Context::getQuantityString(int id, int quantity) {
    return getResources().getQuantityString(id, quantity);
}

int Context::getColor(int id) {
    return getResources().getColor(id);
}

// AOSP Context.getDrawable(id) is final:
//   getResources().getDrawable(id, getTheme()) — the resource load is themed
// through the context's theme (ContextThemeWrapper overrides getTheme, so
// wrapped contexts resolve ?attr inside drawable/CSL XML with their overlay).
Drawable* Context::getDrawable(int id) {
    Resources::Theme t = getTheme();
    return getResources().getDrawable(id, &t);
}

// AOSP Context.getColorStateList(id) is final: loadColorStateList(id, getTheme()).
std::shared_ptr<ColorStateList> Context::getColorStateList(int id) {
    Resources::Theme t = getTheme();
    return getResources().getColorStateList(id, &t);
}

bool Context::getBoolean(int id) {
    return getResources().getBoolean(id);
}

int Context::getInteger(int id) {
    return getResources().getInteger(id);
}

float Context::getDimension(int id) {
    return getResources().getDimension(id);
}

int Context::getDimensionPixelSize(int id) {
    return getResources().getDimensionPixelSize(id);
}

Asset* Context::openRawResource(int id) {
    return getResources().openRawResource(id);
}

// AOSP Context.getFont: plain delegation to Resources.getFont.
Typeface* Context::getFont(int id) {
    return getResources().getFont(id);
}

// AOSP Resources.Theme.obtainStyledAttributes(attrs): resolve each attr against
// the live theme (defStyleAttr=0, defStyleRes=0). Delegates to getTheme() like
// the Java final in android.content.Context. `attrs` is sentinel-terminated.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const uint32_t* attrs) {
    Resources::Theme _th = getTheme(); cdroid::Theme* theme = static_cast<cdroid::Theme*>(_th._engineHandle());
    AssetManager2& am2 = *theme->GetAssetManager();
    size_t n = 0; while (attrs[n]) ++n;  // count up to the trailing-0 sentinel
    std::vector<uint32_t> values(n * STYLE_NUM_ENTRIES);
    ResolveAttrs(theme, 0, 0, nullptr, 0, attrs, n, values.data(), nullptr);
    std::vector<StyledAttr> styled(n);
    styledAttrsFromBlocks(values.data(), n, styled.data());
    return std::make_unique<TypedArray>(&am2, std::move(styled), nullptr,
                                        getResources().getDisplayMetrics().density, &getResources(), &_th);
}

// AOSP Theme.obtainStyledAttributes(resId, attrs): resolve against a style on
// top of the theme (defStyleAttr=0, defStyleRes=resId). `attrs` is sentinel-
// terminated.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(int resid, const uint32_t* attrs) {
    Resources::Theme _th = getTheme(); cdroid::Theme* theme = static_cast<cdroid::Theme*>(_th._engineHandle());
    AssetManager2& am2 = *theme->GetAssetManager();
    size_t n = 0; while (attrs[n]) ++n;
    std::vector<uint32_t> values(n * STYLE_NUM_ENTRIES);
    ResolveAttrs(theme, 0, (uint32_t)resid, nullptr, 0, attrs, n, values.data(), nullptr);
    std::vector<StyledAttr> styled(n);
    styledAttrsFromBlocks(values.data(), n, styled.data());
    return std::make_unique<TypedArray>(&am2, std::move(styled), nullptr,
                                        getResources().getDisplayMetrics().density, &getResources(), &_th);
}

// AOSP Context.obtainStyledAttributes(set, attrs, defStyleAttr, defStyleRes)
// is final and resolves against getTheme(); Assets overrides with the same
// resolution straight through Resources (no virtual detour needed).
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const AttributeSet* attrs,
        const uint32_t* styleable, int32_t defStyleAttr, int32_t defStyleRes) {
    return getTheme().obtainStyledAttributes(attrs, styleable, defStyleAttr, defStyleRes);
}

// Convenience overload: non-null AttributeSet& delegates to the pointer form.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const AttributeSet& attrs,
        const uint32_t* styleable, int32_t defStyleAttr, int32_t defStyleRes) {
    return obtainStyledAttributes(&attrs, styleable, defStyleAttr, defStyleRes);
}

// AOSP Context.getSharedPreferences(String, int): one SharedPreferences
// instance per (file) name for the lifetime of the process (AOSP semantics —
// same name must return the same object so listeners and writes stay
// coherent). AOSP's static cache is keyed by the resolved FILE PATH
// (ContextImpl.sSharedPrefsCache); CDROID keys by package+name, which is the
// same key once getSharedPreferencesPath() is deterministic.
std::shared_ptr<SharedPreferences> Context::getSharedPreferences(
        const std::string& name, int mode) {
    const std::string file = getSharedPreferencesPath(name);
    static std::map<std::string, std::weak_ptr<SharedPreferences>> cache;
    auto& slot = cache[file];
    std::shared_ptr<SharedPreferences> sp = slot.lock();
    if (!sp) {
        sp = std::make_shared<SharedPreferencesImpl>(file, mode);
        slot = sp;
    }
    return sp;
}

// ===========================================================================
// AOSP app-data directories (Context.java:1404-2040). The data root follows
// the AOSP contract: $ANDROID_DATA when set (Environment.getDataDirectory()
// reads it), else /data when writable, else ~/.cdroid — the host fallback
// that keeps dev machines working without root. Everything below the root is
// the AOSP shape: data/data/<pkg>/{files,cache,shared_prefs,...} and
// <storage>/…/Android/data/<pkg>/{files,cache}.
// ===========================================================================

static std::string appDataRoot() {
    // AOSP: the runtime owns /data via ANDROID_DATA; an explicit env value is
    // trusted as-is (contract). Without it, /data is used only when the
    // process can actually write it (device); otherwise fall back to $HOME.
    const std::string data = Environment::getDataDirectory();
    if (!data.empty() && getenv("ANDROID_DATA") != nullptr) return data;
    if (access("/data", W_OK) == 0) return "/data";
    const char* home = getenv("HOME");
    return std::string((home && *home) ? home : "/tmp") + "/.cdroid";
}

// mkdir -p for the data-directory chain (AOSP ensures dirs on demand).
static void ensureDir(const std::string& path) {
    std::string cur;
    for (size_t i = 0; i <= path.size(); i++) {
        const char c = (i < path.size()) ? path[i] : '/';
        if (c == '/' && !cur.empty()) {
            mkdir(cur.c_str(), 0770);
        }
        if (i < path.size()) cur += c;
    }
}

// Context.getDataDir(): /data/data/<pkg> (AOSP :1437; the user_ce dir).
std::string Context::getDataDir() const {
    const std::string dir = appDataRoot() + "/data/" + getPackageName();
    ensureDir(dir);
    return dir;
}

// Context.getFilesDir(): <dataDir>/files (AOSP :1454).
std::string Context::getFilesDir() const {
    const std::string dir = getDataDir() + "/files";
    ensureDir(dir);
    return dir;
}

// Context.getCacheDir(): <dataDir>/cache (AOSP :1805).
std::string Context::getCacheDir() const {
    const std::string dir = getDataDir() + "/cache";
    ensureDir(dir);
    return dir;
}

// Context.getDir(name, mode): <dataDir>/app_<name> — AOSP ContextImpl
// prefixes "app_" (:2040). The mode governs permissions only; path shape is
// mode-independent.
std::string Context::getDir(const std::string& name, int /*mode*/) const {
    const std::string dir = getDataDir() + "/app_" + name;
    ensureDir(dir);
    return dir;
}

// Context.getExternalStorage…: <storage>/…/Android/data/<pkg>/… resolved
// through the Environment port's UserEnvironment builders (AOSP :1589/:1883).
std::string Context::getExternalFilesDir(const std::string& type) const {
    std::vector<std::string> dirs = Environment::buildExternalStorageAppFilesDirs(getPackageName());
    std::string path = dirs.empty() ? std::string() : dirs[0];
    if (!type.empty()) path = path.empty() ? path : path + "/" + type;
    return path;
}

std::string Context::getExternalCacheDir() const {
    std::vector<std::string> dirs = Environment::buildExternalStorageAppCacheDirs(getPackageName());
    return dirs.empty() ? std::string() : dirs[0];
}

// Context.getSharedPreferencesPath(name): <dataDir>/shared_prefs/<n>.xml
// (AOSP :1420).
std::string Context::getSharedPreferencesPath(const std::string& name) const {
    return getDataDir() + "/shared_prefs/" + name + ".xml";
}

// Context.getFileStreamPath(name): <filesDir>/<name> (AOSP :1404).
std::string Context::getFileStreamPath(const std::string& name) const {
    return getFilesDir() + "/" + name;
}

} // namespace cdroid
