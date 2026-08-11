// cdroid::Resources — the libcdroid Resources that fills in the GUI-object
// factories left stubbed by the isolated ResourcesImpl port. Inherits every
// value/meta/asset method (resolved via AssetManager → ResTable); the
// two GUI overrides delegate to cdroid::Context's existing string-based
// inflation (getResourceName(id) -> "pkg:type/key" -> ctx->getDrawable(...)).
//
// Declaration only; implementations in resources.cc.
#ifndef __RESOURCES_CDROID_H__
#define __RESOURCES_CDROID_H__

#include <string>
#include <memory>
#include <cstdint>
#include "androidfw/resourcesimpl.h"   // ResourcesImpl

namespace cdroid {

class Context;
class Drawable;
class ColorStateList;
class AttributeSet;
class TypedArray;

class Resources : public ResourcesImpl {
public:
    // `am` is NOT owned (must outlive this Resources); `ctx` is the bridge to the
    // existing string-based Drawable/ColorStateList inflation.
    Resources(AssetManager* am, cdroid::Context* ctx);

    cdroid::Drawable*       getDrawable(int id, int density) const override;
    cdroid::ColorStateList* getColorStateList(int id) const override;
    // getFont is inherited (returns nullptr); font resource resolution is deferred.

    // AOSP Resources.obtainStyledAttributes(...) — resolve a styleable attr set
    // against an XML element (binary AXML), the live theme, or a style resId.
    // (AttributeSet, attrs[], defStyleAttr, defStyleRes): the element-attr path;
    // null when the AttributeSet is neither a binary element nor a resolved style.
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet& set,
        const uint32_t* attrs, int defStyleAttr = 0, int defStyleRes = 0) const;
    // (attrs[]): theme only.
    std::unique_ptr<TypedArray> obtainStyledAttributes(const uint32_t* attrs) const;
    // (resid, attrs[]): apply a style resId on top of the theme.
    std::unique_ptr<TypedArray> obtainStyledAttributes(int resid, const uint32_t* attrs) const;

private:
    cdroid::Context* mCtx;
};

} // namespace cdroid
#endif // __RESOURCES_CDROID_H__
