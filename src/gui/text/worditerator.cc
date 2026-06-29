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
#include <text/worditerator.h>
#include <text/character.h>      // Character::codePointAt/codePointBefore/isLetter/isDigit
#include <text/textutils.h>      // TextUtils::getChars, TextUtils::isPunctuation
#include <unicode/ubrk.h>        // UBreakIterator (C API — matches minikin/myicu convention)
#include <unicode/utypes.h>      // UErrorCode, U_ZERO_ERROR
#include <unicode/uchar.h>
#include <algorithm>
#include <cassert>

namespace cdroid {

// =====================================================================================
//  Implementation handle (keeps ICU types out of the header). minikin's bundled
//  ICU (myicu) only exposes the C API (ubrk_*), so we use UBreakIterator here
//  rather than icu::BreakIterator — same convention as minikin/WordBreaker.cpp.
// =====================================================================================
struct WordIterator::Impl {
    UBreakIterator* bi = nullptr;
    std::string locale;
    std::u16string buffer;   // MUST outlive bi — ubrk_open stores a pointer into it
};

// =====================================================================================
//  Construction / destruction
// =====================================================================================
WordIterator::WordIterator() : WordIterator(std::string()) {}

WordIterator::WordIterator(const std::string& locale) : mImpl(new Impl) {
    mImpl->locale = locale;
}

WordIterator::~WordIterator() {
    if (mImpl && mImpl->bi) ubrk_close(mImpl->bi);
    delete mImpl;
}

// =====================================================================================
//  setCharSequence — mirrors WordIterator.setCharSequence. The window is
//  [mStart, mEnd] (padded by WINDOW_WIDTH); the BreakIterator walks a flat buffer
//  of that window, so its offsets are translated by ± mStart to/from absolute
//  CharSequence coordinates (matching Android's CharSequenceCharacterIterator,
//  whose begin index is mStart).
// =====================================================================================
void WordIterator::setCharSequence(const CharSequence* charSequence, int start, int end) {
    if (charSequence == nullptr) return;
    const int len = (int)charSequence->length();
    if (!(0 <= start && end <= len)) return;

    mCharSeq = charSequence;
    mStart = std::max(0, start - WINDOW_WIDTH);
    mEnd = std::min(len, end + WINDOW_WIDTH);

    // The buffer MUST outlive the UBreakIterator (ubrk_open stores the pointer),
    // so keep it on the Impl, not as a local — a local here would leave bi->text
    // dangling after this function returns (use-after-free → garbage boundaries).
    mImpl->buffer.assign((size_t)(mEnd - mStart), u'\0');
    TextUtils::getChars(charSequence, mStart, mEnd, &mImpl->buffer[0], 0);

    if (mImpl->bi) { ubrk_close(mImpl->bi); mImpl->bi = nullptr; }
    UErrorCode status = U_ZERO_ERROR;
    mImpl->bi = ubrk_open(UBRK_WORD, mImpl->locale.empty() ? "" : mImpl->locale.c_str(),
                          reinterpret_cast<const UChar*>(mImpl->buffer.data()),
                          (int32_t)mImpl->buffer.size(), &status);
    if (mImpl->bi == nullptr) {
        // Fall back to the root locale if the requested locale failed to open.
        status = U_ZERO_ERROR;
        mImpl->bi = ubrk_open(UBRK_WORD, "", reinterpret_cast<const UChar*>(mImpl->buffer.data()),
                              (int32_t)mImpl->buffer.size(), &status);
    }
}

// =====================================================================================
//  Boundary walks — faithful ports of preceding/following/isBoundary. ubrk works
//  0-based over the windowed buffer, hence the ± mStart translation.
// =====================================================================================
int WordIterator::preceding(int offset) const{
    checkOffsetIsValid(offset);
    while (true) {
        const int32_t b = ubrk_preceding(mImpl->bi, offset - mStart);
        if (b == UBRK_DONE) return DONE;
        offset = b + mStart;
        if (isOnLetterOrDigit(offset)) return offset;
    }
}

int WordIterator::following(int offset) const{
    checkOffsetIsValid(offset);
    while (true) {
        const int32_t b = ubrk_following(mImpl->bi, offset - mStart);
        if (b == UBRK_DONE) return DONE;
        offset = b + mStart;
        if (isAfterLetterOrDigit(offset)) return offset;
    }
}

bool WordIterator::isBoundary(int offset) const{
    checkOffsetIsValid(offset);
    return ubrk_isBoundary(mImpl->bi, offset - mStart);
}

int WordIterator::nextBoundary(int offset) const{
    checkOffsetIsValid(offset);
    const int32_t b = ubrk_following(mImpl->bi, offset - mStart);
    return (b == UBRK_DONE) ? DONE : b + mStart;
}

int WordIterator::prevBoundary(int offset) const{
    checkOffsetIsValid(offset);
    const int32_t b = ubrk_preceding(mImpl->bi, offset - mStart);
    return (b == UBRK_DONE) ? DONE : b + mStart;
}

// =====================================================================================
//  getBeginning / getEnd — the word-boundary queries used by Editor.selectCurrentWord.
// =====================================================================================
int WordIterator::getBeginning(int offset) const{
    return getBeginning(offset, false);
}

int WordIterator::getEnd(int offset) const{
    return getEnd(offset, false);
}

int WordIterator::getPrevWordBeginningOnTwoWordsBoundary(int offset) const{
    return getBeginning(offset, true);
}

int WordIterator::getNextWordEndOnTwoWordBoundary(int offset) const{
    return getEnd(offset, true);
}

int WordIterator::getBeginning(int offset, bool getPrevWordBeginningOnTwoWordsBoundary) const{
    checkOffsetIsValid(offset);
    if (isOnLetterOrDigit(offset)) {
        if (ubrk_isBoundary(mImpl->bi, offset - mStart)
                && (!isAfterLetterOrDigit(offset) || !getPrevWordBeginningOnTwoWordsBoundary)) {
            return offset;
        } else {
            const int32_t b = ubrk_preceding(mImpl->bi, offset - mStart);
            return (b == UBRK_DONE) ? DONE : b + mStart;
        }
    } else {
        if (isAfterLetterOrDigit(offset)) {
            const int32_t b = ubrk_preceding(mImpl->bi, offset - mStart);
            return (b == UBRK_DONE) ? DONE : b + mStart;
        }
    }
    return DONE;
}

int WordIterator::getEnd(int offset, bool getNextWordEndOnTwoWordBoundary) const{
    checkOffsetIsValid(offset);
    if (isAfterLetterOrDigit(offset)) {
        if (ubrk_isBoundary(mImpl->bi, offset - mStart)
                && (!isOnLetterOrDigit(offset) || !getNextWordEndOnTwoWordBoundary)) {
            return offset;
        } else {
            const int32_t b = ubrk_following(mImpl->bi, offset - mStart);
            return (b == UBRK_DONE) ? DONE : b + mStart;
        }
    } else {
        if (isOnLetterOrDigit(offset)) {
            const int32_t b = ubrk_following(mImpl->bi, offset - mStart);
            return (b == UBRK_DONE) ? DONE : b + mStart;
        }
    }
    return DONE;
}

// =====================================================================================
//  Punctuation queries — faithful ports.
// =====================================================================================
int WordIterator::getPunctuationBeginning(int offset) const{
    checkOffsetIsValid(offset);
    while (offset != DONE && !isPunctuationStartBoundary(offset)) {
        offset = prevBoundary(offset);
    }
    return offset;
}

int WordIterator::getPunctuationEnd(int offset) const{
    checkOffsetIsValid(offset);
    while (offset != DONE && !isPunctuationEndBoundary(offset)) {
        offset = nextBoundary(offset);
    }
    return offset;
}

bool WordIterator::isOnPunctuation(int offset) const{
    if (mStart <= offset && offset < mEnd && mCharSeq) {
        const int codePoint = Character::codePointAt(mCharSeq, offset);
        return TextUtils::isPunctuation(codePoint);
    }
    return false;
}

bool WordIterator::isAfterPunctuation(int offset) const{
    if (mStart < offset && offset <= mEnd && mCharSeq) {
        const int codePoint = Character::codePointBefore(mCharSeq, offset);
        return TextUtils::isPunctuation(codePoint);
    }
    return false;
}

bool WordIterator::isMidWordPunctuation(int codePoint) {
    const auto wb = u_getIntPropertyValue(codePoint, UCHAR_WORD_BREAK);

    // Check if the word break property matches MIDLETTER, MIDNUMLET, or SINGLE_QUOTE
    // These indicate punctuation that can appear within a word without necessarily breaking it.
    return (wb == U_WB_MIDLETTER) || (wb == U_WB_MIDNUMLET) || (wb == U_WB_SINGLE_QUOTE);
}

bool WordIterator::isPunctuationStartBoundary(int offset) const{
    return isOnPunctuation(offset) && !isAfterPunctuation(offset);
}

bool WordIterator::isPunctuationEndBoundary(int offset) const{
    return !isOnPunctuation(offset) && isAfterPunctuation(offset);
}

// =====================================================================================
//  Letter/digit probes — faithful ports of Character.isLetterOrDigit (composed
//  from Character::isLetter || Character::isDigit, which have int codePoint
//  overloads in this codebase).
// =====================================================================================
static bool isLetterOrDigit(int codePoint) {
    return Character::isLetter(codePoint) || Character::isDigit(codePoint);
}

bool WordIterator::isAfterLetterOrDigit(int offset) const{
    if (mStart < offset && offset <= mEnd && mCharSeq) {
        const int codePoint = Character::codePointBefore(mCharSeq, offset);
        if (isLetterOrDigit(codePoint)) return true;
    }
    return false;
}

bool WordIterator::isOnLetterOrDigit(int offset) const{
    if (mStart <= offset && offset < mEnd && mCharSeq) {
        const int codePoint = Character::codePointAt(mCharSeq, offset);
        if (isLetterOrDigit(codePoint)) return true;
    }
    return false;
}

void WordIterator::checkOffsetIsValid(int offset) const{
    // Android throws IllegalArgumentException; we just assert in debug builds —
    // every public method already bounds-checks via the isOn*/preceding probes.
    assert(mStart <= offset && offset <= mEnd);
    (void)offset;
}

}  // namespace cdroid
