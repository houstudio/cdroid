/*
 * Ported from Android android.text.method.HideReturnsTransformationMethod (Apache 2.0).
 */
#include <text/method/hidereturnstransformationmethod.h>

namespace cdroid {

HideReturnsTransformationMethod* HideReturnsTransformationMethod::sInstance = nullptr;

HideReturnsTransformationMethod* HideReturnsTransformationMethod::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new HideReturnsTransformationMethod();
    }
    return sInstance;
}

} // namespace cdroid
