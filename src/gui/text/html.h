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
    static void withinHtml(std::stringstream& out, const Spanned& text, int option);
    static void encodeTextAlignmentByDiv(std::stringstream& out,const Spanned& text, int option);
    static void withinDiv(std::stringstream& out,const Spanned& text, int start, int end, int option);
    static std::string getTextDirection(const Spanned& text, int start, int end);
    static std::string getTextStyles(const Spanned& text, int start, int end, bool forceNoVerticalMargin, bool includeTextAlign);

    static void withinBlockquote(std::stringstream& out,const Spanned& text, int start, int end, int option);
    static void withinBlockquoteIndividual(std::stringstream& out,const Spanned& text, int start, int end);
    static void withinBlockquoteConsecutive(std::stringstream& out,const Spanned& text, int start, int end);
    static float getDisplayMetricsDensity();
    //static float getDisplayMetricsDensity$ravenwood();
    static void withinParagraph(std::stringstream& out,const Spanned& text, int start, int end);
    static void withinStyle(std::stringstream& out,const CharSequence& text, int start, int end);
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
    static std::string toHtml(const Spanned& text);

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
    static std::string toHtml(const Spanned& text, int option);

    static std::string escapeHtml(const CharSequence& text);
};/*endof Html*/

}/*endof namespace*/
#endif
