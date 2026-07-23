/*
 * Skeleton of Android android.text.method.InsertModeTransformationMethod (Apache 2.0, @hide).
 *
 * STUB / DEFERRED: this is the transformation method for handwriting insert mode —
 * it inserts a placeholder at an offset and tracks a highlight range (implements
 * TransformationMethod + TextWatcher, draws via a ReplacementSpan). The full
 * implementation depends on ReplacementSpan drawing + TextView insert-mode
 * integration that CDROID hasn't wired yet. The class is declared for API parity;
 * getTransformation currently passes the source through unchanged. Flesh out when
 * handwriting insert mode is needed.
 */
#ifndef CDROID_INSERTMODE_TRANSFORMATION_METHOD_H
#define CDROID_INSERTMODE_TRANSFORMATION_METHOD_H

#include <text/method/transformationmethod.h>
#include <text/textwatcher.h>

namespace cdroid {

class InsertModeTransformationMethod
        : public TransformationMethod, public TextWatcher {
public:
    InsertModeTransformationMethod(int offset, bool singleLine, TransformationMethod* oldMethod);

    CharSequence* getTransformation(CharSequence& source, View& view) override;

    void onFocusChanged(View& view, CharSequence& sourceText, bool focused,
            int direction, const Rect& previouslyFocusedRect) override {}

    int getStart() const { return mStart; }
    int getEnd() const { return mEnd; }

private:
    int mStart;
    int mEnd;
    bool mSingleLine;
    TransformationMethod* mOldTransformationMethod;
};

} // namespace cdroid

#endif // CDROID_INSERTMODE_TRANSFORMATION_METHOD_H
