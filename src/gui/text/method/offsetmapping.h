/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.method.OffsetMapping.
 *
 * The interface for the index mapping information of a transformed text returned
 * by TransformationMethod. Mainly used to support a TransformationMethod that
 * alters the text length (e.g. password dots, AllCaps). The transformed text
 * CharSequence implements this so Editor/movement can map offsets between the
 * original and transformed text.
 *********************************************************************************/
#ifndef __OFFSET_MAPPING_H__
#define __OFFSET_MAPPING_H__
#include <cstdint>

namespace cdroid {

/** Port of android.text.method.OffsetMapping. */
class OffsetMapping {
public:
    /** The mapping strategy for a character offset. */
    static constexpr int MAP_STRATEGY_CHARACTER = 0;
    /** The mapping strategy for a cursor position. */
    static constexpr int MAP_STRATEGY_CURSOR = 1;

    /**
     * The class that stores the text update information that from index `where`,
     * `after` characters will replace the old text that has length `before`.
     */
    class TextUpdate {
    public:
        /** The start index of the text update range, inclusive. */
        int where;
        /** The length of the replaced old text. */
        int before;
        /** The length of the new text that replaces the old text. */
        int after;

        TextUpdate(int where, int before, int after)
            : where(where), before(before), after(after) {}
    };

    virtual ~OffsetMapping() = default;

    /** Map an offset at original text to the offset at transformed text. */
    virtual int originalToTransformed(int offset, int strategy) = 0;

    /** Map an offset at transformed text to the offset at original text. */
    virtual int transformedToOriginal(int offset, int strategy) = 0;

    /**
     * Map a text update in the original text to an update of the transformed text.
     * Always called before the original text is changed; used for incremental layout.
     */
    virtual void originalToTransformed(TextUpdate& textUpdate) = 0;
};

}  // namespace cdroid
#endif  // __OFFSET_MAPPING_H__
