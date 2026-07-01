#include <core/fontlistparser.h>
#include <core/xmlpullparser.h>
#include <fstream>
#include <string>

namespace cdroid {

static std::string attr(XmlPullParser& p, const char* key) {
    return p.hasAttribute(key) ? p.getAttributeValue(key) : std::string();
}

static int parseInt(XmlPullParser& p, const char* key, int defVal) {
    if (!p.hasAttribute(key)) return defVal;
    try { return std::stoi(p.getAttributeValue(key)); }
    catch (...) { return defVal; }
}

static float parseFloat(XmlPullParser& p, const char* key, float defVal) {
    if (!p.hasAttribute(key)) return defVal;
    try { return std::stof(p.getAttributeValue(key)); }
    catch (...) { return defVal; }
}

// Advance to the next START_TAG. Returns false on END_DOCUMENT.
static bool nextStartTag(XmlPullParser& p) {
    int ev;
    while ((ev = p.next()) != XmlPullParser::END_DOCUMENT) {
        if (ev == XmlPullParser::START_TAG) return true;
    }
    return false;
}

// Consume the current element (parser at its START_TAG) through its matching END_TAG.
static void skipElement(XmlPullParser& p) {
    int depth = 1;
    int ev;
    while (depth > 0 && (ev = p.next()) != XmlPullParser::END_DOCUMENT) {
        if (ev == XmlPullParser::START_TAG) depth++;
        else if (ev == XmlPullParser::END_TAG) depth--;
    }
}

static void readAxis(XmlPullParser& p, std::vector<FontVariationAxis>& axes) {
    FontVariationAxis axis;
    axis.tag = attr(p, "tag");
    axis.styleValue = parseFloat(p, "stylevalue", 0.f);
    skipElement(p);  // <axis .../> is empty
    axes.push_back(axis);
}

static FontConfig::Font readFont(XmlPullParser& p, const std::string& fontDir) {
    FontConfig::Font f;
    f.weight = parseInt(p, "weight", 400);
    f.italic = p.hasAttribute("style") && p.getAttributeValue("style") == "italic";
    f.index = parseInt(p, "index", 0);
    std::string text;
    int ev;
    while ((ev = p.next()) != XmlPullParser::END_DOCUMENT) {
        if (ev == XmlPullParser::TEXT) {
            text += p.getText();
        } else if (ev == XmlPullParser::START_TAG) {
            if (p.getName() == "axis") readAxis(p, f.axes);
            else skipElement(p);
        } else if (ev == XmlPullParser::END_TAG) {
            break;  // end of <font>
        }
    }
    auto a = text.find_first_not_of(" \t\r\n");
    if (a != std::string::npos) {
        auto b = text.find_last_not_of(" \t\r\n");
        f.fontName = text.substr(a, b - a + 1);
    }
    f.fileName = fontDir + f.fontName;
    return f;
}

static FontConfig::Family readFamily(XmlPullParser& p, const std::string& fontDir) {
    FontConfig::Family fam;
    fam.name = attr(p, "name");   // empty => fallback family
    fam.lang = attr(p, "lang");
    int ev;
    while ((ev = p.next()) != XmlPullParser::END_DOCUMENT) {
        if (ev == XmlPullParser::START_TAG) {
            if (p.getName() == "font") fam.fonts.push_back(readFont(p, fontDir));
            else skipElement(p);
        } else if (ev == XmlPullParser::END_TAG) {
            break;  // end of <family>
        }
    }
    return fam;
}

static FontConfig::Alias readAlias(XmlPullParser& p) {
    FontConfig::Alias a;
    a.name = attr(p, "name");
    a.to = attr(p, "to");
    a.weight = parseInt(p, "weight", 0);
    skipElement(p);
    return a;
}

FontConfig parseFontConfig(const std::string& xmlPath, const std::string& fontDir) {
    FontConfig cfg;
    auto stream = std::make_unique<std::ifstream>(xmlPath);
    if (!stream->good()) return cfg;  // file missing -> empty config (caller falls back)

    XmlPullParser parser(nullptr, std::move(stream));
    if (!nextStartTag(parser)) return cfg;  // position at <familyset>

    int ev;
    while ((ev = parser.next()) != XmlPullParser::END_DOCUMENT) {
        if (ev == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "family") cfg.families.push_back(readFamily(parser, fontDir));
            else if (tag == "alias") cfg.aliases.push_back(readAlias(parser));
            else skipElement(parser);  // <family-list> and other unknowns
        }
        // END_TAG / TEXT / COMMENT between top-level elements: ignore.
    }
    return cfg;
}

}  // namespace cdroid
