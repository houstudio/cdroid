/*********************************************************************************
 * Port of android.text.WordSegmentFinder (android-36). Header-only.
 *
 * Implementation of SegmentFinder using words as the text segment. Word boundaries
 * are found using WordIterator (which uses myicu ubrk UBRK_WORD, 98.6% UAX#29
 * conformant). Whitespace characters are excluded — they are not included in any
 * text segments.
 *
 * Example: "Hello, World!" → segments "Hello", ",", "World", "!" (spaces excluded).
 *********************************************************************************/
#ifndef __WORD_SEGMENT_FINDER_H__
#define __WORD_SEGMENT_FINDER_H__
#include <text/segmentfinder.h>
#include <text/method/worditerator.h>
#include <text/character.h>

namespace cdroid {

class WordSegmentFinder : public SegmentFinder {
public:
    WordSegmentFinder(CharSequence* text, const std::string& locale)
        : mText(text), mWordIterator(locale) {
        mWordIterator.setCharSequence(text, 0, (int)text->length());
    }

    int previousStartBoundary(int offset) override {
        int boundary = offset;
        do {
            boundary = mWordIterator.prevBoundary(boundary);
            if (boundary == WordIterator::DONE) return DONE;
        } while (Character::isWhitespace((char16_t)mText->charAt(boundary)));
        return boundary;
    }

    int previousEndBoundary(int offset) override {
        int boundary = offset;
        do {
            boundary = mWordIterator.prevBoundary(boundary);
            if (boundary == WordIterator::DONE || boundary == 0) return DONE;
        } while (Character::isWhitespace((char16_t)mText->charAt(boundary - 1)));
        return boundary;
    }

    int nextStartBoundary(int offset) override {
        int boundary = offset;
        do {
            boundary = mWordIterator.nextBoundary(boundary);
            if (boundary == WordIterator::DONE || boundary == (int)mText->length()) return DONE;
        } while (Character::isWhitespace((char16_t)mText->charAt(boundary)));
        return boundary;
    }

    int nextEndBoundary(int offset) override {
        int boundary = offset;
        do {
            boundary = mWordIterator.nextBoundary(boundary);
            if (boundary == WordIterator::DONE) return DONE;
        } while (Character::isWhitespace((char16_t)mText->charAt(boundary - 1)));
        return boundary;
    }

private:
    CharSequence* mText;
    WordIterator mWordIterator;
};

}  // namespace cdroid
#endif  // __WORD_SEGMENT_FINDER_H__
