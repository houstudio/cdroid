// cdroid::Resources — forwarding to the aggregated ResourcesImpl + own GUI
// factories + obtainStyledAttributes/obtainTypedArray. resourcesimpl.h is
// included HERE (not in resources.h) so the heavy androidfw headers stay hidden
// from the many files that include resources.h via context.h.

#include "resources.h"
#include "context.h"
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

const ResTable_config& Resources::getConfiguration() const {
    return mImpl->getConfiguration();
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
std::shared_ptr<ComplexColor> Resources::loadComplexColor(int id) const {
    return mImpl->loadComplexColor(id);
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

cdroid::Drawable* Resources::getDrawable(int id) const {
    return mImpl->getDrawable(id, 0);
}

cdroid::Drawable* Resources::getDrawableForDensity(int id, int density) const {
    return mImpl->getDrawableForDensity(id, density);
}

std::shared_ptr<ColorStateList> Resources::getColorStateList(int id) const {
    return mImpl->getColorStateList(id);
}

// ===========================================================================
// AOSP Resources.obtainStyledAttributes(...)
// ===========================================================================

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet* set,
         const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    Resources::Theme _th = mCtx->getTheme(); ResTable::Theme* theme = static_cast<ResTable::Theme*>(_th._engineHandle());
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
                return std::make_unique<TypedArray>(rt, std::move(styled), xml, getDisplayMetrics().density, this);
            }
        }
        const int styleResId = set->getStyleResourceId();
        if (styleResId != 0) {
            cdroid::obtainStyledAttributes(rt, theme, attrs,
                                           (uint32_t)defStyleAttr, (uint32_t)styleResId, styled.data());
            return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
        }
    }
    cdroid::obtainStyledAttributes(rt, theme, attrs,
                                   (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
}

// Convenience: AttributeSet& → AttributeSet* (for AOSP callers passing the reference).
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet& set,
        const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    return obtainStyledAttributes(&set, attrs, defStyleAttr, defStyleRes);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const uint32_t* attrs) const {
    return obtainStyledAttributes(0, attrs);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(int resid, const uint32_t* attrs) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    Resources::Theme _th = mCtx->getTheme(); ResTable::Theme* theme = static_cast<ResTable::Theme*>(_th._engineHandle());
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);
    cdroid::obtainStyledAttributes(rt, theme, attrs, 0, (uint32_t)resid, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
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

bool Resources::Theme::resolveAttribute(int resId, TypedValue* out, bool resolveRefs) const {
    if (mEngine == nullptr || out == nullptr) return false;
    Res_value v;
    if (!static_cast<const ResTable::Theme*>(mEngine)->resolveAttribute(
            (uint32_t)resId, &v, resolveRefs)) return false;
    out->type = v.dataType;
    out->data = v.data;
    return true;
}

} // namespace cdroid
