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
        START_TAG,
        END_TAG,
        TEXT,
        END_DOCUMENT
    };
    struct XmlEvent {
        EventType type;
        std::string name;
        std::string text;
        AttributeSet attributes;
    };
private:
    class AttrParser;
    struct Private* mData;
    bool readChunk();
public:
    XmlPullParser();
    XmlPullParser(Context*ctx,const std::string&resid);
    ~XmlPullParser();
    void setContent(Context*ctx,const std::string&resid);
    int getDepth()const;
    std::string getName()const;
    int next(XmlEvent& event,int&depth);
    int next(XmlEvent& event);
    operator bool()const;
};
}
#endif /*__XML_PULLPARSER_H__*/
