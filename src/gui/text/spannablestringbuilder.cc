#include <text/spannablestringbuilder.h>
namespace cdroid{
// Mutable SpannableStringBuilder: builder-style mutable spannable (similar to Android's SpannableStringBuilder)
SpannableStringBuilder::SpannableStringBuilder(const std::u16string& text)
        : SpannableStringInternal(text){
}

SpannableStringBuilder::SpannableStringBuilder(const CharSequence*text)
    :SpannableStringInternal(text,0,text?(int)text->length():0,false){
}
    
void SpannableStringBuilder::setSpan(const ParcelableSpan* what, int start, int end, int flags) {
    if (!what) return;
    const int len = (int)mText.length();
    if (start < 0) start = 0;
    if (end < 0) end = 0;
    if (start > len) start = len;
    if (end > len) end = len;
    if (start > end) { const int t = start; start = end; end = t; }
    // Android allows zero-length (point) spans — selection cursors rely on them.
    // Update in place if the span is already present (Android does this); this
    // also prevents a duplicate owned pointer that would double-free on destroy.
    for (auto& r : mSpans) {
        if (r.span == what) {
            r.start = start;
            r.end = end;
            r.flags = flags;
            return;
        }
    }
    addSpan(what, start, end, flags);
}

void SpannableStringBuilder::removeSpan(const ParcelableSpan* what) {
    // Loop in case legacy data held duplicate records for the same pointer.
    while (removeSpanRecord(what)) { /* removed all matching */ }
}

void SpannableStringBuilder::shiftSpans(int index, int delta) {
    for (auto& r : mSpans) {
        if (r.start >= index) {
            r.start += delta;
            r.end += delta;
        } else if (r.end > index) {
            r.end += delta;
        }
    }
}

void SpannableStringBuilder::adjustSpansForReplace(int start, int end, int delta) {
    // Adjust every span to a replacement of [start, end) with text of length
    // (end-start)+delta. The previous logic truncated the tail of spans that
    // contained the changed region (`send = start`), which silently shrank the
    // whole-range watcher spans (mChangeWatcher, DynamicLayout::mWatcher) on
    // every edit. This mirrors Android's intent: spans before the change are
    // untouched, spans after shift by delta, and overlapping spans are clamped
    // — start into [.., start], end into [end+delta, ..].
    for (auto& r : mSpans) {
        if (r.end <= start) {
            // entirely before the change — unchanged
        } else if (r.start >= end) {
            // entirely after the change — shift by delta
            r.start += delta;
            r.end += delta;
        } else {
            // overlaps the change
            if (r.start > start) r.start = start;
            if (r.end >= end) r.end += delta;
            else r.end = end + delta;   // end was inside the replaced region
        }
    }
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::u16string&text,int flags){
     mText.append(text);
     return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::u16string&text, const ParcelableSpan* what, int flags){
    const size_t start=mText.length();
    const size_t end=start+text.length();
    mText.append(text);
    setSpan(what,start,end,flags);
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::u16string& text, const std::vector<const ParcelableSpan*>& whats, int flags){
    const size_t start=mText.length();
    const size_t end=start+text.length();
    mText.append(text);
    for(auto span:whats){
        setSpan(span,start,end,flags);
    }
    return *this;
}

Editable& SpannableStringBuilder::append(const CharSequence& text) {
    const int textLen = (int)text.length();
    for (int i = 0; i < textLen; i++) {
        mText += (char16_t)text.charAt(i);
    }
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const CharSequence& text, const ParcelableSpan* what, int flags) {
    int start = (int)mText.length();
    append(text);
    if (what) {
        setSpan(what, start, (int)mText.length(), flags);
    }
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const CharSequence& text, const std::vector<const ParcelableSpan*>& whats, int flags) {
    int start = (int)mText.length();
    append(text);
    int end = (int)mText.length();
    for (const auto& what : whats) {
        if (what) {
            setSpan(what, start, end, flags);
        }
    }
    return *this;
}

Editable& SpannableStringBuilder::append(const CharSequence& text, int start, int end) {
    if (start < 0) start = 0;
    if (end > (int)text.length()) end = (int)text.length();
    if (start >= end) return *this;
    for (int i = start; i < end; i++) {
        mText += (char16_t)text.charAt(i);
    }
    return *this;
}

Editable& SpannableStringBuilder::insert(int where, const CharSequence& text) {
    // Delegate to the 4-arg insert, which actually does mText.insert + shiftSpans.
    // (The previous body called the `std::string` overload, whose body is stubbed
    // out — so inserting a char via the Editable interface silently did nothing,
    // which broke Editor's commitText / on-the-fly replace and desynced the caret.)
    return insert(where, text, 0, (int)text.length());
}

// Android equivalent: delete(start, end)
// C++ cannot use the keyword `delete` as a method name.
SpannableStringBuilder& SpannableStringBuilder::deleteText(int start, int end) {
    SpannedString empty;
    replace(start, end, empty);
    return *this;
}

void SpannableStringBuilder::getChars(int start, int end, char16_t* dest, int destPos) const{
    // Copy mText[start, end) to dest[destPos, ...). The previous override used
    // wrong indices (`dest[i+destPos] = mText[start+i]`) which, for start != 0,
    // wrote out of bounds and corrupted the heap — surfacing later as garbage
    // span pointers in getSpans. Match SpannableStringInternal::getChars exactly.
    if (start >= end) return;
    const int len = (int)mText.length();
    if (start < 0) start = 0;
    if (end > len) end = len;
    for (int i = 0; i < end - start; i++) {
        dest[destPos + i] = mText[start + i];
    }
}

// Editable interface implementations
Editable& SpannableStringBuilder::replace(int st, int en, const CharSequence& text) {
    if (st < 0) st = 0;
    if (en > (int)mText.length()) en = (int)mText.length();
    if (st >= en) return *this;
    
    const int textLen = (int)text.length();
    std::u16string insertText;
    insertText.reserve(textLen);
    for (int i = 0; i < textLen; i++) {
        insertText += (char16_t)text.charAt(i);
    }
    
    const int oldLen = en - st;
    mText.replace(st, oldLen, insertText);
    const int delta = (int)insertText.length() - oldLen;
    adjustSpansForReplace(st, en, delta);
    return *this;
}

Editable& SpannableStringBuilder::replace(int st, int en, const CharSequence& source, int srcStart, int srcEnd) {
    if (srcStart < 0) srcStart = 0;
    if (srcEnd > (int)source.length()) srcEnd = (int)source.length();
    if (srcStart >= srcEnd) return *this;
    
    std::u16string insertText;
    insertText.reserve(srcEnd - srcStart);
    for (int i = srcStart; i < srcEnd; i++) {
        insertText += (char16_t)source.charAt(i);
    }
    
    if (st < 0) st = 0;
    if (en > (int)mText.length()) en = (int)mText.length();
    if (st >= en) return *this;
    
    const int oldLen = en - st;
    mText.replace(st, oldLen, insertText);
    const int delta = (int)insertText.length() - oldLen;
    adjustSpansForReplace(st, en, delta);
    return *this;
}

Editable& SpannableStringBuilder::insert(int where, const CharSequence& text, int start, int end) {
    if (start < 0) start = 0;
    if (end > (int)text.length()) end = (int)text.length();
    if (start >= end) return *this;
    
    std::u16string insertText;
    insertText.reserve(end - start);
    for (int i = start; i < end; i++) {
        insertText += (char16_t)text.charAt(i);
    }
    
    if (where < 0) where = 0;
    if (where > (int)mText.length()) where = (int)mText.length();
    
    mText.insert(where, insertText);
    shiftSpans(where, (int)insertText.length());
    return *this;
}

Editable& SpannableStringBuilder::Delete(int start, int end) {
    deleteText(start, end);
    return *this;
}

Editable& SpannableStringBuilder::append(char16_t text) {
    mText += text;
    return *this;
}

void SpannableStringBuilder::clearSpans() {
    deleteAllOwnedSpans();
}

void SpannableStringBuilder::clear() {
    mText.clear();
    deleteAllOwnedSpans();
}

void SpannableStringBuilder::setFilters(InputFilter** filters, int count) {
    (void)filters;
    (void)count;
}

InputFilter** SpannableStringBuilder::getFilters(int* outCount) const {
    *outCount = 0;
    return nullptr;
}

Appendable& SpannableStringBuilder::append(const char16_t* s, int start, int len) {
    mText.append(s + start, len);
    return *this;
}
}/* namespace cdroid */
