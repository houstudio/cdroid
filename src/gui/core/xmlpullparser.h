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
        START_ELEMENT,
        END_ELEMENT,
        TEXT,
        END_DOCUMENT
    };

    struct XmlEvent {
        EventType type;
        std::string name;
        std::string text;
        AttributeSet attributes;
    };

    XmlPullParser();
    ~XmlPullParser();
    bool parse(const std::string& filename);
    bool next(XmlEvent& event);
private:
    class AttrParser;
    void* parser;
    std::queue<XmlEvent> eventQueue;
    bool endDocument;
};
}
#endif /*__XML_PULLPARSER_H__*/
