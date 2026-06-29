#ifndef __TRANSFORMATION_METHOD_H__
#define __TRANSFORMATION_METHOD_H__
#include <text/parcelablespan.h>
namespace cdroid{
class TransformationMethod:public NoCopySpan{
public:
    virtual ~TransformationMethod()=default;
    virtual CharSequence* getTransformation(CharSequence& source, View& view)=0;

    virtual void onFocusChanged(View& view, CharSequence& sourceText,bool focused,
            int direction, const Rect& previouslyFocusedRect)=0;
};

class TransformationMethod2:public TransformationMethod {
public:
    //@UnsupportedAppUsage
    virtual void setLengthChangesAllowed(bool allowLengthChanges){};
};
}/*endof namespace*/
#endif/*__TRANSFORMATION_METHOD_H__*/

