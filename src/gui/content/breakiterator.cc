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
#include <content/breakiterator.h>
#include <minikin/GraphemeBreak.h>      // character instance backend
#include <text/method/worditerator.h>   // word instance backend
#include <text/String.h>
#include <algorithm>

namespace cdroid{

struct BreakIterator::Impl {
    enum Kind { CHARACTER, WORD, LINE, SENTENCE } kind = CHARACTER;
    std::string lang;                   // ISO-639 hint for the word breaker
    std::u16string buffer;              // the text (grapheme walks read it)
    String* charSeq = nullptr;          // CharSequence face of buffer for WordIterator
    WordIterator* wordIterator = nullptr;
    int current = 0;
};

BreakIterator::BreakIterator(int kind, const Locale& locale) : mImpl(new Impl) {
    mImpl->kind = (Impl::Kind)kind;
    mImpl->lang = locale.getLanguage();
    if (mImpl->kind == Impl::WORD) {
        mImpl->wordIterator = new WordIterator(mImpl->lang);
    }
}

BreakIterator::~BreakIterator() {
    delete mImpl->wordIterator;
    delete mImpl->charSeq;
    delete mImpl;
}

BreakIterator* BreakIterator::getCharacterInstance(const Locale& locale) {
    return new BreakIterator(Impl::CHARACTER, locale);
}

BreakIterator* BreakIterator::getWordInstance(const Locale& locale) {
    return new BreakIterator(Impl::WORD, locale);
}

BreakIterator* BreakIterator::getLineInstance(const Locale& locale) {
    return new BreakIterator(Impl::LINE, locale);
}

BreakIterator* BreakIterator::getSentenceInstance(const Locale& locale) {
    return new BreakIterator(Impl::SENTENCE, locale);
}

void BreakIterator::setText(const std::u16string& text) {
    mImpl->buffer = text;
    if (mImpl->kind == Impl::WORD) {
        // WordIterator borrows the sequence: keep the String beside it.
        delete mImpl->charSeq;
        mImpl->charSeq = new String(mImpl->buffer);
        mImpl->wordIterator->setCharSequence(mImpl->charSeq, 0, (int)mImpl->buffer.size());
    }
    mImpl->current = 0;
}

int BreakIterator::first() {
    mImpl->current = 0;
    return mImpl->current;
}

int BreakIterator::last() {
    mImpl->current = (int)mImpl->buffer.size();
    return mImpl->current;
}

int BreakIterator::next() {
    const int boundary = following(mImpl->current);
    if (boundary != DONE) mImpl->current = boundary;
    return boundary;
}

int BreakIterator::previous() {
    const int boundary = preceding(mImpl->current);
    if (boundary != DONE) mImpl->current = boundary;
    return boundary;
}

int BreakIterator::following(int offset) {
    const int len = (int)mImpl->buffer.size();
    if (len == 0) return DONE;
    if (offset >= len) return DONE;
    const int from = std::max(offset, -1);
    switch (mImpl->kind) {
        case Impl::CHARACTER:
            return (int)minikin::GraphemeBreak::getTextRunCursor(nullptr,
                    reinterpret_cast<const uint16_t*>(mImpl->buffer.data()), 0, len,
                    (size_t)from, minikin::GraphemeBreak::AFTER);
        case Impl::WORD:
            return mImpl->wordIterator->nextBoundary(from);
        case Impl::LINE:
        case Impl::SENTENCE:
        default:
            // Whole-text boundaries only: the walk lands on the far end.
            return len;
    }
}

int BreakIterator::preceding(int offset) {
    const int len = (int)mImpl->buffer.size();
    if (len == 0) return DONE;
    if (offset <= 0) return DONE;
    const int from = std::min(offset, len);
    switch (mImpl->kind) {
        case Impl::CHARACTER:
            return (int)minikin::GraphemeBreak::getTextRunCursor(nullptr,
                    reinterpret_cast<const uint16_t*>(mImpl->buffer.data()), 0, len,
                    (size_t)from, minikin::GraphemeBreak::BEFORE);
        case Impl::WORD:
            return mImpl->wordIterator->prevBoundary(from);
        case Impl::LINE:
        case Impl::SENTENCE:
        default:
            return 0;
    }
}

int BreakIterator::current() {
    return mImpl->current;
}

bool BreakIterator::isBoundary(int offset) {
    const int len = (int)mImpl->buffer.size();
    if (offset < 0 || offset > len) {
        return false;
    }
    if (offset == 0 || offset == len) {
        return true;
    }
    switch (mImpl->kind) {
        case Impl::CHARACTER:
            return minikin::GraphemeBreak::isGraphemeBreak(nullptr,
                    reinterpret_cast<const uint16_t*>(mImpl->buffer.data()), 0, len,
                    (size_t)offset);
        case Impl::WORD:
            return mImpl->wordIterator->isBoundary(offset);
        case Impl::LINE:
        case Impl::SENTENCE:
        default:
            return false;
    }
}

}  // namespace cdroid
