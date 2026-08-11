// cdroid::Resources — GUI factory overrides. The value/meta methods are all
// inherited from ResourcesImpl (see androidfw/resources.{h,cc}); these two
// overrides bridge the ID-based lookup to cdroid's existing string-based
// Drawable / ColorStateList inflation.

#include "resources.h"
#include "context.h"               // cdroid::Context
#include <core/attributeset.h>     // AttributeSet
#include <core/typedarray.h>       // TypedArray
#include <core/xmlpullparser.h>    // XmlPullParser (binary AXML detection)
#include <androidfw/restable.h>    // obtainStyledAttributes resolver, ResXMLTree, StyledAttr
#include <drawable/drawable.h>     // cdroid::Drawable
#include <drawable/colorstatelist.h>  // cdroid::ColorStateList (+ RefPtr)
#include <vector>

namespace cdroid {

Resources::Resources(AssetManager* am, cdroid::Context* ctx)
    : ResourcesImpl(am), mCtx(ctx) {
}

cdroid::Drawable* Resources::getDrawable(int id, int /*density*/) const {
    if (mCtx == nullptr) return nullptr;
    std::string ref;
    if (!getResourceName(id, &ref)) return nullptr;   // "pkg:type/key"
    return mCtx->getDrawable(ref);                     // existing inflation (cached)
}

cdroid::ColorStateList* Resources::getColorStateList(int id) const {
    if (mCtx == nullptr) return nullptr;
    std::string ref;
    if (!getResourceName(id, &ref)) return nullptr;
    // Assets caches ColorStateList by name (mStateColors), so the object outlives
    // the returned RefPtr; .get() borrows the cached instance.
    auto csl = mCtx->getColorStateList(ref);
    return csl.get();
}

// AOSP Resources.obtainStyledAttributes(AttributeSet, int[], int, int).
// AttributeSet is nullable (AOSP @Nullable). Binary-AXML element -> the
// ResXMLTree resolver; a runtime-resolved style AttributeSet (widget-from-style)
// -> re-resolve its source style through the arsc theme resolver; null or a
// text-XML element set -> theme + defStyleAttr/defStyleRes only (AOSP
// obtainStyledAttributes(null, attrs, defStyleAttr, defStyleRes)).
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(
    const AttributeSet* set, const uint32_t* attrs, int defStyleAttr, int defStyleRes) const
{
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    ResTable::Theme* theme = &mCtx->getTheme();
    size_t count = 0; while (attrs[count]) count++;   // sentinel-terminated
    std::vector<StyledAttr> styled(count);

    if (set != nullptr) {
        const XmlPullParser* parser = dynamic_cast<const XmlPullParser*>(set);
        if (parser && parser->isBinaryAXML()) {
            const ResXMLTree* xml = static_cast<const ResXMLTree*>(parser->getBinaryAXMLTree());
            if (xml) {
                cdroid::obtainStyledAttributes(*xml, rt, theme, attrs,
                                                (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
                return std::make_unique<TypedArray>(rt, std::move(styled), xml, getDisplayMetrics().density, mCtx);
            }
        }
        // widget-from-style: a runtime-resolved style AttributeSet.
        const int styleResId = set->getStyleResourceId();
        if (styleResId != 0) {
            cdroid::obtainStyledAttributes(rt, theme, attrs,
                                            (uint32_t)defStyleAttr, (uint32_t)styleResId, styled.data());
            return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, mCtx);
        }
    }
    // null AttributeSet (AOSP new View(ctx, null, defStyleAttr)) or text-XML
    // element: resolve attrs against the theme + defStyleAttr/defStyleRes only.
    cdroid::obtainStyledAttributes(rt, theme, attrs,
                                    (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, mCtx);
}

// AOSP Resources.obtainStyledAttributes(int[]) — theme only.
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const uint32_t* attrs) const {
    return obtainStyledAttributes(0, attrs);
}

// AOSP Resources.obtainStyledAttributes(int resid, int[]) — apply a style resId.
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(int resid, const uint32_t* attrs) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    ResTable::Theme* theme = &mCtx->getTheme();
    size_t count = 0; while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);
    cdroid::obtainStyledAttributes(rt, theme, attrs, 0, (uint32_t)resid, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, mCtx);
}

} // namespace cdroid
