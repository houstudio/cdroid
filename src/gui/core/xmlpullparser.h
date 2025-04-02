#ifndef __XML_PULLPARSER_H__
#define __XML_PULLPARSER_H__
#include <string>
#include <queue>
#include <memory>
#include <core/attributeset.h>
namespace cdroid{
class XmlPullParser {
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
    Context*getContext()const;
    std::string getPackage()const;
    int getDepth()const;
    std::string getName()const;
    std::string getText()const;
    std::string getPositionDescription()const;
    int getEventType()const;
    int getLineNumber()const;
    int getColumnNumber()const;
    int next();
    int getAttributeCount()const;
    std::string getAttributeValue(const std::string&key);
    bool getAttribute(int idx,std::string&key,std::string&value)const;
    bool hasAttribute(const std::string&key)const;
    operator bool()const;
};
}
#endif /*__XML_PULLPARSER_H__*/
