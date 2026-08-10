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
#include "androidfw/resourcesimpl.h"   // ResourcesImpl

namespace cdroid {

class Context;
class Drawable;
class ColorStateList;

class Resources : public ResourcesImpl {
public:
    // `am` is NOT owned (must outlive this Resources); `ctx` is the bridge to the
    // existing string-based Drawable/ColorStateList inflation.
    Resources(AssetManager* am, cdroid::Context* ctx);

    cdroid::Drawable*       getDrawable(int id, int density) const override;
    cdroid::ColorStateList* getColorStateList(int id) const override;
    // getFont is inherited (returns nullptr); font resource resolution is deferred.

private:
    cdroid::Context* mCtx;
};

} // namespace cdroid
#endif // __RESOURCES_CDROID_H__
