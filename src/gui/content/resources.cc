// cdroid::Resources — forwarding to the aggregated ResourcesImpl + own GUI
// factories + obtainStyledAttributes/obtainTypedArray. resourcesimpl.h is
// included HERE (not in resources.h) so the heavy androidfw headers stay hidden
// from the many files that include resources.h via context.h.

#include "resources.h"
#include <core/context.h>
#include <core/app.h>                  // App::getInstance() (getSystem)
#include <core/attributeset.h>
#include <content/typedarray.h>
#include <core/xmlpullparser.h>
#include <content/xmlblock.h>            // XmlBlock::Parser (the binary downcast below)
#include <content/androidfw/assetmanager2.h>   // AM2 engine + cdroid::Theme
#include <content/androidfw/attributeresolution.h>  // ApplyStyle/ResolveAttrs/RetrieveAttributes
#include <content/resourcesimpl.h>   // ResourcesImpl (aggregated)
#include <content/assetmanager.h>      // AssetManager (getAssets()->getResources)
#include <content/typedvalue.h>   // TypedValue (getValue/getColorStateList)
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

Context* Resources::getContext() const {
    return mImpl ? mImpl->getContext() : nullptr;
}

std::unique_ptr<XmlResourceParser> Resources::getXml(int id) const {
    return mImpl->loadXmlResourceParser(id);
}

std::unique_ptr<XmlResourceParser> Resources::getLayout(int id) const {
    // AOSP getLayout = loadXmlPullParser under a layout-flavored name.
    return mImpl->loadXmlResourceParser(id);
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
            // Positional form "%1$s" (how aapt-stored plurals/strings spell
            // it): the digits before '$' pick the argument, the letter after
            // is the (string-substituted) conversion.
            size_t j = i + 1;
            while (j < fmt.size() && fmt[j] >= '0' && fmt[j] <= '9') j++;
            if (j > i + 1 && j < fmt.size() && fmt[j] == '$' && j + 1 < fmt.size()
                    && (fmt[j + 1] == 's' || fmt[j + 1] == 'd')) {
                const size_t idx = (size_t)atoi(fmt.substr(i + 1, j - i - 1).c_str());
                if (idx >= 1 && idx <= args.size()) {
                    out += args[idx - 1];
                    i = j + 1;
                    continue;
                }
            }
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



// ===========================================================================
// AOSP Resources.obtainStyledAttributes(...)
// ===========================================================================

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet* set,
         const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    // The resolver lives once, in Theme::obtainStyledAttributes (same theme
    // engine, AM2, density and TypedArray wiring — this was a verbatim second
    // copy that also handed the TypedArray the address of a LOCAL Theme
    // copy; the Theme version passes the persistent object). mCtx's theme is
    // exactly the engine the copy below used to extract.
    if (mCtx == nullptr) return nullptr;
    return mCtx->getTheme().obtainStyledAttributes(set, attrs, defStyleAttr, defStyleRes);
}

// Convenience: AttributeSet& → AttributeSet* (for AOSP callers passing the reference).
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet& set,
        const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    return obtainStyledAttributes(&set, attrs, defStyleAttr, defStyleRes);
}

// --- Animator caches (forwarding to ResourcesImpl; see resourcesimpl.cc) ---
Animator* Resources::obtainCachedAnimator(int id, const void* themeEngine) const {
    return mImpl->obtainCachedAnimator(id, themeEngine);
}
void Resources::cacheAnimator(int id, const void* themeEngine,
        const std::shared_ptr<ConstantState<Animator*>>& cs) const {
    mImpl->cacheAnimator(id, themeEngine, cs);
}
StateListAnimator* Resources::obtainCachedStateListAnimator(int id, const void* themeEngine) const {
    return mImpl->obtainCachedStateListAnimator(id, themeEngine);
}
void Resources::cacheStateListAnimator(int id, const void* themeEngine,
        const std::shared_ptr<ConstantState<StateListAnimator*>>& cs) const {
    mImpl->cacheStateListAnimator(id, themeEngine, cs);
}

// AOSP Resources.obtainAttributes(set, attrs): theme-less — only the
// attributes explicitly set in the XML, no style/theme resolution
// (AOSP ResourcesImpl.obtainStyledAttributes(set, attrs, theme=null)).
std::unique_ptr<TypedArray> Resources::obtainAttributes(const AttributeSet* set, const uint32_t* attrs) const {
    if (mCtx == nullptr || set == nullptr) return nullptr;
    AssetManager2& am2 = getAssets()->getAssetManager2();
    size_t count = 0;
    while (attrs[count]) ++count;   // sentinel-terminated
    const XmlBlock::Parser* parser = dynamic_cast<const XmlBlock::Parser*>(set);
    const ResXMLTree* xml = parser ? parser->getResXMLTree() : nullptr;
    std::vector<StyledAttr> styled(count);
    if (xml) {
        // AOSP nativeRetrieveAttributes: only the XML's own attributes, no
        // style/theme resolution.
        std::vector<uint32_t> values(count * STYLE_NUM_ENTRIES);
        RetrieveAttributes(&am2, xml, attrs, count, values.data(), nullptr);
        styledAttrsFromBlocks(values.data(), count, styled.data());
    }
    return std::make_unique<TypedArray>(&am2, std::move(styled), xml,
                                        getDisplayMetrics().density, this, /*theme*/nullptr);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const uint32_t* attrs) const {
    return obtainStyledAttributes(0, attrs);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(int resid, const uint32_t* attrs) const {
    if (mCtx == nullptr) return nullptr;
    AssetManager2& am2 = getAssets()->getAssetManager2();
    Resources::Theme _th = mCtx->getTheme();
    cdroid::Theme* theme = static_cast<cdroid::Theme*>(_th._engineHandle());
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<uint32_t> values(count * STYLE_NUM_ENTRIES);
    ResolveAttrs(theme, 0, (uint32_t)resid, nullptr, 0, attrs, count, values.data(), nullptr);
    std::vector<StyledAttr> styled(count);
    styledAttrsFromBlocks(values.data(), count, styled.data());
    return std::make_unique<TypedArray>(&am2, std::move(styled), nullptr, getDisplayMetrics().density, this, &_th);
}

// AOSP Resources.obtainTypedArray(@ArrayRes int id) — TypedArray view over a
// typed array resource; each index is one element.
std::unique_ptr<TypedArray> Resources::obtainTypedArray(int id) const {
    if (mCtx == nullptr) return nullptr;
    AssetManager2& am2 = getAssets()->getAssetManager2();
    auto bagResult = am2.GetBag((uint32_t)id);
    if (!bagResult.has_value() || (*bagResult)->entry_count == 0) return nullptr;
    const ResolvedBag* bag = *bagResult;
    const size_t count = bag->entry_count;
    std::vector<StyledAttr> styled(count);
    for (size_t i = 0; i < count; i++) {
        styled[i].value = bag->entries[i].value;
        styled[i].stringBlock = bag->entries[i].cookie;   // string pool cookie
        styled[i].set = true;
    }
    return std::make_unique<TypedArray>(&am2, std::move(styled), nullptr, getDisplayMetrics().density, this);
}

// --- Resources::Theme (AOSP Resources.Theme; view over the AM2 cdroid::Theme) ---

void Resources::Theme::applyStyle(int resId, bool force) {
    if (mEngine) {
        if (!static_cast<cdroid::Theme*>(mEngine)->ApplyStyle((uint32_t)resId, force).has_value()) {
            LOGW("Theme::applyStyle(resId=0x%08x) failed", resId);
        }
    }
}

void Resources::Theme::setTo(const Theme& other) {
    if (mEngine == nullptr || other.mEngine == nullptr) return;
    if (!static_cast<cdroid::Theme*>(mEngine)->SetTo(
            *static_cast<const cdroid::Theme*>(other.mEngine)).has_value()) {
        LOGW("Theme::setTo failed");
    }
}

bool Resources::Theme::resolveAttribute(int resId, TypedValue* out, bool resolveRefs) const {
    if (mEngine == nullptr || out == nullptr) return false;
    const cdroid::Theme* theme = static_cast<const cdroid::Theme*>(mEngine);
    auto value = theme->GetAttribute((uint32_t)resId);
    if (!value.has_value()) return false;
    if (resolveRefs) {
        // Flatten ?attr / @ref chains (AOSP Theme.resolveAttribute with
        // resolveRefs=true).
        if (!theme->ResolveAttributeReference(*value).has_value() &&
            value->type == Res_value::TYPE_NULL) {
            return false;
        }
    }
    out->type = value->type;
    out->data = value->data;
    // AOSP TypedValue.resourceId: the reference this value came from. With
    // resolveRefs the chain flattens (a color-selector reference becomes its
    // file-path string) and callers like TypedArray still need the id to load
    // it — keep the LAST traversed reference (SelectedValue.resid), falling
    // back to a plain reference's data.
    out->resourceId = (resolveRefs && value->resid != 0) ? value->resid
            : ((value->type == Res_value::TYPE_REFERENCE
                || value->type == Res_value::TYPE_DYNAMIC_REFERENCE) ? value->data : 0);
    return true;
}

// AOSP Theme.resolveAttributes(@Nullable int[] themeAttrs, int[] attrs):
// re-resolve the ?attr ids recorded by TypedArray.extractThemeAttrs() through
// this theme, shaped like the original styleable (slot i of themeAttrs pairs
// with attrs[i]; 0 slots stay unset). The applyTheme() re-resolution engine.
std::unique_ptr<TypedArray> Resources::Theme::resolveAttributes(
        const std::vector<int>& themeAttrs, const uint32_t* attrs) const {
    if (mEngine == nullptr) return nullptr;
    const cdroid::Theme* engine = static_cast<const cdroid::Theme*>(mEngine);
    const AssetManager2* am2p = engine->GetAssetManager();
    size_t count = 0; while (attrs[count]) ++count;   // sentinel-terminated
    if (themeAttrs.size() < count) count = themeAttrs.size();
    // AOSP nativeResolveAttrs: the recorded ?attr ids ride in as src_values
    // (slot i pairs with attrs[i]; 0 slots stay unset).
    std::vector<uint32_t> src((size_t)count, 0);
    for (size_t i = 0; i < count; i++) src[i] = (uint32_t)themeAttrs[i];
    std::vector<uint32_t> values(count * STYLE_NUM_ENTRIES);
    ResolveAttrs(const_cast<cdroid::Theme*>(engine), 0, 0, src.data(), count,
                 attrs, count, values.data(), nullptr);
    std::vector<StyledAttr> styled(count);
    styledAttrsFromBlocks(values.data(), count, styled.data());
    return std::make_unique<TypedArray>(am2p, std::move(styled), nullptr,
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
    cdroid::Theme* theme = static_cast<cdroid::Theme*>(mEngine);
    AssetManager2& am2 = *theme->GetAssetManager();
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<uint32_t> values(count * STYLE_NUM_ENTRIES);
    std::vector<uint32_t> indices(count + 1);

    if (set != nullptr) {
        // AOSP ResourcesImpl.applyStyle: hard downcast to the binary parser
        // for the parse state; text sets take the string-coercion path below.
        const XmlBlock::Parser* parser = dynamic_cast<const XmlBlock::Parser*>(set);
        if (parser) {
            const ResXMLTree* xml = parser->getResXMLTree();
            if (xml) {
                ApplyStyle(theme, xml, (uint32_t)defStyleAttr, (uint32_t)defStyleRes,
                           attrs, count, values.data(), indices.data());
                std::vector<StyledAttr> styled(count);
                styledAttrsFromBlocks(values.data(), count, styled.data());
                return std::make_unique<TypedArray>(&am2, std::move(styled), xml,
                        mRes.getDisplayMetrics().density, &mRes, this);
            }
        }
    }
    ResolveAttrs(theme, (uint32_t)defStyleAttr, (uint32_t)defStyleRes,
                 nullptr, 0, attrs, count, values.data(), nullptr);
    std::vector<StyledAttr> styled(count);
    styledAttrsFromBlocks(values.data(), count, styled.data());
    return std::make_unique<TypedArray>(&am2, std::move(styled), nullptr,
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
    // AOSP Resources.newTheme(): a fresh engine over this Resources' table.
    AssetManager2& am2 = getAssets()->getAssetManager2();
    std::shared_ptr<cdroid::Theme> engine = am2.NewTheme();
    Theme theme(*this, engine.get());
    theme.mOwned = engine;
    return theme;
}

// --- AOSP @hide face ---

std::vector<uint32_t> Resources::Theme::getAllAttributes() const {
    std::vector<uint32_t> out;
    if (mEngine) static_cast<const cdroid::Theme*>(mEngine)->GetAllAttributes(out);
    return out;
}

int Resources::Theme::getChangingConfigurations() const {
    return mEngine ? (int)static_cast<const cdroid::Theme*>(mEngine)->GetChangingConfigurations() : 0;
}

void Resources::Theme::rebase() {
    // The AM2 engine keeps no setTo snapshot to roll back to (the retired
    // legacy engine's rebase feature); no in-tree callers — documented no-op.
    if (mEngine) LOGW("Theme::rebase() is a no-op on the AM2 engine");
}

void Resources::Theme::dump(const char* tag, const char* prefix) const {
    if (mEngine == nullptr) return;
    const cdroid::Theme* theme = static_cast<const cdroid::Theme*>(mEngine);
    std::vector<uint32_t> attrs;
    theme->GetAllAttributes(attrs);
    LOGD("%s%sTheme %p: %zu attributes", prefix, tag, mEngine, attrs.size());
    for (uint32_t attr : attrs) {
        TypedValue v;
        if (resolveAttribute((int)attr, &v, false)) {
            LOGD("%s  attr 0x%08x: type=0x%x data=0x%x", prefix, attr, v.type, v.data);
        }
    }
}

} // namespace cdroid
