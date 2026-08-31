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
#ifndef __CDROID_ACCESSIBILITY_ITERATORS_H__
#define __CDROID_ACCESSIBILITY_ITERATORS_H__
#include <string>
#include <content/Locale.h>

namespace cdroid{

class BreakIterator;

/**
 * Line-for-line port of android.view.AccessibilityIterators (the view half:
 * Character/Word/Paragraph). The widget half (Line/Page, which need Layout
 * and TextView) lives in widget/accessibilityiterators.h, mirroring the AOSP
 * android.view / android.widget file split. The nested AOSP classes are
 * flattened to top level, matching the CDROID convention.
 *
 * following()/preceding() return the shared two-int segment buffer (AOSP's
 * int[]) or nullptr when there is no segment at that offset. All offsets are
 * UTF-16 code-unit indices into the text passed to initialize(), matching
 * Android's String coordinates.
 */
class TextSegmentIterator{
public:
    virtual ~TextSegmentIterator() = default;
    virtual int* following(int current) = 0;
    virtual int* preceding(int current) = 0;
};

class AbstractTextSegmentIterator : public TextSegmentIterator{
protected:
    std::u16string mText;
public:
    virtual void initialize(const std::u16string& text);
protected:
    int* getRange(int start, int end);
private:
    int mSegment[2];
};

class CharacterTextSegmentIterator : public AbstractTextSegmentIterator{
private:
    static CharacterTextSegmentIterator* sInstance;
    Locale mLocale;
protected:
    BreakIterator* mImpl = nullptr;
    virtual void onLocaleChanged(const Locale& locale);
    CharacterTextSegmentIterator(const Locale& locale);
public:
    static CharacterTextSegmentIterator* getInstance(const Locale& locale);
    void initialize(const std::u16string& text)override;
    int* following(int offset)override;
    int* preceding(int offset)override;
};

class WordTextSegmentIterator : public CharacterTextSegmentIterator{
private:
    static WordTextSegmentIterator* sInstance;
    WordTextSegmentIterator(const Locale& locale);
protected:
    void onLocaleChanged(const Locale& locale)override;
public:
    static WordTextSegmentIterator* getInstance(const Locale& locale);
    int* following(int offset)override;
    int* preceding(int offset)override;
private:
    bool isStartBoundary(int index);
    bool isEndBoundary(int index);
    bool isLetterOrDigit(int index);
};

class ParagraphTextSegmentIterator : public AbstractTextSegmentIterator{
private:
    static ParagraphTextSegmentIterator* sInstance;
    ParagraphTextSegmentIterator() = default;
public:
    static ParagraphTextSegmentIterator* getInstance();
    int* following(int offset)override;
    int* preceding(int offset)override;
private:
    bool isStartBoundary(int index);
    bool isEndBoundary(int index);
};

}  // namespace cdroid
#endif  // __CDROID_ACCESSIBILITY_ITERATORS_H__
