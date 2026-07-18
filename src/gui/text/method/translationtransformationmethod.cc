/*
 * Skeleton of Android android.text.method.TranslationTransformationMethod (Apache 2.0, @hide).
 * DEFERRED — see header. The translation framework is not ported, so this is a passthrough.
 */
#include <text/method/translationtransformationmethod.h>
#include <text/spannablestring.h>

namespace cdroid {

TranslationTransformationMethod::TranslationTransformationMethod(
        void* response, TransformationMethod* originalMethod)
    : mTranslationResponse(response), mOriginalTranslationMethod(originalMethod) {
}

CharSequence* TranslationTransformationMethod::getTransformation(CharSequence& source, View& view) {
    // TODO(translation): once android.view.translation is ported, pull the translated text
    // from mTranslationResponse (when length changes are allowed) and return it. For now,
    // chain to the original method if any, else pass the source through (ownership-safe copy).
    if (mOriginalTranslationMethod != nullptr) {
        return mOriginalTranslationMethod->getTransformation(source, view);
    }
    return new SpannedString(source.toUTF16());
}

} // namespace cdroid
