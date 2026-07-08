/*
 * Skeleton of Android android.text.method.InsertModeTransformationMethod (Apache 2.0, @hide).
 * DEFERRED — see header. getTransformation delegates to the old method (if any) and
 * otherwise returns the source unchanged.
 */
#include <text/method/insertmodetransformationmethod.h>
#include <text/spannablestring.h>  // SpannedString for an ownership-safe passthrough copy

namespace cdroid {

InsertModeTransformationMethod::InsertModeTransformationMethod(
        int offset, bool singleLine, TransformationMethod* oldMethod)
    : mStart(offset), mEnd(offset), mSingleLine(singleLine), mOldTransformationMethod(oldMethod) {
}

CharSequence* InsertModeTransformationMethod::getTransformation(CharSequence& source, View& view) {
    // TODO(insert-mode): insert the placeholder at mEnd, build the highlight range, and
    // draw via a ReplacementSpan. For now, chain to the old method (if any) and pass through.
    if (mOldTransformationMethod != nullptr) {
        return mOldTransformationMethod->getTransformation(source, view);
    }
    // Ownership-safe passthrough: materialize a copy so the host TextView can delete it.
    return new SpannedString(source.toU16String());
}

} // namespace cdroid
