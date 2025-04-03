#ifndef __XML_PULLPARSER_H__
#define __XML_PULLPARSER_H__
#include <string>
#include <queue>
#include <memory>
#include <core/attributeset.h>
namespace cdroid{
class XmlPullParser:public AttributeSet{
public:
    enum EventType {
        BAD_DOCUMENT,
        START_DOCUMENT,
        END_DOCUMENT,

        START_TAG,
        END_TAG,
        TEXT,
        COMMENT
    };
private:
    class AttrParser;
    struct Private* mData;
    XmlPullParser();
public:
    XmlPullParser(const std::string&);
    XmlPullParser(Context*ctx,const std::string&resid);
    XmlPullParser(Context*,std::unique_ptr<std::istream>);
    ~XmlPullParser();
    int getDepth()const;
    std::string getName()const;
    std::string getText()const;
    std::string getPositionDescription()const;
    int getEventType()const;
    int getLineNumber()const;
    int getColumnNumber()const;
    int next();
    operator bool()const;
};
}
#endif /*__XML_PULLPARSER_H__*/
