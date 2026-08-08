// Runtime integration test: verify CDROID's XmlPullParser correctly parses
// binary AXML via the detectBinary/feedFromAxml path (the integration added
// to xmlpullparser.cc). Feeds the known axml_fixture (LinearLayout+TextView
// with typed attrs) through XmlPullParser and checks events/attrs match.
//
// Build: make -C outX64-Debug androidfw_test (picks up this file via GLOB)
#include "gtest/gtest.h"
#include "core/xmlpullparser.h"
#include "androidfw/axml_fixture.h"  // kAXML: binary AXML (LinearLayout+TextView)

#include <sstream>
#include <memory>

using cdroid::XmlPullParser;

namespace {

// Helper: walk a XmlPullParser and collect START_TAG names + their attribute count.
struct TagInfo {
    std::string name;
    int attrCount;
};

std::vector<TagInfo> walkParser(XmlPullParser& parser) {
    std::vector<TagInfo> tags;
    int eventType = parser.next();  // skip START_DOCUMENT
    while (eventType != XmlPullParser::END_DOCUMENT && eventType != XmlPullParser::BAD_DOCUMENT) {
        if (eventType == XmlPullParser::START_TAG) {
            tags.push_back({parser.getName(), 0});
            // Count attributes via AttributeSet.
            const auto& attrs = static_cast<cdroid::AttributeSet&>(parser);
            tags.back().attrCount = attrs.getAttributeCount();
        }
        eventType = parser.next();
    }
    return tags;
}

} // namespace

TEST(XmlPullParserAxmlTest, ParsesBinaryAxml) {
    // Wrap the binary AXML fixture in an istream.
    auto stream = std::make_unique<std::istringstream>(
        std::string((const char*)kAXML, kAXMLLen));
    XmlPullParser parser(nullptr, std::move(stream));

    ASSERT_TRUE(parser);  // operator bool — binary tree loaded OK

    auto tags = walkParser(parser);
    ASSERT_EQ(tags.size(), 2u);                        // LinearLayout + TextView
    EXPECT_EQ(tags[0].name, "LinearLayout");
    EXPECT_EQ(tags[0].attrCount, 9);                   // 7 app: + 2 android:
    EXPECT_EQ(tags[1].name, "TextView");
    EXPECT_EQ(tags[1].attrCount, 3);                   // 1 app: + 2 android:
}

TEST(XmlPullParserAxmlTest, ReadsAttributeValue) {
    auto stream = std::make_unique<std::istringstream>(
        std::string((const char*)kAXML, kAXMLLen));
    XmlPullParser parser(nullptr, std::move(stream));
    parser.next();  // START_DOCUMENT

    // Walk to the first START_TAG (LinearLayout).
    while (parser.next() != XmlPullParser::START_TAG) {}
    const auto& attrs = static_cast<cdroid::AttributeSet&>(parser);

    // app:intdec="42" should be readable as a string attribute.
    // The binary AXML stores the rawValue "42".
    std::string val = attrs.getAttributeValue("intdec");
    EXPECT_FALSE(val.empty());  // some value present
}
