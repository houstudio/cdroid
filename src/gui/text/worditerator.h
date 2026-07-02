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
#ifndef __WORD_ITERATOR_H__
#define __WORD_ITERATOR_H__

#include <string>
#include <text/selection.h>   // Selection::PositionIterator (Android: implements Selection.PositionIterator)

namespace cdroid {

class CharSequence;

/**
 * Walks through cursor positions at word boundaries. Internally uses
 * icu::BreakIterator::createWordInstance (Android: BreakIterator.getWordInstance),
 * and caches the CharSequence for performance reasons.
 *
 * Also provides methods to determine word boundaries.
 *
 * This is a line-for-line C++ port of android.text.method.WordIterator, kept
 * parallel to the Android source so it can be diffed/maintained against it.
 * (android.text.method has no CDROID equivalent yet; this is the first piece
 * of that package to be ported.)
 */
class WordIterator : public Selection::PositionIterator {
public:
    // Size of the window for the word iterator, should be greater than the
    // longest word's length.
    static constexpr int WINDOW_WIDTH = 50;

    // Equivalent of android.icu.text.BreakIterator.DONE.
    static constexpr int DONE = -1;

    /** Constructs a WordIterator using the default locale. */
    WordIterator();
    /** Constructs a new WordIterator for the specified ICU locale name. */
    explicit WordIterator(const std::string& locale);

    WordIterator(const WordIterator&) = delete;
    WordIterator& operator=(const WordIterator&) = delete;
    ~WordIterator();

    /** Sets the char sequence to analyze over the [start, end] window. */
    void setCharSequence(const CharSequence* charSequence, int start, int end);

    int preceding(int offset) const override;
    int following(int offset) const override;
    bool isBoundary(int offset)const;

    /** Position of next boundary after the given offset, or DONE. */
    int nextBoundary(int offset)const;
    /** Position of boundary preceding the given offset, or DONE. */
    int prevBoundary(int offset)const;

    /** First-character index of the word containing offset, or DONE. */
    int getBeginning(int offset)const;
    /** Last-character-plus-one index of the word containing offset, or DONE. */
    int getEnd(int offset)const;

    int getPrevWordBeginningOnTwoWordsBoundary(int offset)const;
    int getNextWordEndOnTwoWordBoundary(int offset)const;

    int getPunctuationBeginning(int offset)const;
    int getPunctuationEnd(int offset)const;

    bool isOnPunctuation(int offset)const;
    bool isAfterPunctuation(int offset)const;
    static bool isMidWordPunctuation(int codePoint);
private:
    int getBeginning(int offset, bool getPrevWordBeginningOnTwoWordsBoundary)const;
    int getEnd(int offset, bool getNextWordEndOnTwoWordBoundary)const;

    bool isAfterLetterOrDigit(int offset)const;
    bool isOnLetterOrDigit(int offset)const;

    bool isPunctuationStartBoundary(int offset)const;
    bool isPunctuationEndBoundary(int offset)const;

    void checkOffsetIsValid(int offset)const;

    const CharSequence* mCharSeq = nullptr;
    int mStart = 0;
    int mEnd = 0;

    // Holds the icu::BreakIterator + the windowed text it iterates, kept out of
    // the header so <unicode/brkiter.h> does not leak to includers.
    struct Impl;
    Impl* mImpl;
};

}  // namespace cdroid

#endif  // __WORD_ITERATOR_H__
