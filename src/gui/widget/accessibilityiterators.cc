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
#include <widget/accessibilityiterators.h>
#include <widget/textview.h>
#include <text/layout.h>
#include <text/textutils.h>    // TextUtils::utf8_utf16
#include <algorithm>

namespace cdroid{

// =====================================================================================
//  LineTextSegmentIterator. Process-lifetime singleton (AOSP static
//  sLineInstance), intentionally never freed.
// =====================================================================================
LineTextSegmentIterator* LineTextSegmentIterator::sLineInstance = nullptr;

LineTextSegmentIterator* LineTextSegmentIterator::getInstance() {
    if (sLineInstance == nullptr) {
        sLineInstance = new LineTextSegmentIterator();
    }
    return sLineInstance;
}

void LineTextSegmentIterator::initialize(const std::u16string& text, Layout* layout) {
    mText = text;
    mLayout = layout;
}

int* LineTextSegmentIterator::following(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset >= (int)mText.length()) {
        return nullptr;
    }
    int nextLine;
    if (offset < 0) {
        nextLine = mLayout->getLineForOffset(0);
    } else {
        const int currentLine = mLayout->getLineForOffset(offset);
        if (getLineEdgeIndex(currentLine, DIRECTION_START) == offset) {
            nextLine = currentLine;
        } else {
            nextLine = currentLine + 1;
        }
    }
    if (nextLine >= mLayout->getLineCount()) {
        return nullptr;
    }
    const int start = getLineEdgeIndex(nextLine, DIRECTION_START);
    const int end = getLineEdgeIndex(nextLine, DIRECTION_END) + 1;
    return getRange(start, end);
}

int* LineTextSegmentIterator::preceding(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset <= 0) {
        return nullptr;
    }
    int previousLine;
    if (offset > (int)mText.length()) {
        previousLine = mLayout->getLineForOffset((int)mText.length());
    } else {
        const int currentLine = mLayout->getLineForOffset(offset);
        if (getLineEdgeIndex(currentLine, DIRECTION_END) + 1 == offset) {
            previousLine = currentLine;
        } else {
            previousLine = currentLine - 1;
        }
    }
    if (previousLine < 0) {
        return nullptr;
    }
    const int start = getLineEdgeIndex(previousLine, DIRECTION_START);
    const int end = getLineEdgeIndex(previousLine, DIRECTION_END) + 1;
    return getRange(start, end);
}

int LineTextSegmentIterator::getLineEdgeIndex(int lineNumber, int direction) {
    const int paragraphDirection = mLayout->getParagraphDirection(lineNumber);
    if (direction * paragraphDirection < 0) {
        return mLayout->getLineStart(lineNumber);
    } else {
        return mLayout->getLineEnd(lineNumber) - 1;
    }
}

// =====================================================================================
//  PageTextSegmentIterator. Process-lifetime singleton (AOSP static
//  sPageInstance), intentionally never freed.
// =====================================================================================
PageTextSegmentIterator* PageTextSegmentIterator::sPageInstance = nullptr;

PageTextSegmentIterator* PageTextSegmentIterator::getInstance() {
    if (sPageInstance == nullptr) {
        sPageInstance = new PageTextSegmentIterator();
    }
    return sPageInstance;
}

void PageTextSegmentIterator::initialize(TextView* view) {
    LineTextSegmentIterator::initialize(
            TextUtils::utf8_utf16(view->getIterableTextForAccessibility()),
            view->getLayout());
    mView = view;
}

int* PageTextSegmentIterator::following(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset >= (int)mText.length()) {
        return nullptr;
    }
    if (!mView->getGlobalVisibleRect(mTempRect, nullptr)) {
        return nullptr;
    }

    const int start = std::max(0, offset);

    const int currentLine = mLayout->getLineForOffset(start);
    const int currentLineTop = mLayout->getLineTop(currentLine);
    const int pageHeight = mTempRect.height - mView->getTotalPaddingTop()
            - mView->getTotalPaddingBottom();
    const int nextPageStartY = currentLineTop + pageHeight;
    const int lastLineTop = mLayout->getLineTop(mLayout->getLineCount() - 1);
    const int currentPageEndLine = (nextPageStartY < lastLineTop)
            ? mLayout->getLineForVertical(nextPageStartY) - 1 : mLayout->getLineCount() - 1;

    const int end = getLineEdgeIndex(currentPageEndLine, DIRECTION_END) + 1;

    return getRange(start, end);
}

int* PageTextSegmentIterator::preceding(int offset) {
    const int textLength = (int)mText.length();
    if (textLength <= 0) {
        return nullptr;
    }
    if (offset <= 0) {
        return nullptr;
    }
    if (!mView->getGlobalVisibleRect(mTempRect, nullptr)) {
        return nullptr;
    }

    const int end = std::min((int)mText.length(), offset);

    const int currentLine = mLayout->getLineForOffset(end);
    const int currentLineTop = mLayout->getLineTop(currentLine);
    const int pageHeight = mTempRect.height - mView->getTotalPaddingTop()
            - mView->getTotalPaddingBottom();
    const int previousPageEndY = currentLineTop - pageHeight;
    int currentPageStartLine = (previousPageEndY > 0) ?
             mLayout->getLineForVertical(previousPageEndY) : 0;
    // If we're at the end of text, we're at the end of the current line rather than the
    // start of the next line, so we should move up one fewer lines than we would otherwise.
    if (end == (int)mText.length() && (currentPageStartLine < currentLine)) {
        currentPageStartLine += 1;
    }

    const int start = getLineEdgeIndex(currentPageStartLine, DIRECTION_START);

    return getRange(start, end);
}

}  // namespace cdroid
