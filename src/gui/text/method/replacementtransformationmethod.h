/*
 * Ported from Android android.text.method.ReplacementTransformationMethod (Apache 2.0).
 *
 * Abstract base for TransformationMethods that display one set of characters
 * (getOriginal) as another (getReplacement). Subclasses (HideReturns,
 * SingleLine) just supply the two arrays. The actual wrapping lives in the .cc
 * (ReplacementCharSequence / SpannedReplacementCharSequence).
 */
#ifndef CDROID_REPLACEMENT_TRANSFORMATION_METHOD_H
#define CDROID_REPLACEMENT_TRANSFORMATION_METHOD_H

#include <text/method/transformationmethod.h>
#include <string>

namespace cdroid {

class ReplacementTransformationMethod : public TransformationMethod {
public:
    // The characters to be replaced when displayed.
    virtual std::u16string getOriginal() const = 0;
    // A parallel array of replacement characters for getOriginal().
    virtual std::u16string getReplacement() const = 0;

    // Wraps `source` so each getOriginal() char reads as its getReplacement()
    // counterpart. Returns an owned CharSequence* (the host TextView deletes it
    // on replace); unlike Android we always wrap rather than return &source.
    CharSequence* getTransformation(CharSequence& source, View& view) override;

    void onFocusChanged(View& /*view*/, CharSequence& /*sourceText*/, bool /*focused*/,
            int /*direction*/, const Rect& /*previouslyFocusedRect*/) override {
        // This callback isn't used.
    }
};

} // namespace cdroid

#endif // CDROID_REPLACEMENT_TRANSFORMATION_METHOD_H
