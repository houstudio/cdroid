/*
 * Skeleton of Android android.text.method.TranslationTransformationMethod (Apache 2.0, @hide).
 *
 * STUB / DEFERRED: transforms source text into a translated string produced by the
 * android.view.translation framework (ViewTranslationResponse / TranslationResponseValue /
 * ViewTranslationRequest). NONE of that framework is ported to CDROID, so the translated
 * text cannot be obtained; getTransformation currently passes the source through unchanged.
 *
 * The ctor's first param is therefore an opaque `void*` standing in for the (unported)
 * ViewTranslationResponse. Replace it with the real type once the translation framework
 * is ported.
 */
#ifndef CDROID_TRANSLATION_TRANSFORMATION_METHOD_H
#define CDROID_TRANSLATION_TRANSFORMATION_METHOD_H

#include <text/method/transformationmethod.h>

namespace cdroid {

class TranslationTransformationMethod : public TransformationMethod2 {
public:
    // `response` is opaque (the unported ViewTranslationResponse).
    TranslationTransformationMethod(void* response, TransformationMethod* originalMethod);

    CharSequence* getTransformation(CharSequence& source, View& view) override;
    void onFocusChanged(View& view, CharSequence& sourceText, bool focused,
            int direction, const Rect& previouslyFocusedRect) override {}

    TransformationMethod* getOriginalTransformationMethod() const { return mOriginalTranslationMethod; }
    void* getViewTranslationResponse() const { return mTranslationResponse; }

private:
    void* mTranslationResponse;             // opaque (ViewTranslationResponse, unported)
    TransformationMethod* mOriginalTranslationMethod;
};

} // namespace cdroid

#endif // CDROID_TRANSLATION_TRANSFORMATION_METHOD_H
