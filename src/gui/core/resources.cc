// cdroid::Resources — GUI factory overrides. The value/meta methods are all
// inherited from ResourcesImpl (see androidfw/resources.{h,cc}); these two
// overrides bridge the ID-based lookup to cdroid's existing string-based
// Drawable / ColorStateList inflation.

#include "resources.h"
#include "context.h"               // cdroid::Context
#include <drawable/drawable.h>     // cdroid::Drawable
#include <drawable/colorstatelist.h>  // cdroid::ColorStateList (+ RefPtr)

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

} // namespace cdroid
