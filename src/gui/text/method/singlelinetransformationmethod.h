/*
 * Ported from Android android.text.method.SingleLineTransformationMethod (Apache 2.0).
 *
 * Displays newline (\n) as space (no line break) and carriage return (\r) as
 * zero-width non-breaking space. Extends ReplacementTransformationMethod.
 */
#ifndef CDROID_SINGLELINE_TRANSFORMATION_METHOD_H
#define CDROID_SINGLELINE_TRANSFORMATION_METHOD_H

#include <text/method/replacementtransformationmethod.h>

namespace cdroid {

class SingleLineTransformationMethod : public ReplacementTransformationMethod {
public:
    // \n -> ' ', \r -> ﻿.
    std::u16string getOriginal() const override { return u"\n\r"; }
    std::u16string getReplacement() const override { return u" ﻿"; }

    static SingleLineTransformationMethod* getInstance();
private:
    static SingleLineTransformationMethod* sInstance;
};

} // namespace cdroid

#endif // CDROID_SINGLELINE_TRANSFORMATION_METHOD_H
