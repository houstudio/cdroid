/*
 * Ported from Android android.text.method.SingleLineTransformationMethod (Apache 2.0).
 */
#include <text/method/singlelinetransformationmethod.h>

namespace cdroid {

SingleLineTransformationMethod* SingleLineTransformationMethod::sInstance = nullptr;

SingleLineTransformationMethod* SingleLineTransformationMethod::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new SingleLineTransformationMethod();
    }
    return sInstance;
}

} // namespace cdroid
