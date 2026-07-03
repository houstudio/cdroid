#ifndef __TEXTWATCHER_H__
#define __TEXTWATCHER_H__
#include <text/parcelablespan.h>
#include <memory>   // std::shared_ptr (mID)
namespace cdroid{
class Editable;
// NoCopySpan is inherited VIRTUALLY so that a class multiply inheriting
// TextWatcher and SpanWatcher (e.g. TextView::ChangeWatcher) has a SINGLE
// NoCopySpan subobject — otherwise dynamic_cast<NoCopySpan*> would be
// ambiguous and the span would be misclassified as owned (→ double-free).
class TextWatcher :virtual public NoCopySpan {
protected:
    std::shared_ptr<void*>mID;
public:
    TextWatcher(){
        mID = std::make_shared<void*>(this);
    }
    TextWatcher(const TextWatcher&other){
        mID = other.mID;
    }
    TextWatcher& operator=(const TextWatcher&other){
        mID = other.mID;
        return *this;
    }
    bool operator == (const TextWatcher&other)const{
        return mID == other.mID;
    }
    bool operator != (const TextWatcher&other)const{
        return mID != other.mID;
    }
    //virtual void beforeTextChanged(CharSequence& s, int start, int count, int after)=0;
    //virtual void onTextChanged(CharSequence& s, int start, int before, int count)=0;
    //virtual void afterTextChanged(Editable& s)=0;
    std::function<void(CharSequence&, int /*start*/, int /*count*/, int /*after*/)> beforeTextChanged;
    std::function<void(CharSequence&, int /*start*/, int /*count*/, int /*after*/)> onTextChanged;
    std::function<void(Editable&)> afterTextChanged;
};
}/*endof namespace*/
#endif/*__TEXTWATCHER_H__*/
