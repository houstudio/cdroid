#include <touchinterceptor.h>

#include <view/layoutinflater.h>
#include <widget/internal_R.h>

namespace cdroid {
namespace music {

// Layout tag is "com.android.music.TouchInterceptor"; the inflater registry
// keys on the last path segment ("TouchInterceptor").
DECLARE_WIDGET3(TouchInterceptor, TouchInterceptor, internal::R::attr::listViewStyle)

} // namespace music
} // namespace cdroid
