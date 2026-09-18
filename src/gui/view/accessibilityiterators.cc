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
#include <view/accessibilityiterators.h>
#include <content/breakiterator.h>
#include <text/character.h>    // Character::codePointAt / isLetterOrDigit

namespace cdroid{

// =====================================================================================
//  AbstractTextSegmentIterator
// =====================================================================================
void AbstractTextSegmentIterator::initialize(const std::u16string& text) {
    mText = text;
}

int* AbstractTextSegmentIterator::getRange(int start, int end) {
    if (start < 0 || end < 0 || start ==  end) {
        return nullptr;
    }
    mSegment[0] = start;
    mSegment[1] = end;
    return mSegment;
}

// =====================================================================================
//  CharacterTextSegmentIterator. The instances are process-lifetime singletons
//  (AOSP static sInstance); they are intentionally never freed.
//  AOSP registers a ViewRootImpl.ConfigChangedCallback to rebuild mImpl on
//  locale changes — CDROID has no ViewRootImpl config-callback infrastructure,
//  so the iterator keeps the locale it was first created with.
// =====================================================================================
CharacterTextSegmentIterator* CharacterTextSegmentIterator::sInstance = nullptr;

CharacterTextSegmentIterator::CharacterTextSegmentIterator(const Locale& locale) {
    mLocale = locale;
    onLocaleChanged(locale);
}

CharacterTextSegmentIterator* CharacterTextSegmentIterator::getInstance(const Locale& locale) {
    if (sInstance == nullptr) {
        sInstance = new CharacterTextSegmentIterator(locale);
    }
    return sInstance;
}

void CharacterTextSegmentIterator::onLocaleChanged(const Locale& locale) {
    // Java replaces the BreakIterator wholesale (GC); we own mImpl.
    delete mImpl;
    mImpl = BreakIterator::getCharacterInstance(locale);
}

void CharacterTextSegmentIterator::initialize(const std::u16string& text) {
    AbstractTextSegmentIterator::initialize(text);
    mImpl->setText(text);
}

int* CharacterTextSegmentIterator::following(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset >= textLength) {
        return nullptr;
    }
    int start = offset;
    if (start < 0) {
        start = 0;
    }
    while (!mImpl->isBoundary(start)) {
        start = mImpl->following(start);
        if (start == BreakIterator::DONE) {
            return nullptr;
        }
    }
    const int end = mImpl->following(start);
    if (end == BreakIterator::DONE) {
        return nullptr;
    }
    return getRange(start, end);
}

int* CharacterTextSegmentIterator::preceding(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset <= 0) {
        return nullptr;
    }
    int end = offset;
    if (end > textLength) {
        end = textLength;
    }
    while (!mImpl->isBoundary(end)) {
        end = mImpl->preceding(end);
        if (end == BreakIterator::DONE) {
            return nullptr;
        }
    }
    const int start = mImpl->preceding(end);
    if (start == BreakIterator::DONE) {
        return nullptr;
    }
    return getRange(start, end);
}

// =====================================================================================
//  WordTextSegmentIterator
// =====================================================================================
WordTextSegmentIterator* WordTextSegmentIterator::sInstance = nullptr;

WordTextSegmentIterator::WordTextSegmentIterator(const Locale& locale)
    : CharacterTextSegmentIterator(locale) {
    // Java dispatches onLocaleChanged virtually even from the super
    // constructor, installing the WORD instance; C++ base construction binds
    // to the base override (character). Re-run it here so the derived
    // override installs the word BreakIterator.
    onLocaleChanged(locale);
}

WordTextSegmentIterator* WordTextSegmentIterator::getInstance(const Locale& locale) {
    if (sInstance == nullptr) {
        sInstance = new WordTextSegmentIterator(locale);
    }
    return sInstance;
}

void WordTextSegmentIterator::onLocaleChanged(const Locale& locale) {
    delete mImpl;
    mImpl = BreakIterator::getWordInstance(locale);
}

int* WordTextSegmentIterator::following(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset >= (int)mText.length()) {
        return nullptr;
    }
    int start = offset;
    if (start < 0) {
        start = 0;
    }
    while (!isLetterOrDigit(start) && !isStartBoundary(start)) {
        start = mImpl->following(start);
        if (start == BreakIterator::DONE) {
            return nullptr;
        }
    }
    const int end = mImpl->following(start);
    if (end == BreakIterator::DONE || !isEndBoundary(end)) {
        return nullptr;
    }
    return getRange(start, end);
}

int* WordTextSegmentIterator::preceding(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset <= 0) {
        return nullptr;
    }
    int end = offset;
    if (end > textLength) {
        end = textLength;
    }
    while (end > 0 && !isLetterOrDigit(end - 1) && !isEndBoundary(end)) {
        end = mImpl->preceding(end);
        if (end == BreakIterator::DONE) {
            return nullptr;
        }
    }
    const int start = mImpl->preceding(end);
    if (start == BreakIterator::DONE || !isStartBoundary(start)) {
        return nullptr;
    }
    return getRange(start, end);
}

bool WordTextSegmentIterator::isStartBoundary(int index) {
    return isLetterOrDigit(index)
        && (index == 0 || !isLetterOrDigit(index - 1));
}

bool WordTextSegmentIterator::isEndBoundary(int index) {
    return (index > 0 && isLetterOrDigit(index - 1))
        && (index == (int)mText.length() || !isLetterOrDigit(index));
}

bool WordTextSegmentIterator::isLetterOrDigit(int index) {
    if (index >= 0 && index < (int)mText.length()) {
        const int codePoint = Character::codePointAt(mText.c_str(), index);
        return Character::isLetterOrDigit(codePoint);
    }
    return false;
}

// =====================================================================================
//  ParagraphTextSegmentIterator
// =====================================================================================
ParagraphTextSegmentIterator* ParagraphTextSegmentIterator::sInstance = nullptr;

ParagraphTextSegmentIterator* ParagraphTextSegmentIterator::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new ParagraphTextSegmentIterator();
    }
    return sInstance;
}

int* ParagraphTextSegmentIterator::following(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset >= textLength) {
        return nullptr;
    }
    int start = offset;
    if (start < 0) {
        start = 0;
    }
    while (start < textLength && mText[start] == u'\n'
            && !isStartBoundary(start)) {
        start++;
    }
    if (start >= textLength) {
        return nullptr;
    }
    int end = start + 1;
    while (end < textLength && !isEndBoundary(end)) {
        end++;
    }
    return getRange(start, end);
}

int* ParagraphTextSegmentIterator::preceding(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset <= 0) {
        return nullptr;
    }
    int end = offset;
    if (end > textLength) {
        end = textLength;
    }
    while (end > 0 && mText[end - 1] == u'\n' && !isEndBoundary(end)) {
        end--;
    }
    if (end <= 0) {
        return nullptr;
    }
    int start = end - 1;
    while (start > 0 && !isStartBoundary(start)) {
        start--;
    }
    return getRange(start, end);
}

bool ParagraphTextSegmentIterator::isStartBoundary(int index) {
    return (mText[index] != u'\n'
        && (index == 0 || mText[index - 1] == u'\n'));
}

bool ParagraphTextSegmentIterator::isEndBoundary(int index) {
    return (index > 0 && mText[index - 1] != u'\n'
        && (index == (int)mText.length() || mText[index] == u'\n'));
}

}  // namespace cdroid
