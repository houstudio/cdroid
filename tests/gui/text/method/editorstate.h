/*********************************************************************************
 * Ported from AOSP coretests EditorState.java (Apache 2.0).
 *
 * Represents an editor state for the BaseKeyListener delete-key tests.
 *
 * The editor state can be specified by following string format.
 * - Components are separated by space(U+0020).
 * - Single-quoted string for printable ASCII characters, e.g. 'a', '123'.
 * - U+XXXX form can be used for a Unicode code point.
 * - Components inside '[' and ']' are in selection.
 * - Components inside '(' and ')' are in ReplacementSpan.
 * - '|' is for specifying cursor position.
 *
 * Selection and cursor can not be specified at the same time.
 *
 * Example:
 *   - "'Hello,' | U+0020 'world!'" means "Hello, world!" is displayed and the
 *     cursor position is 6.
 *   - "'abc' [ 'def' ] 'ghi'" means "abcdefghi" is displayed and "def" is
 *     selected.
 *   - "U+1F441 | ( U+1F441 U+1F441 )" means three U+1F441 characters are
 *     displayed and ReplacementSpan is set from offset 2 to 6.
 *
 * C++ note: mText is deliberately never freed by this helper. The buffer may
 * be adopted by the TextView under test (setText transfers ownership) and
 * state.mText then aliases the TextView's internal buffer, which must outlive
 * every assertEquals call in the test body — the AOSP fixture keeps the
 * TextView alive for the whole test method, and so do we.
 *********************************************************************************/
#ifndef TEXT_METHOD_TESTS_EDITORSTATE_H
#define TEXT_METHOD_TESTS_EDITORSTATE_H

#include <text/spannablestring.h>
#include <text/spannablestringbuilder.h>
#include <text/style/replacementspan.h>
#include <text/selection.h>
#include <cdroid.h>
#include <gtest/gtest.h>
#include <string>

namespace cdroid {

// AOSP mocks a ReplacementSpan whose getSize() returns 0 and draw() does
// nothing; ReplacementSpan::getSize() already defaults to 0, so only the pure
// virtual draw() needs a no-op body.
class MockReplacementSpan : public ReplacementSpan {
public:
    void draw(Canvas&, const CharSequence*, int, int, float, int, int, int, const Paint&) const override {}
};

class EditorState {
private:
    static constexpr const char* REPLACEMENT_SPAN_START = "(";
    static constexpr const char* REPLACEMENT_SPAN_END = ")";
    static constexpr const char* SELECTION_START = "[";
    static constexpr const char* SELECTION_END = "]";
    static constexpr const char* CURSOR = "|";

    // Returns true if the code point is ASCII and graph.
    static bool isGraphicAscii(int codePoint) {
        return 0x20 < codePoint && codePoint < 0x7F;
    }

    static void appendCodePoint(std::u16string& out, int codePoint) {
        if (codePoint <= 0xFFFF) {
            out.push_back((char16_t)codePoint);
        } else {
            codePoint -= 0x10000;
            out.push_back((char16_t)(0xD800 + (codePoint >> 10)));
            out.push_back((char16_t)(0xDC00 + (codePoint & 0x3FF)));
        }
    }

public:
    Editable* mText = nullptr;
    int mSelectionStart = -1;
    int mSelectionEnd = -1;

    // Setup editor state with string. Please see class description for string format.
    void setByString(const std::string& string) {
        std::u16string sb;
        int replacementSpanStart = -1;
        int replacementSpanEnd = -1;
        mSelectionStart = -1;
        mSelectionEnd = -1;

        // string.split(" +") on a copy: strtok-style walk over runs of spaces.
        std::string s = string;
        size_t pos = 0;
        while (pos < s.size() || (pos == 0 && !s.empty())) {
            size_t start = s.find_first_not_of(' ', pos);
            if (start == std::string::npos) break;
            size_t end = s.find_first_of(' ', start);
            if (end == std::string::npos) end = s.size();
            const std::string token = s.substr(start, end - start);
            pos = end;

            if (token.size() >= 2 && token.front() == '\'' && token.back() == '\'') {
                for (size_t i = 1; i < token.size() - 1; ++i) {
                    const char ch = token[i];
                    if (!isGraphicAscii(ch)) {
                        // AOSP throws IllegalArgumentException here.
                        ADD_FAILURE() << "Only printable characters can be in single quote: " << token;
                        return;
                    }
                }
                const std::string body = token.substr(1, token.size() - 2);
                sb.append(body.begin(), body.end());
            } else if (token.size() > 2 && token[0] == 'U' && token[1] == '+') {
                const int codePoint = std::stoi(token.substr(2), nullptr, 16);
                if (codePoint < 0 || 0x10FFFF < codePoint) {
                    ADD_FAILURE() << "Invalid code point is specified:" << token;
                    return;
                }
                appendCodePoint(sb, codePoint);
            } else if (token == CURSOR) {
                if (mSelectionStart != -1 || mSelectionEnd != -1) {
                    ADD_FAILURE() << "Two or more cursor/selection positions are specified.";
                    return;
                }
                mSelectionStart = mSelectionEnd = (int)sb.size();
            } else if (token == SELECTION_START) {
                if (mSelectionStart != -1) {
                    ADD_FAILURE() << "Two or more cursor/selection positions are specified.";
                    return;
                }
                mSelectionStart = (int)sb.size();
            } else if (token == SELECTION_END) {
                if (mSelectionEnd != -1) {
                    ADD_FAILURE() << "Two or more cursor/selection positions are specified.";
                    return;
                }
                mSelectionEnd = (int)sb.size();
            } else if (token == REPLACEMENT_SPAN_START) {
                if (replacementSpanStart != -1) {
                    ADD_FAILURE() << "Only one replacement span is supported";
                    return;
                }
                replacementSpanStart = (int)sb.size();
            } else if (token == REPLACEMENT_SPAN_END) {
                if (replacementSpanEnd != -1) {
                    ADD_FAILURE() << "Only one replacement span is supported";
                    return;
                }
                replacementSpanEnd = (int)sb.size();
            } else {
                ADD_FAILURE() << "Unknown or invalid token: " << token;
                return;
            }
        }

        if (mSelectionStart == -1 || mSelectionEnd == -1) {
            if (mSelectionEnd != -1) {
                ADD_FAILURE() << "Selection start position doesn't exist.";
            } else if (mSelectionStart != -1) {
                ADD_FAILURE() << "Selection end position doesn't exist.";
            } else {
                ADD_FAILURE() << "At least cursor position or selection range must be specified.";
            }
            return;
        } else if (mSelectionStart > mSelectionEnd) {
            ADD_FAILURE() << "Selection start position appears after end position.";
            return;
        }

        SpannableString* spannable = new SpannableString(sb);

        if (replacementSpanStart != -1 || replacementSpanEnd != -1) {
            if (replacementSpanStart == -1) {
                ADD_FAILURE() << "ReplacementSpan start position doesn't exist.";
                return;
            }
            if (replacementSpanEnd == -1) {
                ADD_FAILURE() << "ReplacementSpan end position doesn't exist.";
                return;
            }
            if (replacementSpanStart > replacementSpanEnd) {
                ADD_FAILURE() << "ReplacementSpan start position appears after end position.";
                return;
            }
            spannable->setSpan(new MockReplacementSpan(), replacementSpanStart, replacementSpanEnd,
                    Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        // Editable.Factory.getInstance().newEditable(spannable): AOSP copies the
        // source (plus its spans) into a new SpannableStringBuilder.
        mText = new SpannableStringBuilder(spannable);
    }

    void assertEquals(const std::string& string) {
        EditorState expected;
        expected.setByString(string);

        ASSERT_TRUE(expected.mText != nullptr);
        ASSERT_TRUE(mText != nullptr);
        EXPECT_EQ(expected.mText->toUTF16(), mText->toUTF16());
        EXPECT_EQ(expected.mSelectionStart, mSelectionStart);
        EXPECT_EQ(expected.mSelectionEnd, mSelectionEnd);
    }
};

} // namespace cdroid

#endif // TEXT_METHOD_TESTS_EDITORSTATE_H
