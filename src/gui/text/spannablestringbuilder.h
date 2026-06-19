#ifndef __SPANNABLE_STRING_BUILDER_H__
#define __SPANNABLE_STRING_BUILDER_H__
#include <core/predicate.h>
#include <text/textutils.h>
#include <text/textpaint.h>
#include <text/spannablestring.h>
#include <text/editable.h>
namespace cdroid{
// Mutable SpannableStringBuilder: builder-style mutable spannable (similar to Android's SpannableStringBuilder)
class SpannableStringBuilder : public SpannableStringInternal, virtual public Spannable, virtual public Editable{
public:
    SpannableStringBuilder() = default;
    explicit SpannableStringBuilder(const std::u16string& text);
    SpannableStringBuilder(const CharSequence*);    
    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override;
    void removeSpan(const ParcelableSpan* what) override;
    void shiftSpans(int index, int delta);
    void adjustSpansForReplace(int start, int end, int delta);
    SpannableStringBuilder& append(const std::string& utf8);
    SpannableStringBuilder& append(const std::u16string& utf16);
    SpannableStringBuilder& append(const char16_t*utf16,int start,int count);
    SpannableStringBuilder& append(const std::string& utf8,const ParcelableSpan* what, int flags);
    SpannableStringBuilder& append(const std::string& utf8, const std::vector<const ParcelableSpan*>& whats, int flags);
    Editable& append(const CharSequence& text);
    SpannableStringBuilder& append(const CharSequence& text, const ParcelableSpan* what, int flags);
    SpannableStringBuilder& append(const CharSequence& text, const std::vector<const ParcelableSpan*>& whats, int flags);
    SpannableStringBuilder& append(const std::string& utf8, int start, int end);
    Editable& append(const CharSequence& text, int start, int end);
    SpannableStringBuilder& insert(int where, const std::string& utf8);
    Editable& insert(int where, const CharSequence& text);
    SpannableStringBuilder& deleteText(int start, int end);
    SpannableStringBuilder& replace(int start, int end, const std::string& utf8);
    void setSpanForText(const std::string& targetUtf8, const ParcelableSpan* what, int flags, bool firstOnly = true);
    void getChars(int start, int end, char16_t* dest, int destPos) const override;
    
    // Editable interface methods
    Editable& replace(int st, int en, const CharSequence& source, int start, int end) override;
    Editable& replace(int st, int en, const CharSequence& text) override;
    Editable& insert(int where, const CharSequence& text, int start, int end) override;
    Editable& Delete(int st, int en) override;
    Editable& append(char16_t text) override;
    void clear() override;
    void clearSpans() override;
};
}/* namespace cdroid */
#endif/*__SPANNABLE_STRING_BUILDER_H__*/
