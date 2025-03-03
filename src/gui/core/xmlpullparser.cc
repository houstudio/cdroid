#include <core/xmlpullparser.h>
#include <expat.h>
#include <iostream>
#include <fstream>
namespace cdroid{

class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        XmlEvent event;
        event.type = START_ELEMENT;
        event.name = name;
        event.attributes.set(attrs);
        XmlPullParser*pull=static_cast<XmlPullParser*>(userData);
        pull->eventQueue.push(event);
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        XmlEvent event;
        event.type = END_ELEMENT;
        event.name = name;
        static_cast<XmlPullParser*>(userData)->eventQueue.push(event);
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        XmlEvent event;
        event.type = TEXT;
        event.text.assign(s, len);
        static_cast<XmlPullParser*>(userData)->eventQueue.push(event);
    }
};

XmlPullParser::XmlPullParser() : endDocument(false) {
    parser = XML_ParserCreateNS(NULL,' ');
    XML_SetUserData((XML_Parser)parser, this);
    XML_SetElementHandler((XML_Parser)parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler((XML_Parser)parser, AttrParser::characterDataHandler);
}

XmlPullParser::~XmlPullParser() {
    XML_ParserFree((XML_Parser)parser);
}

bool XmlPullParser::parse(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (XML_Parse((XML_Parser)parser, content.c_str(), content.size(), XML_TRUE) == XML_STATUS_ERROR) {
        std::cerr << "XML Parse error: " << XML_ErrorString(XML_GetErrorCode((XML_Parser)parser)) << std::endl;
        return false;
    }

    endDocument = true;
    eventQueue.push({END_DOCUMENT, "", "", {}});
    return true;
}

bool XmlPullParser::next(XmlEvent& event) {
    if (eventQueue.empty()) {
        return false;
    }

    event = eventQueue.front();
    eventQueue.pop();
    return true;
}

}/*endof namespace*/
