#ifndef __CDROID_HTML_H__
#define __CDROID_HTML_H__
#include <string>
#include <sstream>
#include <functional>
#include <text/spannablestring.h>
namespace cdroid{
class Drawable;
class Editable;
class XMLReader;
class Html {
public:
    /**
     * Retrieves images for HTML &lt;img&gt; tags.
     */
    using ImageGetter = std::function<Drawable*(const std::string&)>;
    using TagHandler = std::function<void(bool,const std::string&,Editable&output,XMLReader&)>;
    /**
     * Option for {@link #toHtml(Spanned, int)}: Wrap consecutive lines of text delimited by '\n'
     * inside &lt;p&gt; elements. {@link BulletSpan}s are ignored.
     */
    static constexpr int TO_HTML_PARAGRAPH_LINES_CONSECUTIVE = 0x00000000;

    /**
     * Option for {@link #toHtml(Spanned, int)}: Wrap each line of text delimited by '\n' inside a
     * &lt;p&gt; or a &lt;li&gt; element. This allows {@link ParagraphStyle}s attached to be
     * encoded as CSS styles within the corresponding &lt;p&gt; or &lt;li&gt; element.
     */
    static constexpr int TO_HTML_PARAGRAPH_LINES_INDIVIDUAL = 0x00000001;

    /**
     * Flag indicating that texts inside &lt;p&gt; elements will be separated from other texts with
     * one newline character by default.
     */
    static constexpr int FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH = 0x00000001;

    /**
     * Flag indicating that texts inside &lt;h1&gt;~&lt;h6&gt; elements will be separated from
     * other texts with one newline character by default.
     */
    static constexpr int FROM_HTML_SEPARATOR_LINE_BREAK_HEADING = 0x00000002;

    /**
     * Flag indicating that texts inside &lt;li&gt; elements will be separated from other texts
     * with one newline character by default.
     */
    static constexpr int FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM = 0x00000004;

    /**
     * Flag indicating that texts inside &lt;ul&gt; elements will be separated from other texts
     * with one newline character by default.
     */
    static constexpr int FROM_HTML_SEPARATOR_LINE_BREAK_LIST = 0x00000008;

    /**
     * Flag indicating that texts inside &lt;div&gt; elements will be separated from other texts
     * with one newline character by default.
     */
    static constexpr int FROM_HTML_SEPARATOR_LINE_BREAK_DIV = 0x00000010;

    /**
     * Flag indicating that texts inside &lt;blockquote&gt; elements will be separated from other
     * texts with one newline character by default.
     */
    static constexpr int FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE = 0x00000020;

    /**
     * Flag indicating that CSS color values should be used instead of those defined in
     * {@link Color}.
     */
    static constexpr int FROM_HTML_OPTION_USE_CSS_COLORS = 0x00000100;

    /**
     * Flags for {@link #fromHtml(String, int, ImageGetter, TagHandler)}: Separate block-level
     * elements with blank lines (two newline characters) in between. This is the legacy behavior
     * prior to N.
     */
    static constexpr int FROM_HTML_MODE_LEGACY = 0x00000000;

    /**
     * Flags for {@link #fromHtml(String, int, ImageGetter, TagHandler)}: Separate block-level
     * elements with line breaks (single newline character) in between. This inverts the
     * {@link Spanned} to HTML string conversion done with the option
     * {@link #TO_HTML_PARAGRAPH_LINES_INDIVIDUAL}.
     */
    static constexpr int FROM_HTML_MODE_COMPACT =
            FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH
            | FROM_HTML_SEPARATOR_LINE_BREAK_HEADING
            | FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM
            | FROM_HTML_SEPARATOR_LINE_BREAK_LIST
            | FROM_HTML_SEPARATOR_LINE_BREAK_DIV
            | FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE;

private:
    /**
     * The bit which indicates if lines delimited by '\n' will be grouped into &lt;p&gt; elements.
     */
    static constexpr int TO_HTML_PARAGRAPH_FLAG = 0x00000001;
private:
    Html()=default;
    static void withinHtml(std::stringstream& out, Spanned* text, int option);
    static void encodeTextAlignmentByDiv(std::stringstream& out, Spanned* text, int option);
    static void withinDiv(std::stringstream& out, Spanned* text, int start, int end, int option);
    static std::string getTextDirection(Spanned* text, int start, int end);
    static std::string getTextStyles(Spanned* text, int start, int end, bool forceNoVerticalMargin, bool includeTextAlign);

    static void withinBlockquote(std::stringstream& out, Spanned* text, int start, int end, int option);
    static void withinBlockquoteIndividual(std::stringstream& out, Spanned* text, int start, int end);
    static void withinBlockquoteConsecutive(std::stringstream& out, Spanned* text, int start, int end);
    static float getDisplayMetricsDensity();
    //static float getDisplayMetricsDensity$ravenwood();
    static void withinParagraph(std::stringstream& out, Spanned* text, int start, int end);
    static void withinStyle(std::stringstream& out, CharSequence* text, int start, int end);
public:
    /**
     * Returns displayable styled text from the provided HTML string with the legacy flags
     * {@link #FROM_HTML_MODE_LEGACY}.
     *
     * @deprecated use {@link #fromHtml(String, int)} instead.
     */
    static Spanned* fromHtml(const std::string& source);

    /**
     * Returns displayable styled text from the provided HTML string. Any &lt;img&gt; tags in the
     * HTML will display as a generic replacement image which your program can then go through and
     * replace with real images.
     *
     * <p>This uses TagSoup to handle real HTML, including all of the brokenness found in the wild.
     */
    static Spanned* fromHtml(const std::string& source, int flags);

    /**
     * Returns displayable styled text from the provided HTML string with the legacy flags
     * {@link #FROM_HTML_MODE_LEGACY}.
     *
     * @deprecated use {@link #fromHtml(String, int, ImageGetter, TagHandler)} instead.
     */
    static Spanned* fromHtml(const std::string& source, ImageGetter imageGetter, TagHandler tagHandler);
    /**
     * Returns displayable styled text from the provided HTML string. Any &lt;img&gt; tags in the
     * HTML will use the specified ImageGetter to request a representation of the image (use null
     * if you don't want this) and the specified TagHandler to handle unknown tags (specify null if
     * you don't want this).
     *
     * <p>This uses TagSoup to handle real HTML, including all of the brokenness found in the wild.
     */
    static Spanned* fromHtml(const std::string& source, int flags, ImageGetter imageGetter,
            TagHandler tagHandler);

    /**
     * @deprecated use {@link #toHtml(Spanned, int)} instead.
     */
    static std::string toHtml(Spanned* text);

    /**
     * Returns an HTML representation of the provided Spanned text. A best effort is
     * made to add HTML tags corresponding to spans. Also note that HTML metacharacters
     * (such as "&lt;" and "&amp;") within the input text are escaped.
     *
     * @param text input text to convert
     * @param option one of {@link #TO_HTML_PARAGRAPH_LINES_CONSECUTIVE} or
     *     {@link #TO_HTML_PARAGRAPH_LINES_INDIVIDUAL}
     * @return string containing input converted to HTML
     */
    static std::string toHtml(Spanned* text, int option);

    static std::string escapeHtml(CharSequence* text);
};/*endof Html*/

#if 0
class HtmlToSpannedConverter implements ContentHandler {

    private static float[] HEADING_SIZES = {
        1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1f,
    };

    private std::string mSource;
    private XMLReader mReader;
    private SpannableStringBuilder mSpannableStringBuilder;
    private Html::ImageGetter mImageGetter;
    private Html::TagHandler mTagHandler;
    private int mFlags;

    private static Pattern sTextAlignPattern;
    private static Pattern sForegroundColorPattern;
    private static Pattern sBackgroundColorPattern;
    private static Pattern sTextDecorationPattern;

    /**
     * Name-value mapping of HTML/CSS colors which have different values in {@link Color}.
     */
    private static Map<String, Integer> sColorMap;

    static {
        sColorMap = new HashMap<>();
        sColorMap.put("darkgray", 0xFFA9A9A9);
        sColorMap.put("gray", 0xFF808080);
        sColorMap.put("lightgray", 0xFFD3D3D3);
        sColorMap.put("darkgrey", 0xFFA9A9A9);
        sColorMap.put("grey", 0xFF808080);
        sColorMap.put("lightgrey", 0xFFD3D3D3);
        sColorMap.put("green", 0xFF008000);
    }

    private static Pattern getTextAlignPattern() {
        if (sTextAlignPattern == null) {
            sTextAlignPattern = Pattern.compile("(?:\\s+|\\A)text-align\\s*:\\s*(\\S*)\\b");
        }
        return sTextAlignPattern;
    }

    private static Pattern getForegroundColorPattern() {
        if (sForegroundColorPattern == null) {
            sForegroundColorPattern = Pattern.compile(
                    "(?:\\s+|\\A)color\\s*:\\s*(\\S*)\\b");
        }
        return sForegroundColorPattern;
    }

    private static Pattern getBackgroundColorPattern() {
        if (sBackgroundColorPattern == null) {
            sBackgroundColorPattern = Pattern.compile(
                    "(?:\\s+|\\A)background(?:-color)?\\s*:\\s*(\\S*)\\b");
        }
        return sBackgroundColorPattern;
    }

    private static Pattern getTextDecorationPattern() {
        if (sTextDecorationPattern == null) {
            sTextDecorationPattern = Pattern.compile(
                    "(?:\\s+|\\A)text-decoration\\s*:\\s*(\\S*)\\b");
        }
        return sTextDecorationPattern;
    }

    public HtmlToSpannedConverter(const std::string& source, Html::ImageGetter imageGetter,
            Html::TagHandler tagHandler, Parser parser, int flags) {
        mSource = source;
        mSpannableStringBuilder = new SpannableStringBuilder();
        mImageGetter = imageGetter;
        mTagHandler = tagHandler;
        mReader = parser;
        mFlags = flags;
    }

    public Spanned convert();

    private void handleStartTag(const std::string& tag, Attributes attributes);

    private static int getFontWeightAdjustment() {
        return ActivityThread.currentApplication().getResources()
                .getConfiguration().fontWeightAdjustment;
    }

    private static int getFontWeightAdjustment$ravenwood() {
        return Resources.getSystem().getConfiguration().fontWeightAdjustment;
    }

    private void handleEndTag(const std::string& tag);

    private int getMarginParagraph() {
        return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH);
    }

    private int getMarginHeading() {
        return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_HEADING);
    }

    private int getMarginListItem() {
        return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM);
    }

    private int getMarginList() {
        return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_LIST);
    }

    private int getMarginDiv() {
        return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_DIV);
    }

    private int getMarginBlockquote() {
        return getMargin(Html::FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE);
    }

    private int getMargin(int flag) {
        if ((flag & mFlags) != 0) {
            return 1;
        }
        return 2;
    }

    private static void appendNewlines(Editable text, int minNewline);

    private static void startBlockElement(Editable text, Attributes attributes, int margin);

    private static void endBlockElement(Editable text);

    private static void handleBr(Editable text) {
        text.append('\n');
    }

    private void startLi(Editable text, Attributes attributes) {
        startBlockElement(text, attributes, getMarginListItem());
        start(text, new Bullet());
        startCssStyle(text, attributes);
    }

    private static void endLi(Editable text) {
        endCssStyle(text);
        endBlockElement(text);
        end(text, Bullet.class, new BulletSpan());
    }

    private void startBlockquote(Editable text, Attributes attributes) {
        startBlockElement(text, attributes, getMarginBlockquote());
        start(text, new Blockquote());
    }

    private static void endBlockquote(Editable text) {
        endBlockElement(text);
        end(text, Blockquote.class, new QuoteSpan());
    }

    private void startHeading(Editable text, Attributes attributes, int level) {
        startBlockElement(text, attributes, getMarginHeading());
        start(text, new Heading(level));
    }

    private static void endHeading(Editable text);

    private static <T> T getLast(Spanned text, Class<T> kind) {
        /*
         * This knows that the last returned object from getSpans()
         * will be the most recently added.
         */
        T[] objs = text.getSpans(0, text.length(), kind);

        if (objs.length == 0) {
            return null;
        } else {
            return objs[objs.length - 1];
        }
    }

    private static void setSpanFromMark(Spannable text, Object mark, Object... spans) {
        int where = text.getSpanStart(mark);
        text.removeSpan(mark);
        int len = text.length();
        if (where != len) {
            for (Object span : spans) {
                text.setSpan(span, where, len, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
        }
    }

    private static void start(Editable text, Object mark) {
        int len = text.length();
        text.setSpan(mark, len, len, Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
    }

    private static void end(Editable text, Class kind, Object repl) {
        int len = text.length();
        Object obj = getLast(text, kind);
        if (obj != null) {
            setSpanFromMark(text, obj, repl);
        }
    }

    private void startCssStyle(Editable text, Attributes attributes);

    private static void endCssStyle(Editable text);

    private static void startImg(Editable text, Attributes attributes, Html::ImageGetter img);

    private void startFont(Editable text, Attributes attributes);
    private static void endFont(Editable text);

    private static void startA(Editable text, Attributes attributes) {
        std::string href = attributes.getValue("", "href");
        start(text, new Href(href));
    }

    private static void endA(Editable text) {
        Href h = getLast(text, Href.class);
        if (h != null) {
            if (h.mHref != null) {
                setSpanFromMark(text, h, new URLSpan((h.mHref)));
            }
        }
    }

    private int getHtmlColor(const std::string& color);

    public void setDocumentLocator(Locator locator) {
    }

    public void startDocument() throws SAXException {
    }

    public void endDocument() throws SAXException {
    }

    public void startPrefixMapping(const std::string& prefix, const std::string& uri) throws SAXException {
    }

    public void endPrefixMapping(const std::string& prefix) throws SAXException {
    }

    public void startElement(const std::string& uri, const std::string& localName, const std::string& qName, Attributes attributes)
            throws SAXException {
        handleStartTag(localName, attributes);
    }

    public void endElement(const std::string& uri, const std::string& localName, const std::string& qName) throws SAXException {
        handleEndTag(localName);
    }

    public void characters(char* ch, int start, int length);

    public void ignorableWhitespace(char ch[], int start, int length) throws SAXException {
    }

    public void processingInstruction(const std::string& target, const std::string& data) throws SAXException {
    }

    public void skippedEntity(const std::string& name) throws SAXException {
    }

    private static class Bold { }
    private static class Italic { }
    private static class Underline { }
    private static class Strikethrough { }
    private static class Big { }
    private static class Small { }
    private static class Monospace { }
    private static class Blockquote { }
    private static class Super { }
    private static class Sub { }
    private static class Bullet { }

    private static class Font {
        public String mFace;

        public Font(String face) {
            mFace = face;
        }
    }

    private static class Href {
        public String mHref;

        public Href(String href) {
            mHref = href;
        }
    }

    private static class Foreground {
        private int mForegroundColor;

        public Foreground(int foregroundColor) {
            mForegroundColor = foregroundColor;
        }
    }

    private static class Background {
        private int mBackgroundColor;

        public Background(int backgroundColor) {
            mBackgroundColor = backgroundColor;
        }
    }

    private static class Heading {
        private int mLevel;

        public Heading(int level) {
            mLevel = level;
        }
    }

    private static class Newline {
        private int mNumNewlines;

        public Newline(int numNewlines) {
            mNumNewlines = numNewlines;
        }
    }

    private static class Alignment {
        private Layout.Alignment mAlignment;

        public Alignment(Layout.Alignment alignment) {
            mAlignment = alignment;
        }
    }
};
#endif/**/
}/*endof namespace*/
#endif
