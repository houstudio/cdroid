#ifndef CDROID_TEXT_LINKIFY_H
#define CDROID_TEXT_LINKIFY_H
#include <string>
#include <vector>
#include <functional>
#include <regex>

namespace cdroid {

class Spannable;
class TextView;
class MovementMethod;

/* android.text.util.Linkify — faithful C++ port. Scans text for web URLs, email
 * addresses and phone numbers and attaches URLSpans over them, so a TextView
 * with autoLink (or an explicit addLinks call) renders them as clickable links
 * when paired with LinkMovementMethod.
 *
 * Regex engine note: AOSP's AUTOLINK_WEB_URL / AUTOLINK_EMAIL_ADDRESS use
 * lookbehind / inline case flags / character-class intersection that libstdc++
 * std::regex (ECMAScript) cannot compile, so those two are replaced here with
 * simplified ECMAScript patterns (good enough for everyday autolinking). The
 * phone pattern is AOSP's verbatim (it is plain ECMAScript). PCRE was ruled out
 * to avoid a new dependency on an embedded target.
 *
 * gatherTelLinks is a degraded port: AOSP uses libphonenumber
 * (PhoneNumberUtil.findNumbers) which CDROID does not have, so we fall back to
 * the pre-libphonenumber behaviour (Patterns.PHONE + the digit-count match
 * filter + digitsAndPlusOnly transform). MAP_ADDRESSES is a no-op (depends on
 * WebView.findAddress, deprecated upstream). */
class Linkify {
public:
    static constexpr int WEB_URLS        = 0x01;
    static constexpr int EMAIL_ADDRESSES = 0x02;
    static constexpr int PHONE_NUMBERS   = 0x04;
    static constexpr int MAP_ADDRESSES   = 0x08; // DEFERRED: needs WebView.findAddress
    static constexpr int ALL = WEB_URLS | EMAIL_ADDRESSES | PHONE_NUMBERS | MAP_ADDRESSES;

    using MatchFilter     = std::function<bool(const std::string& s, int start, int end)>;
    using TransformFilter = std::function<std::string(const std::smatch& m, const std::string& url)>;

    /* AOSP public entry: scan a Spannable and attach URLSpans for the masked
     * link kinds. Returns true if any link was added. */
    static bool addLinks(Spannable* text, int mask);
    /* AOSP public entry: same on a TextView; wraps non-Spannable text in a
     * SpannableString and installs LinkMovementMethod when links were added and
     * the view's links are clickable. */
    static bool addLinks(TextView* text, int mask);

private:
    struct LinkSpec { std::string url; int start = 0; int end = 0; };

    static constexpr int PHONE_NUMBER_MINIMUM_DIGITS = 5;

    // CVE-116321860 guard: reject bidi-override characters before linking.
    static bool containsUnsupportedCharacters(const std::string& s);

    static void gatherLinks(std::vector<LinkSpec>& links, Spannable* s,
            const std::regex& pattern, const std::vector<std::string>& schemes,
            MatchFilter matchFilter, TransformFilter transformFilter);
    // Degraded (no libphonenumber): Patterns.PHONE + digit-count filter.
    static void gatherTelLinks(std::vector<LinkSpec>& links, Spannable* s);
    static std::string makeUrl(const std::string& url, const std::vector<std::string>& prefixes,
            const std::smatch& m, TransformFilter filter);
    static void applyLink(const std::string& url, int start, int end, Spannable* text);
    static void pruneOverlaps(std::vector<LinkSpec>& links);
    static void addLinkMovementMethod(TextView* t);
    // AOSP Patterns.digitsAndPlusOnly: keep only [0-9] and '+'.
    static std::string digitsAndPlusOnly(const std::smatch& m);
};

} // namespace cdroid

#endif // CDROID_TEXT_LINKIFY_H
