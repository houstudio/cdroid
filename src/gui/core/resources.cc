// cdroid::Resources — forwarding to the aggregated ResourcesImpl + own GUI
// factories + obtainStyledAttributes/obtainTypedArray. resourcesimpl.h is
// included HERE (not in resources.h) so the heavy androidfw headers stay hidden
// from the many files that include resources.h via context.h.

#include "resources.h"
#include <core/context.h>
#include <core/app.h>                  // App::getInstance() (getSystem)
#include <core/attributeset.h>
#include <core/typedarray.h>
#include <core/xmlpullparser.h>
#include <androidfw/restable.h>        // obtainStyledAttributes resolver, ResXMLTree, StyledAttr
#include <core/resourcesimpl.h>   // ResourcesImpl (aggregated)
#include <core/assetmanager.h>      // AssetManager (getAssets()->getResources)
#include <core/typedvalue.h>   // TypedValue (getValue/getColorStateList)
#include <drawable/drawable.h>
#include <drawable/colorstatelist.h>   // ColorStateList (forward decl in resources.h)

namespace cdroid {

// ===========================================================================
// Construction / destruction. Resources is a thin forwarder now: the AOSP
// mDrawableCache / mComplexColorCache + loadDrawable/loadComplexColor live in
// the aggregated ResourcesImpl (resourcesimpl.cc). Resources hands its Context
// to ResourcesImpl once — the inflation bridge (AOSP passes the Resources
// wrapper into loadDrawable; CDROID's aggregation can't reach it).
// ===========================================================================

Resources::Resources(AssetManager* am, cdroid::Context* ctx)
    : mImpl(std::make_unique<ResourcesImpl>(am, nullptr,
              ctx ? &ctx->getDisplayMetrics() : nullptr)),
      mCtx(ctx) {
    mImpl->setContext(ctx);
}

Resources::~Resources() = default;   // unique_ptr<ResourcesImpl> dtor instantiated here

// ===========================================================================
// Forwarded to mImpl (AOSP Resources -> ResourcesImpl)
// ===========================================================================

AssetManager* Resources::getAssets() const {
    return mImpl->getAssets();
}

// AOSP Resources.getConfiguration(): the live Configuration.
const Configuration& Resources::getConfiguration() const {
    return mImpl->getConfiguration();
}

// AOSP Resources.updateConfiguration(@Nullable Configuration, @Nullable DisplayMetrics).
void Resources::updateConfiguration(const Configuration* config, const DisplayMetrics* metrics) {
    mImpl->updateConfiguration(config, metrics);
}

// AOSP Resources.calcConfigChanges(@Nullable Configuration): the change bits.
int Resources::calcConfigChanges(const Configuration* config) const {
    return mImpl->calcConfigChanges(config);
}

const DisplayMetrics& Resources::getDisplayMetrics() const {
    return mImpl->getDisplayMetrics();
}

void Resources::setConfiguration(const ResTable_config& c) {
    mImpl->setConfiguration(c);
}

void Resources::setDisplayMetrics(const DisplayMetrics& m) {
    mImpl->setDisplayMetrics(m);
}

int Resources::getIdentifier(const std::string& n, const std::string& t, const std::string& p) const {
    return mImpl->getIdentifier(n, t, p);
}

bool Resources::getResourceName(int id, std::string* o) const {
    return mImpl->getResourceName(id, o);
}

bool Resources::getResourceEntryName(int id, std::string* o) const {
    return mImpl->getResourceEntryName(id, o);
}

bool Resources::getResourceTypeName(int id, std::string* o) const {
    return mImpl->getResourceTypeName(id, o);
}

bool Resources::getResourcePackageName(int id, std::string* o) const {
    return mImpl->getResourcePackageName(id, o);
}

bool Resources::getValue(int id, TypedValue* o, bool r) const {
    return mImpl->getValue(id, o, r);
}

bool Resources::getValue(const std::string& n, TypedValue* o, bool r) const {
    return mImpl->getValue(n, o, r);
}

std::string Resources::getString(int id) const {
    return mImpl->getString(id);
}

std::u16string Resources::getText(int id) const {
    return mImpl->getText(id);
}

std::u16string Resources::getText(int id, const std::u16string& def) const {
    return mImpl->getText(id, def);
}

int Resources::getInteger(int id) const {
    return mImpl->getInteger(id);
}

bool Resources::getBoolean(int id) const {
    return mImpl->getBoolean(id);
}

float Resources::getFloat(int id) const {
    return mImpl->getFloat(id);
}

int Resources::getColor(int id) const {
    return mImpl->getColor(id);
}

// AOSP Resources.getColor(int id, @Nullable Theme theme) →
// impl.loadComplexColor(id, theme).getDefaultColor().
int Resources::getColor(int id, const Theme* theme) const {
    auto cc = mImpl->loadComplexColor(id, theme ? theme->_engineHandle() : nullptr);
    return cc ? cc->getDefaultColor() : mImpl->getColor(id);
}

float Resources::getDimension(int id) const {
    return mImpl->getDimension(id);
}

int Resources::getDimensionPixelOffset(int id) const {
    return mImpl->getDimensionPixelOffset(id);
}

int Resources::getDimensionPixelSize(int id) const {
    return mImpl->getDimensionPixelSize(id);
}

float Resources::getFraction(int id, float b, float p) const {
    return mImpl->getFraction(id, b, p);
}

std::string Resources::getQuantityString(int id, int q) const {
    return mImpl->getQuantityString(id, q);
}

std::u16string Resources::getQuantityText(int id, int q) const {
    return mImpl->getQuantityText(id, q);
}

std::vector<std::string> Resources::getStringArray(int id) const {
    return mImpl->getStringArray(id);
}

std::vector<std::u16string> Resources::getTextArray(int id) const {
    return mImpl->getTextArray(id);
}

std::vector<int> Resources::getIntArray(int id) const {
    return mImpl->getIntArray(id);
}

Asset* Resources::openRawResource(int id, TypedValue* o) const {
    return mImpl->openRawResource(id, o);
}

Asset* Resources::getXml(int id) const {
    return mImpl->getXml(id);
}

Asset* Resources::getLayout(int id) const {
    return mImpl->getLayout(id);
}

Asset* Resources::getAnimation(int id) const {
    return mImpl->getAnimation(id);
}

Typeface* Resources::getFont(int id) const {
    return mImpl->getFont(id);
}

// AOSP ResourcesImpl.loadComplexColor — owned by the aggregated ResourcesImpl
// (cache + createFromXml). Resources just forwards.
std::shared_ptr<ComplexColor> Resources::loadComplexColor(int id, const Theme* theme) const {
    return mImpl->loadComplexColor(id, theme ? theme->_engineHandle() : nullptr);
}

Movie* Resources::getMovie(int id) const {
    return mImpl->getMovie(id);
}

// ===========================================================================
// ===========================================================================
// AOSP parity additions (getSystem, format-arg overloads, openRawResourceFd)
// ===========================================================================

Resources& Resources::getSystem() {
    return App::getInstance().getResources();
}

// Simple %s/%d/%f placeholder replacement (not full Java Formatter — enough for
// the common getString(R.string.x, arg) pattern).
namespace {
std::string formatWithArgs(const std::string& fmt, const std::vector<std::string>& args) {
    if (args.empty()) return fmt;
    std::string out;
    size_t ai = 0;
    for (size_t i = 0; i < fmt.size(); i++) {
        if (fmt[i] == '%' && i + 1 < fmt.size() && ai < args.size()) {
            const char c = fmt[i + 1];
            if (c == 's' || c == 'd' || c == 'f') { out += args[ai++]; i++; continue; }
        }
        out += fmt[i];
    }
    return out;
}
} // anon

std::string Resources::getString(int id, const std::vector<std::string>& formatArgs) const {
    return formatWithArgs(mImpl->getString(id), formatArgs);
}

std::string Resources::getQuantityString(int id, int quantity, const std::vector<std::string>& formatArgs) const {
    return formatWithArgs(mImpl->getQuantityString(id, quantity), formatArgs);
}

Asset* Resources::openRawResourceFd(int id) const {
    (void)id;
    return nullptr;   // CDROID has no AssetFileDescriptor (fd-based assets)
}

// ===========================================================================
// GUI factories — thin forwarders. The AOSP mDrawableCache / mComplexColorCache
// + loadDrawable/loadComplexColor live in the aggregated ResourcesImpl (it can
// use cairo + the Context inflation bridge now that it's in the cdroid target).
// ===========================================================================

// AOSP Resources.getDrawable(int id, @Nullable Theme theme)
//   → getDrawableForDensity(id, 0, theme).
cdroid::Drawable* Resources::getDrawable(int id, const Theme* theme) const {
    return mImpl->getDrawable(id, 0, theme ? theme->_engineHandle() : nullptr);
}

cdroid::Drawable* Resources::getDrawableForDensity(int id, int density, const Theme* theme) const {
    return mImpl->getDrawableForDensity(id, density, theme ? theme->_engineHandle() : nullptr);
}

std::shared_ptr<ColorStateList> Resources::getColorStateList(int id, const Theme* theme) const {
    return mImpl->getColorStateList(id, theme ? theme->_engineHandle() : nullptr);
}


// Text-XML element attributes for the non-binary obtainStyledAttributes path:
// the theme-only androidfw resolver has no element step (by design — AOSP
// Theme.obtainStyledAttributes), but AOSP's set-based resolution always
// prefers element attrs. Text XML (text-mode XmlPullParser / plain
// AttributeSet) therefore layers this ON TOP: each styleable attr whose id
// matches an element attribute (AttributeSet::getAttributeNameResource
// resolves text names through the arsc) overrides the theme/style fallback
// with the parsed text value — the missing piece that made text-packed
// res/color/ selectors (text_color_primary.xml) read no android:color at all
// and fall to MAGENTA.
static void applyTextElementAttrs(const AttributeSet& set, const ResTable& table,
                                  const uint32_t* attrs, StyledAttr* out) {
    const int n = set.getAttributeCount();
    if (n <= 0) return;
    for (size_t i = 0; attrs[i] != 0; i++) {
        for (int j = 0; j < n; j++) {
            if ((uint32_t)set.getAttributeNameResource(j) != attrs[i]) continue;
            const std::string v = set.getAttributeValue(j);
            if (v.empty()) break;
            Res_value rv = {};
            if (v[0] == '#') {
                // Color literal.
                uint32_t argb = 0; unsigned r=0,g=0,b=0,a=0xff;
                if (v.size()==4) { sscanf(v.c_str(), "#%1x%1x%1x", &r,&g,&b);
                    argb = (a<<24)|(r<<20)|(g<<12)|(b<<4); rv.dataType = Res_value::TYPE_INT_COLOR_RGB4; }
                else if (v.size()==5) { sscanf(v.c_str(), "#%1x%1x%1x%1x", &a,&r,&g,&b);
                    argb = (a<<28)|(r<<20)|(g<<12)|(b<<4); rv.dataType = Res_value::TYPE_INT_COLOR_ARGB4; }
                else if (v.size()==7) { sscanf(v.c_str(), "#%2x%2x%2x", &r,&g,&b);
                    argb = (a<<24)|(r<<16)|(g<<8)|b; rv.dataType = Res_value::TYPE_INT_COLOR_RGB8; }
                else { sscanf(v.c_str(), "#%2x%2x%2x%2x", &a,&r,&g,&b);
                    argb = (a<<24)|(r<<16)|(g<<8)|b; rv.dataType = Res_value::TYPE_INT_COLOR_ARGB8; }
                rv.data = argb;
            } else if (v[0] == '?' || v[0] == '@'
                       || (v[0] == ':' && v.find('/') != std::string::npos)
                       || v.compare(0, 7, "cdroid:") == 0) {
                // Reference forms: raw "?attr/name"/"@type/name", or the
                // normalized AttributeSet form (AttributeSet::normalize rewrites
                // a leading '?' to a package prefix: ":attr/name" /
                // "cdroid:attr/name"). Strip to name(+type).
                char kind = v[0];
                std::string body = (kind == '?' || kind == '@') ? v.substr(1) : v;
                if (kind != '?' && kind != '@') {
                    // Normalized form ":type/name" — the leading '@'/'?' was
                    // stripped: "attr/" means a ?attr reference, any other
                    // type is an @resource reference.
                    const size_t colon2 = body.find(':');
                    if (colon2 != std::string::npos) body = body.substr(colon2 + 1);
                    kind = (body.compare(0, 5, "attr/") == 0) ? '?' : '@';
                }
                std::string pkg, rest = body;
                const size_t colon = body.find(':');
                if (colon != std::string::npos) { pkg = body.substr(0, colon); rest = body.substr(colon + 1); }
                std::string type = (v[0] == '?') ? "attr" : "attr";
                std::string name = rest;
                if (rest.compare(0, 4, "attr/") == 0) name = rest.substr(4);
                else { const size_t slash = rest.find('/'); if (slash != std::string::npos) { type = rest.substr(0, slash); name = rest.substr(slash + 1); } }
                const uint32_t id = table.getIdentifier(name, type, pkg.empty() ? "" : pkg.c_str());
                if (id != 0) {
                    rv.data = id;
                    rv.dataType = (kind == '?') ? Res_value::TYPE_ATTRIBUTE : Res_value::TYPE_REFERENCE;
                    out[i].resourceId = id;
                } else break;   // unresolvable: keep the fallback value
            } else if (v == "true" || v == "false") {
                rv.data = (v == "true") ? 1 : 0;
                rv.dataType = Res_value::TYPE_INT_BOOLEAN;
            } else {
                // Decimal int (enum/flag values land here after normalize).
                char* end = nullptr;
                const long num = strtol(v.c_str(), &end, 10);
                if (end && *end == '\0' && end != v.c_str()) {
                    rv.data = (uint32_t)num;
                    rv.dataType = Res_value::TYPE_INT_DEC;
                } else break;   // free-form string: no pool here, keep fallback
            }
            out[i].value = rv;
            out[i].stringBlock = -2;   // element-sourced (no pool block)
            out[i].set = true;
            break;
        }
    }
}

// ===========================================================================
// AOSP Resources.obtainStyledAttributes(...)
// ===========================================================================

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet* set,
         const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    Resources::Theme _th = mCtx->getTheme();
    ResTable::Theme* theme = static_cast<ResTable::Theme*>(_th._engineHandle());
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);

    if (set != nullptr) {
        const XmlPullParser* parser = dynamic_cast<const XmlPullParser*>(set);
        if (parser && parser->isBinaryAXML()) {
            const ResXMLTree* xml = static_cast<const ResXMLTree*>(parser->getBinaryAXMLTree());
            if (xml) {
                cdroid::obtainStyledAttributes(*xml, rt, theme, attrs,
                                               (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
                return std::make_unique<TypedArray>(rt, std::move(styled), xml, getDisplayMetrics().density, this, &_th);
            }
        }
    }
    cdroid::obtainStyledAttributes(rt, theme, attrs,
                                   (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
    if (set != nullptr) {
        // AOSP precedence: element attrs override the style/theme fallback.
        applyTextElementAttrs(*set, rt, attrs, styled.data());
    }
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this, &_th);
}

// Convenience: AttributeSet& → AttributeSet* (for AOSP callers passing the reference).
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet& set,
        const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    return obtainStyledAttributes(&set, attrs, defStyleAttr, defStyleRes);
}

// AOSP Resources.obtainAttributes(set, attrs): theme-less — only the
// attributes explicitly set in the XML, no style/theme resolution
// (AOSP ResourcesImpl.obtainStyledAttributes(set, attrs, theme=null)).
std::unique_ptr<TypedArray> Resources::obtainAttributes(const AttributeSet* set, const uint32_t* attrs) const {
    if (mCtx == nullptr || set == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    size_t count = 0;
    while (attrs[count]) ++count;   // sentinel-terminated
    std::vector<StyledAttr> styled(count);
    const XmlPullParser* parser = dynamic_cast<const XmlPullParser*>(set);
    const ResXMLTree* xml = (parser && parser->isBinaryAXML())
            ? static_cast<const ResXMLTree*>(parser->getBinaryAXMLTree()) : nullptr;
    if (xml) {
        cdroid::obtainStyledAttributes(*xml, rt, /*theme*/nullptr, attrs, 0, 0, styled.data());
    } else {
        cdroid::obtainStyledAttributes(rt, /*theme*/nullptr, attrs, 0, 0, styled.data());
    }
    return std::make_unique<TypedArray>(rt, std::move(styled), xml,
                                        getDisplayMetrics().density, this, /*theme*/nullptr);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const uint32_t* attrs) const {
    return obtainStyledAttributes(0, attrs);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(int resid, const uint32_t* attrs) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    Resources::Theme _th = mCtx->getTheme();
    ResTable::Theme* theme = static_cast<ResTable::Theme*>(_th._engineHandle());
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);
    cdroid::obtainStyledAttributes(rt, theme, attrs, 0, (uint32_t)resid, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this, &_th);
}

// AOSP Resources.obtainTypedArray(@ArrayRes int id) — TypedArray view over a
// typed array resource; each index is one element.
std::unique_ptr<TypedArray> Resources::obtainTypedArray(int id) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    size_t count = 0;
    ssize_t block = -1;
    const ResTable_map* map = rt.getBag((uint32_t)id, &count, nullptr, &block);
    if (!map || count == 0) return nullptr;
    std::vector<StyledAttr> styled(count);
    for (size_t i = 0; i < count; i++) {
        styled[i].value = map[i].value;
        styled[i].stringBlock = block;
        styled[i].set = true;
    }
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
}

// --- Resources::Theme (AOSP Resources.Theme; view over ResTable::Theme) ---

void Resources::Theme::applyStyle(int resId, bool force) {
    if (mEngine) static_cast<ResTable::Theme*>(mEngine)->applyStyle((uint32_t)resId, force);
}

void Resources::Theme::setTo(const Theme& other) {
    if (mEngine == nullptr || other.mEngine == nullptr) return;
    static_cast<ResTable::Theme*>(mEngine)->setTo(
            *static_cast<const ResTable::Theme*>(other.mEngine));
}

bool Resources::Theme::resolveAttribute(int resId, TypedValue* out, bool resolveRefs) const {
    if (mEngine == nullptr || out == nullptr) return false;
    Res_value v;
    if (!static_cast<const ResTable::Theme*>(mEngine)->resolveAttribute(
            (uint32_t)resId, &v, resolveRefs)) return false;
    out->type = v.dataType;
    out->data = v.data;
    // AOSP TypedValue.resourceId: a non-resolved reference's data IS the
    // referenced resource id — TypedArray's ?attr branches depend on it.
    out->resourceId = (v.dataType == Res_value::TYPE_REFERENCE
                       || v.dataType == Res_value::TYPE_DYNAMIC_REFERENCE) ? v.data : 0;
    return true;
}

// AOSP Theme.resolveAttributes(@Nullable int[] themeAttrs, int[] attrs):
// re-resolve the ?attr ids recorded by TypedArray.extractThemeAttrs() through
// this theme, shaped like the original styleable (slot i of themeAttrs pairs
// with attrs[i]; 0 slots stay unset). The applyTheme() re-resolution engine.
std::unique_ptr<TypedArray> Resources::Theme::resolveAttributes(
        const std::vector<int>& themeAttrs, const uint32_t* attrs) const {
    if (mEngine == nullptr) return nullptr;
    const ResTable::Theme* engine = static_cast<const ResTable::Theme*>(mEngine);
    const ResTable& rt = engine->getResTable();
    size_t count = 0; while (attrs[count]) ++count;   // sentinel-terminated
    if (themeAttrs.size() < count) count = themeAttrs.size();
    std::vector<StyledAttr> styled(count);
    for (size_t i = 0; i < count; i++) {
        if (themeAttrs[i] == 0) continue;
        Res_value rv;
        if (engine->resolveAttribute((uint32_t)themeAttrs[i], &rv, true)) {
            styled[i].value = rv;
            styled[i].set = true;
        }
    }
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr,
                                        mRes.getDisplayMetrics().density, &mRes, this);
}

AssetManager* Resources::Theme::getAssets() const {
    return mRes.getAssets();
}

// AOSP Resources.Theme.getDrawable(id) = Resources.getDrawable(id, this) —
// the load is resolved through THIS theme (themed cache + CSL inflation).
// AOSP Resources.Theme.getDrawable(id) = Resources.getDrawable(id, this).
Drawable* Resources::Theme::getDrawable(int id) const {
    return mRes.getDrawable(id, this);
}

// AOSP Resources.Theme.getColor(id) = Resources.getColor(id, this).
int Resources::Theme::getColor(int id) const {
    return mRes.getColor(id, this);
}

// AOSP Resources.Theme.obtainStyledAttributes(AttributeSet, int[],
// defStyleAttr, defStyleRes) — same resolution ladder as Resources::
// obtainStyledAttributes, but against THIS theme's engine (a Theme obtained
// from a different Context/Assets resolves independently).
std::unique_ptr<TypedArray> Resources::Theme::obtainStyledAttributes(const AttributeSet* set,
        const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    if (mEngine == nullptr) return nullptr;
    const ResTable::Theme* theme = static_cast<const ResTable::Theme*>(mEngine);
    const ResTable& rt = theme->getResTable();
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);

    if (set != nullptr) {
        const XmlPullParser* parser = dynamic_cast<const XmlPullParser*>(set);
        if (parser && parser->isBinaryAXML()) {
            const ResXMLTree* xml = static_cast<const ResXMLTree*>(parser->getBinaryAXMLTree());
            if (xml) {
                cdroid::obtainStyledAttributes(*xml, rt, theme, attrs,
                                               (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
                return std::make_unique<TypedArray>(rt, std::move(styled), xml,
                        mRes.getDisplayMetrics().density, &mRes, this);
            }
        }
    }
    cdroid::obtainStyledAttributes(rt, theme, attrs,
                                   (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr,
            mRes.getDisplayMetrics().density, &mRes, this);
}

std::unique_ptr<TypedArray> Resources::Theme::obtainStyledAttributes(const AttributeSet* set,
        const uint32_t* attrs) const {
    return obtainStyledAttributes(set, attrs, 0, 0);
}

std::unique_ptr<TypedArray> Resources::Theme::obtainStyledAttributes(const uint32_t* attrs) const {
    return obtainStyledAttributes(nullptr, attrs, 0, 0);
}

std::unique_ptr<TypedArray> Resources::Theme::obtainStyledAttributes(int resid, const uint32_t* attrs) const {
    return obtainStyledAttributes(nullptr, attrs, 0, resid);
}

// AOSP Resources.newTheme(): a fresh empty theme owning its own engine over
// this Resources' table. getTheme()-style views borrow the Context's engine;
// this one lives as long as the returned Theme (shared ownership on copy).
Resources::Theme Resources::newTheme() {
    const ResTable& rt = getAssets()->getResources(false);
    std::shared_ptr<ResTable::Theme> engine = std::make_shared<ResTable::Theme>(rt);
    Theme theme(*this, engine.get());
    theme.mOwned = engine;
    return theme;
}

// --- AOSP @hide face ---

std::vector<uint32_t> Resources::Theme::getAllAttributes() const {
    std::vector<uint32_t> out;
    if (mEngine) static_cast<const ResTable::Theme*>(mEngine)->getAllAttributes(out);
    return out;
}

int Resources::Theme::getChangingConfigurations() const {
    return mEngine ? (int)static_cast<const ResTable::Theme*>(mEngine)->getChangingConfigurations() : 0;
}

void Resources::Theme::rebase() {
    if (mEngine) static_cast<ResTable::Theme*>(mEngine)->rebase();
}

void Resources::Theme::dump(const char* tag, const char* prefix) const {
    if (mEngine == nullptr) return;
    const ResTable::Theme* theme = static_cast<const ResTable::Theme*>(mEngine);
    std::vector<uint32_t> attrs;
    theme->getAllAttributes(attrs);
    LOGD("%s%sTheme %p: %zu attributes", prefix, tag, mEngine, attrs.size());
    for (uint32_t attr : attrs) {
        TypedValue v;
        if (resolveAttribute((int)attr, &v, false)) {
            LOGD("%s  attr 0x%08x: type=0x%x data=0x%x", prefix, attr, v.type, v.data);
        }
    }
}

} // namespace cdroid
