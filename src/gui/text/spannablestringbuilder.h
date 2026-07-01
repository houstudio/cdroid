#ifndef __SPANNABLE_STRING_BUILDER_H__
#define __SPANNABLE_STRING_BUILDER_H__

#include <core/predicate.h>
#include <text/textutils.h>
#include <text/textpaint.h>
#include <text/spannablestring.h>
#include <text/editable.h>
#include <vector>

namespace cdroid {

class InputFilter;

class SpannableStringBuilder : public SpannableStringInternal, virtual public Spannable, virtual public Editable {
public:
    SpannableStringBuilder() = default;
    explicit SpannableStringBuilder(const std::u16string& text);
    SpannableStringBuilder(const CharSequence*);    

    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override;
    void removeSpan(const ParcelableSpan* what) override;
    void shiftSpans(int index, int delta);
    void adjustSpansForReplace(int start, int end, int delta);

    Editable& append(const CharSequence& text);
    SpannableStringBuilder& append(const std::u16string&text,int flags);
    SpannableStringBuilder& append(const std::u16string&text, const ParcelableSpan* what, int flags);
    SpannableStringBuilder& append(const std::u16string& text, const std::vector<const ParcelableSpan*>& whats, int flags);
    SpannableStringBuilder& append(const CharSequence& text, const ParcelableSpan* what, int flags);
    SpannableStringBuilder& append(const CharSequence& text, const std::vector<const ParcelableSpan*>& whats, int flags);

    Editable& append(const CharSequence& text, int start, int end);
    Editable& insert(int where, const CharSequence& text);
    SpannableStringBuilder& deleteText(int start, int end);
    void getChars(int start, int end, char16_t* dest, int destPos) const override;
    
    // Editable interface methods
    Editable& replace(int st, int en, const CharSequence& source, int start, int end) override;
    Editable& replace(int st, int en, const CharSequence& text) override;
    Editable& insert(int where, const CharSequence& text, int start, int end) override;
    Editable& Delete(int st, int en) override;
    Editable& append(char16_t text) override;
    void clear() override;
    void clearSpans() override;
    void setFilters(InputFilter** filters, int count) override;
    InputFilter** getFilters(int* outCount) const override;

    // Appendable interface methods
    Appendable& append(const char16_t* s, int start, int len) override;
};

}/* namespace cdroid */
#endif/*__SPANNABLE_STRING_BUILDER_H__*/
