#include <text/html.h>
#include <text/textutils.h>
#include <text/style/alignmentspan.h>
#include <text/style/characterstyles.h>
#include <text/style/clickablespan.h>
#include <text/style/leadingmarginspan.h>
#include <text/style/linebackgroundspan.h>
#include <text/style/lineheightspan.h>
#include <text/style/replacementspan.h>
#include <text/style/paragraphstyles.h>
#include <text/style/tabstopspan.h>
#include <text/style/wraptogetherspan.h>
#include <expat.h>
#include <text/spannablestringbuilder.h>
#include <core/color.h>
#include <regex>
#include <cstdint>
#include <initializer_list>
namespace cdroid{

// Concrete XMLReader. Forward-declared in html.h so the public TagHandler
// signature (which takes an XMLReader&) compiles; defined here because
// Html::fromHtml drives parsing directly via expat. As in Android, real
// TagHandler implementations only use the Editable and the tag name, so this
// is intentionally minimal.
class XMLReader {
public:
    XMLReader() = default;
    virtual ~XMLReader() = default;
};

namespace {

// HTML named entities expat (an XML parser) does not know about. Expanded to
// numeric character references before parsing. XML predefined entities
// (&amp; &lt; &gt; &quot; &apos;) and numeric refs (&#NNN;) are left for expat.
struct HtmlEntity { const char* name; uint32_t cp; };
static const HtmlEntity kHtmlEntities[] = {
    {"nbsp",160},{"copy",169},{"reg",174},{"trade",8482},{"mdash",8212},
    {"ndash",8211},{"hellip",8230},{"laquo",171},{"raquo",187},{"times",215},
    {"divide",247},{"deg",176},{"plusmn",177},{"para",182},{"middot",183},
    {"frac12",189},{"frac14",188},{"frac34",190},{"sup2",178},{"sup3",179},
    {"sup1",185},{"euro",8364},{"pound",163},{"yen",165},{"cent",162},
    {"sect",167},{"bull",8226},{"dagger",8224},{"Dagger",8225},{"lsquo",8216},
    {"rsquo",8217},{"ldquo",8220},{"rdquo",8221},{"szlig",223},{"agrave",224},
    {"aacute",225},{"acirc",226},{"atilde",227},{"auml",228},{"aring",229},
    {"aelig",230},{"ccedil",231},{"egrave",232},{"eacute",233},{"ecirc",234},
    {"euml",235},{"igrave",236},{"iacute",237},{"icirc",238},{"iuml",239},
    {"ntilde",241},{"ograve",242},{"oacute",243},{"ocirc",244},{"otilde",245},
    {"ouml",246},{"oslash",248},{"ugrave",249},{"uacute",250},{"ucirc",251},
    {"uuml",252},{"yacute",253},{"yuml",255},{"Agrave",192},{"Aacute",193},
    {"Acirc",194},{"Atilde",195},{"Auml",196},{"Aring",197},{"AElig",198},
    {"Ccedil",199},{"Egrave",200},{"Eacute",201},{"Ecirc",202},{"Euml",203},
    {"Igrave",204},{"Iacute",205},{"Icirc",206},{"Iuml",207},{"Ntilde",209},
    {"Ograve",210},{"Oacute",211},{"Ocirc",212},{"Otilde",213},{"Ouml",214},
    {"Oslash",216},{"Ugrave",217},{"Uacute",218},{"Ucirc",219},{"Uuml",220},
    {"Yacute",221}
};

std::string expandHtmlEntities(const std::string& src) {
    std::string out;
    out.reserve(src.size());
    const size_t n = src.size();
    for (size_t i = 0; i < n; ) {
        if (src[i] == '&') {
            size_t semi = src.find(';', i + 1);
            if (semi != std::string::npos && semi - (i + 1) <= 8) {
                std::string name = src.substr(i + 1, semi - (i + 1));
                if (!name.empty() && name[0] != '#') {
                    bool matched = false;
                    for (const auto& e : kHtmlEntities) {
                        if (name == e.name) {
                            out += "&#";
                            out += std::to_string(e.cp);
                            out += ';';
                            i = semi + 1;
                            matched = true;
                            break;
                        }
                    }
                    if (matched) continue;
                }
            }
        }
        out += src[i++];
    }
    return out;
}

// Decode a UTF-8 byte run into UTF-16 (char16_t, with surrogate pairs).
std::u16string utf8ToU16(const char* s, int len) {
    std::u16string out;
    for (int i = 0; i < len; ) {
        unsigned char c = (unsigned char)s[i];
        uint32_t cp;
        int extra;
        if (c < 0x80)               { cp = c;        extra = 0; }
        else if ((c & 0xE0) == 0xC0){ cp = c & 0x1F; extra = 1; }
        else if ((c & 0xF0) == 0xE0){ cp = c & 0x0F; extra = 2; }
        else if ((c & 0xF8) == 0xF0){ cp = c & 0x07; extra = 3; }
        else { out.push_back((char16_t)c); ++i; continue; }
        for (int j = 1; j <= extra && i + j < len; ++j)
            cp = (cp << 6) | (((unsigned char)s[i + j]) & 0x3F);
        i += 1 + extra;
        if (cp <= 0xFFFF) {
            out.push_back((char16_t)cp);
        } else {
            cp -= 0x10000;
            out.push_back((char16_t)(0xD800 | (cp >> 10)));
            out.push_back((char16_t)(0xDC00 | (cp & 0x3FF)));
        }
    }
    return out;
}

inline bool equalsIgnoreCase(const std::string& a, const char* b) {
    size_t i = 0;
    for (; i < a.size() && b[i]; ++i) {
        char x = (a[i] >= 'a' && a[i] <= 'z') ? char(a[i] - 32) : a[i];
        char y = (b[i] >= 'a' && b[i] <= 'z') ? char(b[i] - 32) : b[i];
        if (x != y) return false;
    }
    return i == a.size() && b[i] == 0;
}
inline bool equalsIgnoreCase(const std::string& a, const std::string& b) {
    return equalsIgnoreCase(a, b.c_str());
}

// Case-insensitive lookup over expat's null-terminated name/value attribute pairs.
std::string getAttr(const XML_Char** atts, const char* name) {
    if (!atts) return std::string();
    for (int i = 0; atts[i]; i += 2) {
        if (equalsIgnoreCase(atts[i], name))
            return atts[i + 1] ? std::string(atts[i + 1]) : std::string();
    }
    return std::string();
}

// Sentinel root element so fragments with several top-level blocks parse.
constexpr const char* kHtmlRootTag = "cdroid-html-root";

// SAX-style converter. Expat's start/end/character callbacks are the analog of
// Android's ContentHandler. Tags drop inert "mark" bookmarks; real spans are
// created only at the matching end tag via setSpanFromMark().
class HtmlToSpannedConverter {
public:
    HtmlToSpannedConverter(const std::string& source, Html::ImageGetter imageGetter,
                           Html::TagHandler tagHandler, int flags)
        : mSource(source)
        , mImageGetter(std::move(imageGetter))
        , mTagHandler(std::move(tagHandler))
        , mFlags(flags) {}

    Spanned* convert() {
        XML_Parser parser = XML_ParserCreate(nullptr); // UTF-8, no namespaces
        XML_SetUserData(parser, this);
        XML_SetElementHandler(parser, &HtmlToSpannedConverter::startThunk,
                                     &HtmlToSpannedConverter::endThunk);
        XML_SetCharacterDataHandler(parser, &HtmlToSpannedConverter::charsThunk);

        std::string body = expandHtmlEntities(mSource);
        const std::string rootOpen  = std::string("<") + kHtmlRootTag + ">";
        const std::string rootClose = std::string("</") + kHtmlRootTag + ">";
        bool ok = true;
        auto feed = [&](const char* p, int len, int isFinal) {
            if (ok && XML_Parse(parser, p, len, isFinal) == XML_STATUS_ERROR) ok = false;
        };
        feed(rootOpen.data(),  (int)rootOpen.size(),  XML_FALSE);
        feed(body.data(),      (int)body.size(),      XML_FALSE);
        feed(rootClose.data(), (int)rootClose.size(), XML_TRUE);
        XML_ParserFree(parser);

        // Fix flags and range for paragraph-type markup.
        auto paragraphs = mBuilder.getSpans(0, (int)mBuilder.length(),
                                            make_span_filter<ParagraphStyle>());
        for (auto span : paragraphs) {
            int start = mBuilder.getSpanStart(span);
            int end   = mBuilder.getSpanEnd(span);
            if (end - 2 >= 0) {
                if (mBuilder.charAt(end - 1) == '\n' &&
                    mBuilder.charAt(end - 2) == '\n') {
                    end--;
                }
            }
            // CDROID's setSpan appends rather than replacing an existing span,
            // so drop the old entry first to mirror Android's replace-on-setSpan.
            mBuilder.removeSpan(span);
            if (end != start) {
                mBuilder.setSpan(span, start, end, Spanned::SPAN_PARAGRAPH);
            }
        }
        return new SpannableStringBuilder(mBuilder);
    }

private:
    static void startThunk(void* ud, const XML_Char* name, const XML_Char** atts) {
        static_cast<HtmlToSpannedConverter*>(ud)->startElement(name, atts);
    }
    static void endThunk(void* ud, const XML_Char* name) {
        static_cast<HtmlToSpannedConverter*>(ud)->endElement(name);
    }
    static void charsThunk(void* ud, const XML_Char* s, int len) {
        static_cast<HtmlToSpannedConverter*>(ud)->characters(s, len);
    }

    void startElement(const XML_Char* name, const XML_Char** atts) {
        std::string tag(name);
        if (!equalsIgnoreCase(tag, kHtmlRootTag)) handleStartTag(tag, atts);
    }
    void endElement(const XML_Char* name) {
        std::string tag(name);
        if (!equalsIgnoreCase(tag, kHtmlRootTag)) handleEndTag(tag);
    }
    void characters(const XML_Char* s, int len) {
        // Collapse whitespace runs the way Android's characters() does.
        std::u16string decoded = utf8ToU16(s, len);
        for (char16_t c : decoded) {
            if (c == u' ' || c == u'\n') {
                int blen = (int)mBuilder.length();
                char16_t pred = (blen == 0) ? u'\n' : (char16_t)mBuilder.charAt(blen - 1);
                if (pred != u' ' && pred != u'\n') mBuilder.append((char16_t)u' ');
            } else {
                mBuilder.append(c);
            }
        }
    }

    static float headingSize(int level) {
        static const float kSizes[6] = {1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1.0f};
        return kSizes[level];
    }

    // ---- mark objects: inert ParcelableSpan bookmarks (members are public). ----
    struct Bold         : ParcelableSpan {};
    struct Italic       : ParcelableSpan {};
    struct Underline    : ParcelableSpan {};
    struct Strikethrough: ParcelableSpan {};
    struct Big          : ParcelableSpan {};
    struct Small        : ParcelableSpan {};
    struct Monospace    : ParcelableSpan {};
    struct Blockquote   : ParcelableSpan {};
    struct Super        : ParcelableSpan {};
    struct Sub          : ParcelableSpan {};
    struct Bullet       : ParcelableSpan {};
    struct Font         : ParcelableSpan { std::string mFace; explicit Font(const std::string& f):mFace(f){} };
    struct Href         : ParcelableSpan { std::string mHref; explicit Href(const std::string& h):mHref(h){} };
    struct Foreground   : ParcelableSpan { int mColor; explicit Foreground(int c):mColor(c){} };
    struct Background   : ParcelableSpan { int mColor; explicit Background(int c):mColor(c){} };
    struct Heading      : ParcelableSpan { int mLevel; explicit Heading(int l):mLevel(l){} };
    struct Newline      : ParcelableSpan { int mNum;  explicit Newline(int n):mNum(n){}  };
    struct Align        : ParcelableSpan { Layout::Alignment mAlign; explicit Align(Layout::Alignment a):mAlign(a){} };

    std::string mSource;
    Html::ImageGetter mImageGetter;
    Html::TagHandler mTagHandler;
    int mFlags;
    XMLReader mReader;
    SpannableStringBuilder mBuilder;

    static const std::regex& textAlignRe() {
        static const std::regex re(R"((?:^|\s+)text-align\s*:\s*([^\s;]+))", std::regex::icase);
        return re;
    }
    static const std::regex& colorRe() {
        static const std::regex re(R"((?:^|\s+)color\s*:\s*([^\s;]+))", std::regex::icase);
        return re;
    }
    static const std::regex& backgroundRe() {
        static const std::regex re(R"((?:^|\s+)background(?:-color)?\s*:\s*([^\s;]+))", std::regex::icase);
        return re;
    }
    static const std::regex& textDecorationRe() {
        static const std::regex re(R"((?:^|\s+)text-decoration\s*:\s*([^\s;]+))", std::regex::icase);
        return re;
    }
    static const std::regex& verticalAlignRe() {
        static const std::regex re(R"((?:^|\s+)vertical-align\s*:\s*([^\s;]+))", std::regex::icase);
        return re;
    }

    int getFontWeightAdjustment() const { return 0; }

    int getMargin(int flag) const { return (flag & mFlags) != 0 ? 1 : 2; }
    int getMarginParagraph() const  { return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH); }
    int getMarginHeading() const    { return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_HEADING); }
    int getMarginListItem() const   { return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM); }
    int getMarginList() const       { return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_LIST); }
    int getMarginDiv() const        { return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_DIV); }
    int getMarginBlockquote() const { return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE); }

    static void appendNewlines(SpannableStringBuilder& text, int minNewline) {
        int len = (int)text.length();
        if (len == 0) return;
        int existing = 0;
        for (int i = len - 1; i >= 0 && text.charAt(i) == '\n'; --i) existing++;
        for (int j = existing; j < minNewline; ++j) text.append((char16_t)u'\n');
    }

    static void start(SpannableStringBuilder& text, const ParcelableSpan* mark) {
        int len = (int)text.length();
        text.setSpan(mark, len, len, Spanned::SPAN_INCLUSIVE_EXCLUSIVE);
    }
    static void setSpanFromMark(SpannableStringBuilder& text, const ParcelableSpan* mark,
                                std::initializer_list<const ParcelableSpan*> spans) {
        int where = text.getSpanStart(mark);
        text.removeSpan(mark);
        int len = (int)text.length();
        if (where != len) {
            for (auto sp : spans) text.setSpan(sp, where, len, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
        }
    }
    template<typename T>
    static const T* getLast(const SpannableStringBuilder& text) {
        auto objs = text.getSpans(0, (int)text.length(), make_span_filter<T>());
        if (objs.empty()) return nullptr;
        return static_cast<const T*>(objs.back());
    }
    template<typename T>
    static void end(SpannableStringBuilder& text, const ParcelableSpan* repl) {
        const T* obj = getLast<T>(text);
        if (obj) setSpanFromMark(text, obj, { repl });
    }

    static bool isHeadingTag(const std::string& tag) {
        return tag.size() == 2 && (tag[0] == 'h' || tag[0] == 'H')
            && tag[1] >= '1' && tag[1] <= '6';
    }

    void handleStartTag(const std::string& tag, const XML_Char** atts) {
        if (equalsIgnoreCase(tag, "br")) {
            // newline emitted on the end tag
        } else if (equalsIgnoreCase(tag, "p")) {
            startBlockElement(mBuilder, atts, getMarginParagraph());
            startCssStyle(mBuilder, atts);
        } else if (equalsIgnoreCase(tag, "ul")) {
            startBlockElement(mBuilder, atts, getMarginList());
        } else if (equalsIgnoreCase(tag, "li")) {
            startLi(mBuilder, atts);
        } else if (equalsIgnoreCase(tag, "div")) {
            startBlockElement(mBuilder, atts, getMarginDiv());
        } else if (equalsIgnoreCase(tag, "span")) {
            startCssStyle(mBuilder, atts);
        } else if (equalsIgnoreCase(tag, "strong") || equalsIgnoreCase(tag, "b")) {
            start(mBuilder, new Bold());
        } else if (equalsIgnoreCase(tag, "em") || equalsIgnoreCase(tag, "cite") ||
                   equalsIgnoreCase(tag, "dfn") || equalsIgnoreCase(tag, "i")) {
            start(mBuilder, new Italic());
        } else if (equalsIgnoreCase(tag, "big")) {
            start(mBuilder, new Big());
        } else if (equalsIgnoreCase(tag, "small")) {
            start(mBuilder, new Small());
        } else if (equalsIgnoreCase(tag, "font")) {
            startFont(mBuilder, atts);
        } else if (equalsIgnoreCase(tag, "blockquote")) {
            startBlockquote(mBuilder, atts);
        } else if (equalsIgnoreCase(tag, "tt")) {
            start(mBuilder, new Monospace());
        } else if (equalsIgnoreCase(tag, "a")) {
            startA(mBuilder, atts);
        } else if (equalsIgnoreCase(tag, "u")) {
            start(mBuilder, new Underline());
        } else if (equalsIgnoreCase(tag, "del") || equalsIgnoreCase(tag, "s") ||
                   equalsIgnoreCase(tag, "strike")) {
            start(mBuilder, new Strikethrough());
        } else if (equalsIgnoreCase(tag, "sup")) {
            start(mBuilder, new Super());
        } else if (equalsIgnoreCase(tag, "sub")) {
            start(mBuilder, new Sub());
        } else if (isHeadingTag(tag)) {
            startHeading(mBuilder, atts, tag[1] - '1');
        } else if (equalsIgnoreCase(tag, "img")) {
            startImg(atts);
        } else if (mTagHandler) {
            mTagHandler(true, tag, mBuilder, mReader);
        }
    }

    void handleEndTag(const std::string& tag) {
        if (equalsIgnoreCase(tag, "br")) {
            mBuilder.append((char16_t)u'\n');
        } else if (equalsIgnoreCase(tag, "p")) {
            endCssStyle(mBuilder);
            endBlockElement(mBuilder);
        } else if (equalsIgnoreCase(tag, "ul")) {
            endBlockElement(mBuilder);
        } else if (equalsIgnoreCase(tag, "li")) {
            endLi(mBuilder);
        } else if (equalsIgnoreCase(tag, "div")) {
            endBlockElement(mBuilder);
        } else if (equalsIgnoreCase(tag, "span")) {
            endCssStyle(mBuilder);
        } else if (equalsIgnoreCase(tag, "strong") || equalsIgnoreCase(tag, "b")) {
            end<Bold>(mBuilder, new StyleSpan(Typeface::BOLD, getFontWeightAdjustment()));
        } else if (equalsIgnoreCase(tag, "em") || equalsIgnoreCase(tag, "cite") ||
                   equalsIgnoreCase(tag, "dfn") || equalsIgnoreCase(tag, "i")) {
            end<Italic>(mBuilder, new StyleSpan(Typeface::ITALIC));
        } else if (equalsIgnoreCase(tag, "big")) {
            end<Big>(mBuilder, new RelativeSizeSpan(1.25f));
        } else if (equalsIgnoreCase(tag, "small")) {
            end<Small>(mBuilder, new RelativeSizeSpan(0.8f));
        } else if (equalsIgnoreCase(tag, "font")) {
            endFont(mBuilder);
        } else if (equalsIgnoreCase(tag, "blockquote")) {
            endBlockquote(mBuilder);
        } else if (equalsIgnoreCase(tag, "tt")) {
            end<Monospace>(mBuilder, new TypefaceSpan("monospace"));
        } else if (equalsIgnoreCase(tag, "a")) {
            endA(mBuilder);
        } else if (equalsIgnoreCase(tag, "u")) {
            end<Underline>(mBuilder, new UnderlineSpan());
        } else if (equalsIgnoreCase(tag, "del") || equalsIgnoreCase(tag, "s") ||
                   equalsIgnoreCase(tag, "strike")) {
            end<Strikethrough>(mBuilder, new StrikethroughSpan());
        } else if (equalsIgnoreCase(tag, "sup")) {
            end<Super>(mBuilder, new SuperscriptSpan());
        } else if (equalsIgnoreCase(tag, "sub")) {
            end<Sub>(mBuilder, new SubscriptSpan());
        } else if (isHeadingTag(tag)) {
            endHeading(mBuilder);
        } else if (mTagHandler) {
            mTagHandler(false, tag, mBuilder, mReader);
        }
    }

    void startBlockElement(SpannableStringBuilder& text, const XML_Char** atts, int margin) {
        if (margin > 0) {
            appendNewlines(text, margin);
            start(text, new Newline(margin));
        }
        std::string style = getAttr(atts, "style");
        std::smatch m;
        if (!style.empty() && std::regex_search(style, m, textAlignRe())) {
            const std::string& a = m[1].str();
            if (equalsIgnoreCase(a, "start") || equalsIgnoreCase(a, "left"))
                start(text, new Align(Layout::Alignment::ALIGN_NORMAL));
            else if (equalsIgnoreCase(a, "center"))
                start(text, new Align(Layout::Alignment::ALIGN_CENTER));
            else if (equalsIgnoreCase(a, "end") || equalsIgnoreCase(a, "right"))
                start(text, new Align(Layout::Alignment::ALIGN_OPPOSITE));
        }
    }
    void endBlockElement(SpannableStringBuilder& text) {
        const Newline* n = getLast<Newline>(text);
        if (n) { appendNewlines(text, n->mNum); text.removeSpan(n); }
        const Align* a = getLast<Align>(text);
        if (a) setSpanFromMark(text, a, { new AlignmentSpan::Standard(a->mAlign) });
    }

    void startLi(SpannableStringBuilder& text, const XML_Char** atts) {
        startBlockElement(text, atts, getMarginListItem());
        start(text, new Bullet());
        startCssStyle(text, atts);
    }
    void endLi(SpannableStringBuilder& text) {
        endCssStyle(text);
        endBlockElement(text);
        end<Bullet>(text, new BulletSpan());
    }

    void startBlockquote(SpannableStringBuilder& text, const XML_Char** atts) {
        startBlockElement(text, atts, getMarginBlockquote());
        start(text, new Blockquote());
    }
    void endBlockquote(SpannableStringBuilder& text) {
        endBlockElement(text);
        end<Blockquote>(text, new QuoteSpan());
    }

    void startHeading(SpannableStringBuilder& text, const XML_Char** atts, int level) {
        startBlockElement(text, atts, getMarginHeading());
        start(text, new Heading(level));
    }
    void endHeading(SpannableStringBuilder& text) {
        const Heading* h = getLast<Heading>(text);
        if (h) {
            setSpanFromMark(text, h, {
                new RelativeSizeSpan(headingSize(h->mLevel)),
                new StyleSpan(Typeface::BOLD, getFontWeightAdjustment())
            });
        }
        endBlockElement(text);
    }

    void startCssStyle(SpannableStringBuilder& text, const XML_Char** atts) {
        std::string style = getAttr(atts, "style");
        if (style.empty()) return;
        std::smatch m;
        if (std::regex_search(style, m, colorRe())) {
            int c = getHtmlColor(m[1].str());
            if (c != -1) start(text, new Foreground(c | 0xFF000000));
        }
        if (std::regex_search(style, m, backgroundRe())) {
            int c = getHtmlColor(m[1].str());
            if (c != -1) start(text, new Background(c | 0xFF000000));
        }
        if (std::regex_search(style, m, textDecorationRe())) {
            if (equalsIgnoreCase(m[1].str(), "line-through")) start(text, new Strikethrough());
        }
    }
    void endCssStyle(SpannableStringBuilder& text) {
        const Strikethrough* s = getLast<Strikethrough>(text);
        if (s) setSpanFromMark(text, s, { new StrikethroughSpan() });
        const Background* b = getLast<Background>(text);
        if (b) setSpanFromMark(text, b, { new BackgroundColorSpan(b->mColor) });
        const Foreground* f = getLast<Foreground>(text);
        if (f) setSpanFromMark(text, f, { new ForegroundColorSpan(f->mColor) });
    }

    void startFont(SpannableStringBuilder& text, const XML_Char** atts) {
        std::string color = getAttr(atts, "color");
        std::string face  = getAttr(atts, "face");
        if (!color.empty()) {
            int c = getHtmlColor(color);
            if (c != -1) start(text, new Foreground(c | 0xFF000000));
        }
        if (!face.empty()) start(text, new Font(face));
    }
    void endFont(SpannableStringBuilder& text) {
        const Font* font = getLast<Font>(text);
        if (font) setSpanFromMark(text, font, { new TypefaceSpan(font->mFace) });
        const Foreground* fg = getLast<Foreground>(text);
        if (fg) setSpanFromMark(text, fg, { new ForegroundColorSpan(fg->mColor) });
    }

    void startA(SpannableStringBuilder& text, const XML_Char** atts) {
        start(text, new Href(getAttr(atts, "href")));
    }
    void endA(SpannableStringBuilder& text) {
        const Href* h = getLast<Href>(text);
        if (!h) return;
        // Reserved: URLSpan is abstract in this port (pure-virtual
        // ClickableSpan::onClick). Re-enable once URLSpan is instantiable:
        //     if (!h->mHref.empty()) setSpanFromMark(text, h, { new URLSpan(h->mHref) });
        setSpanFromMark(text, h, {});
    }

    void startImg(const XML_Char** atts) {
        // Reserved: ImageSpan/ImageGetter are unavailable in this port. Re-enable
        // once ImageSpan exists (mirrors Android's behavior):
        std::string src = getAttr(atts, "src");
        Drawable* d = mImageGetter ? mImageGetter(src) : nullptr;

        // 解析对齐参数：优先使用 img 的 align 属性，其次在 style 中解析 vertical-align
        int valign = DynamicDrawableSpan::ALIGN_BOTTOM;
        std::string align = getAttr(atts, "align");
        if (!align.empty()) {
            if (equalsIgnoreCase(align, "baseline")) valign = DynamicDrawableSpan::ALIGN_BASELINE;
            else if (equalsIgnoreCase(align, "center") || equalsIgnoreCase(align, "middle"))
                valign = DynamicDrawableSpan::ALIGN_CENTER;
            else valign = DynamicDrawableSpan::ALIGN_BOTTOM;
        }
        std::string style = getAttr(atts, "style");
        if (!style.empty()) {
            std::smatch m;
            if (std::regex_search(style, m, verticalAlignRe())) {
                const std::string& va = m[1].str();
                if (equalsIgnoreCase(va, "baseline")) valign = DynamicDrawableSpan::ALIGN_BASELINE;
                else if (equalsIgnoreCase(va, "center") || equalsIgnoreCase(va, "middle"))
                    valign = DynamicDrawableSpan::ALIGN_CENTER;
                else if (equalsIgnoreCase(va, "bottom"))
                    valign = DynamicDrawableSpan::ALIGN_BOTTOM;
            }
        }

        const int len = (int)mBuilder.length();
        mBuilder.append(u'￼');/*0xFFFC*/
        mBuilder.setSpan(new ImageSpan(d, valign), len, (int)mBuilder.length(),
                            Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
        (void)mImageGetter;
    }

    int getHtmlColor(const std::string& color) const {
        if (color.empty()) return -1;
        if ((mFlags & Html::FROM_HTML_OPTION_USE_CSS_COLORS) != 0) {
            struct Nv { const char* n; unsigned int v; };
            static const Nv cssMap[] = {
                {"darkgray",0xFFA9A9A9},{"gray",0xFF808080},{"lightgray",0xFFD3D3D3},
                {"darkgrey",0xFFA9A9A9},{"grey",0xFF808080},{"lightgrey",0xFFD3D3D3},
                {"green",0xFF008000}
            };
            std::string lower = color;
            for (auto& ch : lower) if (ch >= 'A' && ch <= 'Z') ch = char(ch + 32);
            for (const auto& e : cssMap) if (lower == e.n) return (int)e.v;
        }
        try { return (int)Color::parseColor(color); } catch (...) {}
        try { return (int)std::stoul(color, nullptr, 0); }   // 0x.. / decimal
        catch (...) { return -1; }
    }
};

} // anonymous namespace

Spanned* Html::fromHtml(const std::string& source) {
    return fromHtml(source, FROM_HTML_MODE_LEGACY, nullptr, nullptr);
}

Spanned* Html::fromHtml(const std::string& source, int flags) {
    return fromHtml(source, flags, nullptr, nullptr);
}

Spanned* Html::fromHtml(const std::string& source, ImageGetter imageGetter, TagHandler tagHandler) {
    return fromHtml(source, FROM_HTML_MODE_LEGACY, imageGetter, tagHandler);
}

Spanned* Html::fromHtml(const std::string& source, int flags, ImageGetter imageGetter, TagHandler tagHandler) {
    HtmlToSpannedConverter converter(source, std::move(imageGetter), std::move(tagHandler), flags);
    return converter.convert();
}

std::string Html::toHtml(const Spanned& text) {
    return toHtml(text, TO_HTML_PARAGRAPH_LINES_CONSECUTIVE);
}

std::string Html::toHtml(const Spanned& text, int option) {
    std::stringstream out;
    withinHtml(out, text, option);
    return out.str();
}

std::string Html::escapeHtml(const CharSequence& text) {
    std::stringstream out;
    withinStyle(out, text, 0, text.length());
    return out.str();
}

void Html::withinHtml(std::stringstream& out,const Spanned& text, int option) {
    if ((option & TO_HTML_PARAGRAPH_FLAG) == TO_HTML_PARAGRAPH_LINES_CONSECUTIVE) {
        encodeTextAlignmentByDiv(out, text, option);
        return;
    }

    withinDiv(out, text, 0, text.length(), option);
}

void Html::encodeTextAlignmentByDiv(std::stringstream& out,const Spanned& text, int option) {
    int next;
    size_t len = text.length();
    for (size_t i = 0; i < len; i = next) {
        auto kindClass = make_span_filter<ParagraphStyle>();
        next = text.nextSpanTransition(i, len, kindClass);
        auto style = text.getSpans(i, next, kindClass);
        std::string elements = " ";
        bool needDiv = false;

        for(int j = 0; j < style.size(); j++) {
            if (dynamic_cast<const AlignmentSpan*>(style[j])) {
                auto align =(dynamic_cast<const AlignmentSpan*>(style[j]))->getAlignment();
                needDiv = true;
                if (align == Layout::Alignment::ALIGN_CENTER) {
                    elements = "align=\"center\" " + elements;
                } else if (align == Layout::Alignment::ALIGN_OPPOSITE) {
                    elements = "align=\"right\" " + elements;
                } else {
                    elements = "align=\"left\" " + elements;
                }
            }
        }
        if (needDiv) {
            out <<"<div " << elements <<">";
        }

        withinDiv(out, text, i, next, option);

        if (needDiv) {
            out << "</div>";
        }
    }
}

void Html::withinDiv(std::stringstream& out,const Spanned& text, int start, int end, int option) {
    int next;
    for (int i = start; i < end; i = next) {
        next = text.nextSpanTransition(i, end, make_span_filter<QuoteSpan>());
        auto quotes = text.getSpans(i, next, make_span_filter<QuoteSpan>());
        for (auto& quote : quotes) {
            out << "<blockquote>";
        }
        withinBlockquote(out, text, i, next, option);
        for (auto& quote : quotes) {
            out << "</blockquote>\n";
        }
    }
}

std::string Html::getTextDirection(const Spanned& text, int start, int end) {
    if (TextDirectionHeuristics::FIRSTSTRONG_LTR->isRtl(&text, start, end - start)) {
        return " dir=\"rtl\"";
    } else {
        return " dir=\"ltr\"";
    }
}

std::string Html::getTextStyles(const Spanned& text, int start, int end, bool forceNoVerticalMargin, bool includeTextAlign) {
    std::string margin;
    std::string textAlign;

    if (forceNoVerticalMargin) {
        margin = "margin-top:0; margin-bottom:0;";
    }
    if (includeTextAlign) {
        auto alignmentSpans = text.getSpans(start, end, make_span_filter<AlignmentSpan>());

        // Only use the last AlignmentSpan with flag SPAN_PARAGRAPH
        for (int i = alignmentSpans.size() - 1; i >= 0; i--) {
            AlignmentSpan* s = (AlignmentSpan*)alignmentSpans[i];
            if ((text.getSpanFlags(s) & Spanned::SPAN_PARAGRAPH) == Spanned::SPAN_PARAGRAPH) {
                auto alignment = s->getAlignment();
                if (alignment == Layout::Alignment::ALIGN_NORMAL) {
                    textAlign = "text-align:start;";
                } else if (alignment == Layout::Alignment::ALIGN_CENTER) {
                    textAlign = "text-align:center;";
                } else if (alignment == Layout::Alignment::ALIGN_OPPOSITE) {
                    textAlign = "text-align:end;";
                }
                break;
            }
        }
    }

    if (margin.empty() && textAlign.empty()) {
        return "";
    }

    std::stringstream style;
    if (!margin.empty()&& !textAlign.empty()) {
        style << margin <<" " << textAlign;
    } else if (!margin.empty()) {
        style <<margin;
    } else if (!textAlign.empty()) {
        style << textAlign;
    }

    style<<"\"";
    return style.str();
}

void Html::withinBlockquote(std::stringstream& out,const Spanned& text, int start, int end, int option) {
    if ((option & TO_HTML_PARAGRAPH_FLAG) == TO_HTML_PARAGRAPH_LINES_CONSECUTIVE) {
        withinBlockquoteConsecutive(out, text, start, end);
    } else {
        withinBlockquoteIndividual(out, text, start, end);
    }
}

void Html::withinBlockquoteIndividual(std::stringstream& out,const Spanned& text, int start, int end) {
    bool isInList = false;
    int next;
    for (int i = start; i <= end; i = next) {
        next = TextUtils::indexOf(&text, '\n', i, end);
        if (next < 0) {
            next = end;
        }

        if (next == i) {
            if (isInList) {
                // Current paragraph is no longer a list item; close the previously opened list
                isInList = false;
                out << "</ul>\n";
            }
            out << "<br>\n";
        } else {
            bool isListItem = false;
            auto paragraphStyles = text.getSpans(i, next, make_span_filter<ParagraphStyle>());
            for (auto paragraphStyle : paragraphStyles) {
                const int spanFlags = text.getSpanFlags(paragraphStyle);
                if ((spanFlags & Spanned::SPAN_PARAGRAPH) == Spanned::SPAN_PARAGRAPH
                        && dynamic_cast<const BulletSpan*>(paragraphStyle)) {
                    isListItem = true;
                    break;
                }
            }

            if (isListItem && !isInList) {
                // Current paragraph is the first item in a list
                isInList = true;
                out << "<ul" <<getTextStyles(text, i, next, true, false)<<">\n";
            }

            if (isInList && !isListItem) {
                // Current paragraph is no longer a list item; close the previously opened list
                isInList = false;
                out << "</ul>\n";
            }

            std::string tagType = isListItem ? "li" : "p";
            out << "<" << tagType <<getTextDirection(text, i, next)
                    << getTextStyles(text, i, next, !isListItem, true) <<">";

            withinParagraph(out, text, i, next);

            out << "</" << tagType << ">\n";

            if (next == end && isInList) {
                isInList = false;
                out << "</ul>\n";
            }
        }

        next++;
    }
}

void Html::withinBlockquoteConsecutive(std::stringstream& out,const Spanned& text, int start, int end) {
    out << "<p" << getTextDirection(text, start, end) << ">";

    int next;
    for (int i = start; i < end; i = next) {
        next = TextUtils::indexOf(&text, '\n', i, end);
        if (next < 0) {
            next = end;
        }
        int nl = 0;
        while (next < end && text.charAt(next) == '\n') {
            nl++;
            next++;
        }
        withinParagraph(out, text, i, next - nl);
        if (nl == 1) {
            out<<"<br>\n";
        } else {
            for (int j = 2; j < nl; j++) {
                out<<"<br>";
            }
            if (next != end) {
                /* Paragraph should be closed and reopened */
                out<<"</p>\n";
                out<<"<p"<<getTextDirection(text, start, end)<<">";
            }
        }
    }
    out << "</p>\n";
}

float Html::getDisplayMetricsDensity() {
    return 1.f;//ActivityThread.currentApplication().getResources().getDisplayMetrics().density;
}

/*float Html::getDisplayMetricsDensity$ravenwood() {
    return Resources.getSystem().getDisplayMetrics().density;
}*/

void Html::withinParagraph(std::stringstream& out,const Spanned& text, int start, int end) {
    int next;
    for (int i = start; i < end; i = next) {
        next = text.nextSpanTransition(i, end, make_span_filter<CharacterStyle>());
        auto style = text.getSpans(i, next, make_span_filter<CharacterStyle>());

        for (int j = 0; j < style.size(); j++) {
            if (dynamic_cast<const StyleSpan*>(style[j])) {
                int s = dynamic_cast<const StyleSpan*>(style[j])->getStyle();

                if ((s & Typeface::BOLD) != 0) {
                    out<<"<b>";
                }
                if ((s & Typeface::ITALIC) != 0) {
                    out<<"<i>";
                }
            }
            if (dynamic_cast<const TypefaceSpan*>(style[j])) {
                std::string s = dynamic_cast<const TypefaceSpan*>(style[j])->getFamily();

                if (s.compare("monospace")==0) {
                    out<<"<tt>";
                }
            }
            if (dynamic_cast<const SuperscriptSpan*>(style[j])) {
                out<<"<sup>";
            }
            if (dynamic_cast<const SubscriptSpan*>(style[j])) {
                out<<"<sub>";
            }
            if (dynamic_cast<const UnderlineSpan*>(style[j])) {
                out<<"<u>";
            }
            if (dynamic_cast<const StrikethroughSpan*>(style[j])) {
                out<<"<span style=\"text-decoration:line-through;\">";
            }
            if (dynamic_cast<const URLSpan*>(style[j])) {
                out<<"<a href=\"";
                out<<dynamic_cast<const URLSpan*>(style[j])->getURL();
                out<<"\">";
            }
            if (dynamic_cast<const ImageSpan*>(style[j])) {
                out<<"<img src=\"";
                out<<dynamic_cast<const ImageSpan*>(style[j])->getSource();
                out<<"\">";

                // Don't output the placeholder character underlying the image.
                i = next;
            }
            if (dynamic_cast<const AbsoluteSizeSpan*>(style[j])) {
                const AbsoluteSizeSpan* s = dynamic_cast<const AbsoluteSizeSpan*>(style[j]);
                float sizeDip = s->getSize();
                if (!s->getDip()) {
                    sizeDip /= getDisplayMetricsDensity();
                }

                // px in CSS is the equivalance of dip in Android
                out<<TextUtils::stringPrintf("<span style=\"font-size:%.0fpx\";>", sizeDip);
            }
            if (dynamic_cast<const RelativeSizeSpan*>(style[j])) {
                const float sizeEm = dynamic_cast<const RelativeSizeSpan*>(style[j])->getSizeChange();
                out<<TextUtils::stringPrintf("<span style=\"font-size:%.2fem;\">", sizeEm);
            }
            if (dynamic_cast<const ForegroundColorSpan*>(style[j])) {
                const int color = dynamic_cast<const ForegroundColorSpan*>(style[j])->getForegroundColor();
                out<<TextUtils::stringPrintf("<span style=\"color:#%06X;\">", 0xFFFFFF & color);
            }
            if (dynamic_cast<const BackgroundColorSpan*>(style[j])) {
                const int color = dynamic_cast<const BackgroundColorSpan*>(style[j])->getBackgroundColor();
                out<<TextUtils::stringPrintf("<span style=\"background-color:#%06X;\">", 0xFFFFFF & color);
            }
        }

        withinStyle(out, text, i, next);

        for (int j = style.size() - 1; j >= 0; j--) {
            if (dynamic_cast<const BackgroundColorSpan*>(style[j])) {
                out << "</span>";
            }
            if (dynamic_cast<const ForegroundColorSpan*>(style[j])) {
                out << "</span>";
            }
            if (dynamic_cast<const RelativeSizeSpan*>(style[j])) {
                out << "</span>";
            }
            if (dynamic_cast<const AbsoluteSizeSpan*>(style[j])) {
                out << "</span>";
            }
            if (dynamic_cast<const URLSpan*>(style[j])) {
                out << "</a>";
            }
            if (dynamic_cast<const StrikethroughSpan*>(style[j])) {
                out << "</span>";
            }
            if (dynamic_cast<const UnderlineSpan*>(style[j])) {
                out << "</u>";
            }
            if (dynamic_cast<const SubscriptSpan*>(style[j])) {
                out << "</sub>";
            }
            if (dynamic_cast<const SuperscriptSpan*>(style[j])) {
                out << "</sup>";
            }
            if (dynamic_cast<const TypefaceSpan*>(style[j])) {
                std::string s = dynamic_cast<const TypefaceSpan*>(style[j])->getFamily();

                if (s.compare("monospace")==0) {
                    out << "</tt>";
                }
            }
            if (dynamic_cast<const StyleSpan*>(style[j])) {
                int s = (dynamic_cast<const StyleSpan*>(style[j]))->getStyle();

                if ((s & Typeface::BOLD) != 0) {
                    out << "</b>";
                }
                if ((s & Typeface::ITALIC) != 0) {
                    out << "</i>";
                }
            }
        }
    }
}

void Html::withinStyle(std::stringstream& out,const CharSequence& text, int start, int end) {
    for (int i = start; i < end; i++) {
        char c = text.charAt(i);

        if (c == '<') {
            out << "&lt;";
        } else if (c == '>') {
            out << "&gt;";
        } else if (c == '&') {
            out << "&amp;";
        } else if (c >= 0xD800 && c <= 0xDFFF) {
            if (c < 0xDC00 && i + 1 < end) {
                char d = text.charAt(i + 1);
                if (d >= 0xDC00 && d <= 0xDFFF) {
                    i++;
                    int codepoint = 0x010000 | (int) c - 0xD800 << 10 | (int) d - 0xDC00;
                    out << "&#" << codepoint << ";";
                }
            }
        } else if (c > 0x7E || c < ' ') {
            out<< "&#"<< (int) c << ";";
        } else if (c == ' ') {
            while (i + 1 < end && text.charAt(i + 1) == ' ') {
                out << "&nbsp;";
                i++;
            }

            out << ' ';
        } else {
            out << c;
        }
    }
}

}/*endof namespace*/
