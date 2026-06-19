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
    if (start < 0) start = 0;
    if (end > (int)mText.length()) end = (int)mText.size();
    if (start >= end) return;
    mSpans.push_back({what,start,end,flags});
}

void SpannableStringBuilder::removeSpan(const ParcelableSpan* what) {
    mSpans.erase(std::remove_if(mSpans.begin(), mSpans.end(), 
        [&](const std::tuple<const ParcelableSpan*,int,int,int>& s){ 
            return std::get<0>(s) == what; 
        }), mSpans.end());
}

void SpannableStringBuilder::shiftSpans(int index, int delta) {
    for (auto t : mSpans) {
        const ParcelableSpan*span;
        int sstart,send,sflags;
        std::tie(span,sstart,send,sflags) = t;
        if (sstart >= index) {
            sstart += delta;
            send += delta;
        } else if (send > index) {
            send += delta;
        }
    }
}

void SpannableStringBuilder::adjustSpansForReplace(int start, int end, int delta) {
    std::vector<std::tuple<const ParcelableSpan*,int,int,int>> result;
    for (auto& t : mSpans) {
        const ParcelableSpan*span;
        int sstart,send,sflags;
        std::tie(span,sstart,send,sflags) = t;
        if (send <= start) {
            result.push_back({span,sstart,send,sflags});
        } else if (sstart >= end) {
            sstart += delta;
            send += delta;
            result.push_back({span,sstart,send,sflags});
        } else if (sstart < start && send > end) {
            send = start;
            result.push_back({span,sstart,send,sflags});
        } else if (sstart < start && send > start) {
            send = start;
            result.push_back({span,sstart,send,sflags});
        } else if (sstart < end && send > end) {
            sstart = start + delta;
            send += delta;
            result.push_back({span,sstart,send,sflags});
        }
    }
    mSpans.swap(result);
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::string& utf8){
    mText += TextUtils::utf8_utf16(utf8);
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::u16string& utf16){
    mText+=utf16;
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const char16_t*utf16,int start,int count){
    mText+=std::u16string(utf16+start,count);
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::string& utf8,const ParcelableSpan* what, int flags) {
    int start = (int)mText.length();
    append(utf8);
    setSpan(what, start, (int)mText.length(), flags);
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::string& utf8, const std::vector<const ParcelableSpan*>& whats, int flags) {
    int start = (int)mText.length();
    append(utf8);
    int end = (int)mText.length();
    for (const auto& what : whats) {
        if (what) {
            setSpan(what, start, end, flags);
        }
    }
    return *this;
}

Editable& SpannableStringBuilder::append(const CharSequence& text) {
    append(text.toString());
    return *this;
}

SpannableStringBuilder& SpannableStringBuilder::append(const CharSequence& text, const ParcelableSpan* what, int flags) {
    return append(text.toString(), what, flags);
}

SpannableStringBuilder& SpannableStringBuilder::append(const CharSequence& text, const std::vector<const ParcelableSpan*>& whats, int flags) {
    return append(text.toString(), whats, flags);
}

SpannableStringBuilder& SpannableStringBuilder::append(const std::string& utf8, int start, int end) {
    std::u16string source = TextUtils::utf8_utf16(utf8);
    if (start < 0) start = 0;
    if (end > (int)source.length()) end = (int)source.length();
    if (start >= end) return *this;
    mText += source.substr(start, end - start);
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

SpannableStringBuilder& SpannableStringBuilder::insert(int where, const std::string& utf8) {
    /*std::wstring insertText = TextUtils::utf8tounicode(utf8);
    if (where < 0) where = 0;
    if (where > (int)mText.length()) where = (int)mText.length();
    mText.insert(where, insertText);
    shiftSpans(where, (int)insertText.length());*/
    return *this;
}

Editable& SpannableStringBuilder::insert(int where, const CharSequence& text) {
    insert(where, text.toString());
    return *this;
}

// Android equivalent: delete(start, end)
// C++ cannot use the keyword `delete` as a method name.
SpannableStringBuilder& SpannableStringBuilder::deleteText(int start, int end) {
    return replace(start, end, std::string());
}

SpannableStringBuilder& SpannableStringBuilder::replace(int start, int end, const std::string& utf8) {
    /*if (start < 0) start = 0;
    if (end > (int)mText.length()) end = (int)mText.length();
    if (start >= end) return *this;
    std::wstring insertText = TextUtils::utf8tounicode(utf8);
    int oldLen = end - start;
    mText.replace(start, oldLen, insertText);
    int delta = (int)insertText.length() - oldLen;
    adjustSpansForReplace(start, end, delta);*/
    return *this;
}

void SpannableStringBuilder::setSpanForText(const std::string& targetUtf8, const ParcelableSpan* what, int flags, bool firstOnly) {
    if (!what) return;
    /*std::wstring target = TextUtils::utf8tounicode(targetUtf8);
    size_t pos = 0;
    while (pos < mText.size()) {
        size_t found = mText.find(target, pos);
        if (found == std::wstring::npos) break;
        setSpan(what, (int)found, (int)(found + target.length()), flags);
        if (firstOnly) break;
        pos = found + 1;
    }*/
}

void SpannableStringBuilder::getChars(int start, int end, char16_t* dest, int destPos) const{
    for(int i=start;i<end;i++)
        dest[i+destPos]=mText[start+i];
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

Editable& SpannableStringBuilder::Delete(int st, int en) {
    deleteText(st, en);
    return *this;
}

Editable& SpannableStringBuilder::append(char16_t text) {
    mText += text;
    return *this;
}

void SpannableStringBuilder::clearSpans() {
    mSpans.clear();
}

void SpannableStringBuilder::clear() {
    mText.clear();
    mSpans.clear();
}
}/* namespace cdroid */
