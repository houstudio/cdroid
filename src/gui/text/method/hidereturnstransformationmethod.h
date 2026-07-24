/*
 * Ported from Android android.text.method.HideReturnsTransformationMethod (Apache 2.0).
 *
 * Displays carriage returns (\r) as zero-width non-breaking space (﻿),
 * i.e. hides them. Extends ReplacementTransformationMethod.
 */
#ifndef CDROID_HIDERETURNS_TRANSFORMATION_METHOD_H
#define CDROID_HIDERETURNS_TRANSFORMATION_METHOD_H

#include <text/method/replacementtransformationmethod.h>

namespace cdroid {

class HideReturnsTransformationMethod : public ReplacementTransformationMethod {
public:
    // \r -> ﻿.
    std::u16string getOriginal() const override { return u"\r"; }
    std::u16string getReplacement() const override { return u"﻿"; }

    static HideReturnsTransformationMethod* getInstance();
private:
    static HideReturnsTransformationMethod* sInstance;
};

} // namespace cdroid

#endif // CDROID_HIDERETURNS_TRANSFORMATION_METHOD_H
