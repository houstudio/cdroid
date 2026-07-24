#ifndef __EDITABLE_H__
#define __EDITABLE_H__

#include <text/getchars.h>
#include <text/appendable.h>
#include <text/spannablestring.h>   // full CharSequence + Spannable bases (Editable inherits both)

namespace cdroid {

class CharSequence;
class InputFilter;

class Editable : virtual public CharSequence, virtual public Spannable, public GetChars, public Appendable {
public:
    using Appendable::append;
    using Factory = std::function<Editable*(CharSequence*)>;
    virtual Editable& replace(int st, int en, const CharSequence& source, int start, int end) = 0;

    virtual Editable& replace(int st, int en, const CharSequence& text) = 0;

    virtual Editable& insert(int where, const CharSequence& text, int start, int end) = 0;

    virtual Editable& insert(int where, const CharSequence& text) = 0;

    virtual Editable& Delete(int st, int en) = 0;

    virtual Editable& append(const CharSequence& text) = 0;

    virtual Editable& append(const CharSequence& text, int start, int end) = 0;

    virtual Editable& append(char16_t text) = 0;

    virtual void clear() = 0;

    virtual void clearSpans() = 0;

    virtual void setFilters(InputFilter** filters, int count) = 0;

    virtual InputFilter** getFilters(int* outCount) const = 0;

};

}/*endof namespace*/
#endif/*__EDITABLE_H__*/
