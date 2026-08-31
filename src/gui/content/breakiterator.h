/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CDROID_BREAK_ITERATOR_H__
#define __CDROID_BREAK_ITERATOR_H__
#include <string>
#include <content/Locale.h>

namespace cdroid{

/**
 * Line-for-line port of java.text.BreakIterator, exposing the surface
 * android.view.AccessibilityIterators consumes: the locale factories plus
 * the boundary walks. Text is UTF-16 and all offsets are UTF-16 code-unit
 * indices, exactly like the Java original (String coordinates).
 *
 * Backends (deliberately ICU-free at this layer — whichever ICU minikin was
 * built with is minikin's own business):
 *   - character instance → minikin GraphemeBreak (UTR#29 grapheme clusters)
 *   - word instance      → android.text.method.WordIterator (UAX#29 words)
 *   - line/sentence      → whole-text boundaries only (no consumer yet; the
 *                          a11y LINE/PAGE granularities go through Layout)
 *
 * Java's abstract class hands out GC'd instances; here the factories return
 * owned pointers the caller deletes.
 */
class BreakIterator{
public:
    /** DONE is returned by previous()/next()/following()/preceding() when
     *  the boundary walk ran off either end of the text. */
    static constexpr int DONE = -1;

    static BreakIterator* getCharacterInstance(const Locale& locale);
    static BreakIterator* getWordInstance(const Locale& locale);
    static BreakIterator* getLineInstance(const Locale& locale);
    static BreakIterator* getSentenceInstance(const Locale& locale);

    BreakIterator(const BreakIterator&) = delete;
    BreakIterator& operator=(const BreakIterator&) = delete;
    ~BreakIterator();

    /** Points the iterator at a new text ( setText(String) ). */
    void setText(const std::u16string& text);

    /** First boundary in the text. */
    int first();
    /** Last boundary in the text. */
    int last();
    /** Boundary following the current position, or DONE. */
    int next();
    /** Boundary preceding the current position, or DONE. */
    int previous();
    /** First boundary following the given offset, or DONE. */
    int following(int offset);
    /** Last boundary preceding the given offset, or DONE. */
    int preceding(int offset);
    /** Character offset of the current boundary. */
    int current();
    /** True if the given offset is a boundary. */
    bool isBoundary(int offset);
private:
    BreakIterator(int kind, const Locale& locale);

    // Kind-specific state (u16 buffer for the grapheme walker, WordIterator
    // for words), kept out of the header so minikin/method types do not
    // leak to includers.
    struct Impl;
    Impl* mImpl;
};

}  // namespace cdroid
#endif  // __CDROID_BREAK_ITERATOR_H__
