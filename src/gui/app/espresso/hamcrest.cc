#include <app/espresso/hamcrest.h>

#include <cstdio>

namespace cdroid {
namespace espresso {

// StringDescription renders values Java-style: strings double-quoted,
// booleans/numbers bare, floats with the 'f' suffix (Java appendValue style).

Description& StringDescription::appendText(const std::string& text) {
    mOut.append(text);
    return *this;
}

Description& StringDescription::appendDescriptionOf(const SelfDescribing& value) {
    value.describeTo(*this);
    return *this;
}

Description& StringDescription::appendValue(bool value) {
    mOut.append(value ? "true" : "false");
    return *this;
}

static void appendNumber(std::string& out, const char* fmt, double v) {
    char buf[64];
    snprintf(buf, sizeof(buf), fmt, v);
    out.append(buf);
}

Description& StringDescription::appendValue(int value) {
    appendNumber(mOut, "%d", value);
    return *this;
}

Description& StringDescription::appendValue(int64_t value) {
    appendNumber(mOut, "%lld", static_cast<long long>(value));
    return *this;
}

Description& StringDescription::appendValue(float value) {
    appendNumber(mOut, "%gf", value);
    return *this;
}

Description& StringDescription::appendValue(double value) {
    appendNumber(mOut, "%g", value);
    return *this;
}

Description& StringDescription::appendValue(const char* value) {
    return appendValue(std::string(value ? value : "(null)"));
}

Description& StringDescription::appendValue(const std::string& value) {
    mOut.push_back('"');
    mOut.append(value);
    mOut.push_back('"');
    return *this;
}

Description& StringDescription::appendValueList(const std::string& start,
        const std::string& separator, const std::string& end,
        const std::vector<std::string>& values) {
    appendText(start);
    bool first = true;
    for (const auto& v : values) {
        if (!first) appendText(separator);
        first = false;
        appendValue(v);
    }
    appendText(end);
    return *this;
}

std::string StringDescription::toString(const SelfDescribing& selfDescribing) {
    StringDescription description;
    selfDescribing.describeTo(description);
    return description.str();
}

std::string StringDescription::toString(const std::string& prefix,
        const SelfDescribing& selfDescribing) {
    StringDescription description;
    description.appendText(prefix);
    selfDescribing.describeTo(description);
    return description.str();
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
