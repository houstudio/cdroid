#include <touchinterceptor.h>

#include <view/layoutinflater.h>
#include <widget/internal_R.h>

namespace cdroid {
namespace music {

// Layout tag is "com.android.music.TouchInterceptor": registered under the
// short key and under the upstream FQCN (getInflater resolves dotted tags
// exactly first).
DECLARE_WIDGET2(TouchInterceptor, "TouchInterceptor");

static const int sTouchInterceptorFqcn = (LayoutInflater::registerInflater(
        "com.android.music.TouchInterceptor",
        [](Context* ctx, const AttributeSet& attr) -> View* {
    return new TouchInterceptor(ctx, &attr, internal::R::attr::listViewStyle);
}), 0);

} // namespace music
} // namespace cdroid
