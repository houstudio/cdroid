#include <touchinterceptor.h>

#include <view/layoutinflater.h>
#include <widget/internal_R.h>

namespace cdroid {
namespace music {

// Layout tag is "com.android.music.TouchInterceptor": registered under the
// short key and under the upstream FQCN (getInflater resolves dotted tags
// exactly first).
DECLARE_WIDGET3(TouchInterceptor, TouchInterceptor, internal::R::attr::listViewStyle)

static const int sTouchInterceptorFqcn = (LayoutInflater::registerInflater(
        "com.android.music.TouchInterceptor", internal::R::attr::listViewStyle,
        [](Context* ctx, const AttributeSet& attr) -> View* {
    return new TouchInterceptor(ctx, &attr, internal::R::attr::listViewStyle);
}), 0);

} // namespace music
} // namespace cdroid
