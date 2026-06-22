#ifndef __SPAN_WATCHER_H__
#define __SPAN_WATCHER_H__

#include <text/parcelablespan.h>
#include <text/spannablestring.h>

namespace cdroid {

class SpanWatcher : public NoCopySpan {
protected:
    std::shared_ptr<void*>mID;
public:
    SpanWatcher(){
        mID = std::make_shared<void*>(this);
    }
    SpanWatcher(const SpanWatcher&other){
        mID = other.mID;
    }
    SpanWatcher& operator=(const SpanWatcher&other){
        mID = other.mID;
        return *this;
    }
    bool operator == (const SpanWatcher&other)const{
        return mID == other.mID;
    }
    bool operator != (const SpanWatcher&other)const{
        return mID != other.mID;
    }

    virtual ~SpanWatcher() = default;
    //virtual void onSpanAdded(Spanned& text, const ParcelableSpan* what, int start, int end) = 0;
    //virtual void onSpanRemoved(Spanned& text, const ParcelableSpan* what, int start, int end) = 0;
    //virtual void onSpanChanged(Spanned& text, const ParcelableSpan* what, int ostart, int oend, int nstart, int nend) = 0;
    std::function<void(Spanned&,const ParcelableSpan*,int,int)>onSpanAdded;
    std::function<void(Spanned&,const ParcelableSpan*,int,int)>onSpanRemoved;
    std::function<void(Spanned&,const ParcelableSpan*,int,int,int,int)>onSpanChanged;
};

}/* namespace cdroid */
#endif/*__SPAN_WATCHER_H__*/
