#include <text/linkify.h>
#include <text/spannablestring.h>      // Spannable/Spanned/SPAN_EXCLUSIVE_EXCLUSIVE
#include <text/spannablestringbuilder.h>
#include <text/style/clickablespan.h>  // URLSpan
#include <text/method/linkmovementmethod.h>
#include <text/method/movementmethod.h>
#include <widget/textview.h>
#include <parcelablespan.h>            // ParcelableSpan, SpanFilter
#include <algorithm>
#include <cctype>

namespace cdroid {

namespace {
constexpr int PHONE_NUMBER_MINIMUM_DIGITS = 5;

// AOSP Patterns.AUTOLINK_WEB_URL — simplified to plain ECMAScript (std::regex
// can't compile AOSP's lookbehind/inline-flag/class-intersection form). Matches
// either a scheme-prefixed URL or a bare "host.tld[/path]" domain.
const std::regex& webUrlRe() {
    static const std::regex re(
        R"((?:https?|ftp)://[^\s/$.?#].[^\s]*)"
        R"(|)"
        R"([a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?)*\.[a-zA-Z]{2,}(?:/[^\s]*)?)",
        std::regex::icase);
    return re;
}
// AOSP Patterns.AUTOLINK_EMAIL_ADDRESS — simplified.
const std::regex& emailRe() {
    static const std::regex re(
        R"([a-zA-Z0-9.+\-]+@[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?)*\.[a-zA-Z]{2,})",
        std::regex::icase);
    return re;
}
// AOSP Patterns.PHONE — verbatim, already ECMAScript-compatible.
const std::regex& phoneRe() {
    static const std::regex re(R"((\+[0-9]+[\- \.]*)?(\([0-9]+\)[\- \.]*)?([0-9][0-9\- \.]+[0-9]))");
    return re;
}

// AOSP Linkify.sUrlMatchFilter: reject when the char just before the match is
// '@' (so the domain in foo@bar.com isn't separately web-linked).
bool sUrlMatchFilter(const std::string& s, int start, int /*end*/) {
    return !(start > 0 && s[start - 1] == '@');
}
// AOSP Linkify.sPhoneNumberMatchFilter: accept only if the match contains at
// least PHONE_NUMBER_MINIMUM_DIGITS digits.
bool sPhoneNumberMatchFilter(const std::string& s, int start, int end) {
    int digits = 0;
    for (int i = start; i < end; ++i)
        if (std::isdigit((unsigned char)s[i])) ++digits;
    return digits >= PHONE_NUMBER_MINIMUM_DIGITS;
}
} // namespace

std::string Linkify::digitsAndPlusOnly(const std::smatch& m) {
    // AOSP Patterns.digitsAndPlusOnly: keep only '+' and digits from the match.
    std::string out;
    const std::string g = m.str();
    for (char c : g)
        if (c == '+' || std::isdigit((unsigned char)c)) out.push_back(c);
    return out;
}

bool Linkify::containsUnsupportedCharacters(const std::string& s) {
    // CVE-116321860: reject U+202C/202D/202E (bidi override/embedding) before
    // linking. Compared as UTF-8 byte sequences since `s` is UTF-8.
    return s.find("\xe2\x80\xac") != std::string::npos  // U+202C POP DIRECTIONAL FORMATTING
        || s.find("\xe2\x80\xad") != std::string::npos  // U+202D LEFT-TO-RIGHT OVERRIDE
        || s.find("\xe2\x80\xae") != std::string::npos; // U+202E RIGHT-TO-LEFT OVERRIDE
}

std::string Linkify::makeUrl(const std::string& url, const std::vector<std::string>& prefixes,
        const std::smatch& m, TransformFilter filter) {
    std::string u = filter ? filter(m, url) : url;
    bool hasPrefix = false;
    auto ieq = [](const std::string& a, int off, const std::string& b) {
        if ((int)b.size() > (int)a.size() - off) return false;
        for (size_t i = 0; i < b.size(); ++i) {
            char ac = a[off + i], bc = b[i];
            if (std::tolower((unsigned char)ac) != std::tolower((unsigned char)bc)) return false;
        }
        return true;
    };
    for (const std::string& p : prefixes) {
        if (ieq(u, 0, p)) {
            hasPrefix = true;
            // Fix capitalization: if it matches case-insensitively but not exactly,
            // replace the prefix with the canonical one.
            if (u.compare(0, p.size(), p) != 0) u = p + u.substr(p.size());
            break;
        }
    }
    if (!hasPrefix && !prefixes.empty()) u = prefixes[0] + u;
    return u;
}

void Linkify::applyLink(const std::string& url, int start, int end, Spannable* text) {
    // AOSP applyLink: one new URLSpan per link (URLSpan is an owned span: the
    // Spannable owns/deletes it since it is not a NoCopySpan).
    text->setSpan(new URLSpan(url), start, end, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
}

void Linkify::gatherLinks(std::vector<LinkSpec>& links, Spannable* s,
        const std::regex& pattern, const std::vector<std::string>& schemes,
        MatchFilter matchFilter, TransformFilter transformFilter) {
    const std::string text = s->toString();
    auto begin = std::sregex_iterator(text.begin(), text.end(), pattern);
    const auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const int start = (int)it->position();
        const int eend = start + (int)it->length();
        if (!matchFilter || matchFilter(text, start, eend)) {
            LinkSpec spec;
            spec.url = makeUrl(it->str(), schemes, *it, transformFilter);
            spec.start = start;
            spec.end = eend;
            links.push_back(spec);
        }
    }
}

void Linkify::gatherTelLinks(std::vector<LinkSpec>& links, Spannable* s) {
    // Degraded: AOSP uses libphonenumber (PhoneNumberUtil.findNumbers with
    // Leniency.POSSIBLE + region code). CDROID has no libphonenumber, so fall
    // back to the pre-libphonenumber behaviour: Patterns.PHONE + the
    // digit-count match filter + digitsAndPlusOnly transform. Region-agnostic.
    const std::string text = s->toString();
    auto begin = std::sregex_iterator(text.begin(), text.end(), phoneRe());
    const auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const int start = (int)it->position();
        const int eend = start + (int)it->length();
        if (!sPhoneNumberMatchFilter(text, start, eend)) continue;
        LinkSpec spec;
        spec.url = std::string("tel:") + digitsAndPlusOnly(*it);
        spec.start = start;
        spec.end = eend;
        links.push_back(spec);
    }
}

void Linkify::pruneOverlaps(std::vector<LinkSpec>& links) {
    // AOSP pruneOverlaps: sort by start ascending (longer span first on tie);
    // drop the inner/shorter of two overlapping spans.
    std::sort(links.begin(), links.end(), [](const LinkSpec& a, const LinkSpec& b) {
        if (a.start != b.start) return a.start < b.start;
        return a.end > b.end; // longer first when starts equal
    });
    size_t i = 0;
    while (i + 1 < links.size()) {
        LinkSpec& a = links[i];
        LinkSpec& b = links[i + 1];
        int remove = -1;
        if (a.start <= b.start && a.end > b.start) { // overlap (strict >, touching is ok)
            if (b.end <= a.end) remove = (int)(i + 1);              // b inside a
            else if (a.end - a.start > b.end - b.start) remove = (int)(i + 1);
            else if (a.end - a.start < b.end - b.start) remove = (int)i;
        }
        if (remove != -1) {
            links.erase(links.begin() + remove);
            continue; // re-evaluate the shifted pair without advancing
        }
        ++i;
    }
}

bool Linkify::addLinks(Spannable* text, int mask) {
    if (text == nullptr || mask == 0) return false;
    const std::string s = text->toString();
    if (containsUnsupportedCharacters(s)) return false;

    // Wipe existing URLSpans so re-linking doesn't stack duplicates.
    auto urlFilter = SpanFilter([](const ParcelableSpan* p) {
        return dynamic_cast<const URLSpan*>(p) != nullptr;
    });
    auto old = text->getSpans(0, text->length(), urlFilter);
    for (int i = (int)old.size() - 1; i >= 0; --i) text->removeSpan(old[i]);

    std::vector<LinkSpec> links;
    if (mask & WEB_URLS) {
        gatherLinks(links, text, webUrlRe(),
                {"http://", "https://", "rtsp://", "ftp://"}, sUrlMatchFilter, nullptr);
    }
    if (mask & EMAIL_ADDRESSES) {
        gatherLinks(links, text, emailRe(), {"mailto:"}, nullptr, nullptr);
    }
    if (mask & PHONE_NUMBERS) {
        gatherTelLinks(links, text);
    }
    if (mask & MAP_ADDRESSES) {
        // DEFERRED: needs WebView.findAddress (deprecated upstream). No-op.
    }
    pruneOverlaps(links);
    if (links.empty()) return false;
    for (const LinkSpec& l : links) applyLink(l.url, l.start, l.end, text);
    return true;
}

bool Linkify::addLinks(TextView* text, int mask) {
    if (text == nullptr || mask == 0) return false;
    // AOSP addLinks(TextView,...): operate on a Spannable; wrap non-Spannable
    // text in a SpannableString first. TextView's buffer is Spannable in the
    // common (editable / autoLink) cases; the wrap branch is left as a TODO.
    Spannable* s = dynamic_cast<Spannable*>(&text->getText());
    if (s == nullptr) {
        // TODO: SpannableString.valueOf(text) for non-Spannable buffers.
        return false;
    }
    if (addLinks(s, mask)) {
        addLinkMovementMethod(text);
        return true;
    }
    return false;
}

void Linkify::addLinkMovementMethod(TextView* t) {
    MovementMethod* m = t->getMovementMethod();
    if ((m == nullptr) || dynamic_cast<LinkMovementMethod*>(m) == nullptr) {
        if (t->getLinksClickable()) t->setMovementMethod(LinkMovementMethod::getInstance());
    }
}

} // namespace cdroid
