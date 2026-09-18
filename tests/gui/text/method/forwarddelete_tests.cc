/*********************************************************************************
 * Ported from AOSP coretests android.text.method.ForwardDeleteTest (Apache 2.0).
 * Test forward delete key handling of android.text.method.BaseKeyListener.
 *
 * Only contains edge cases. For normal cases, see the CTS ForwardDeleteTest.
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization): same
 * Phase-1 grapheme stub as BackspaceTest — getOffsetForForwardDeleteKey
 * deletes one BMP UTF-16 unit instead of a grapheme/emoji cluster.
 *********************************************************************************/
#include "editorstate.h"
#include <widget/edittext.h>
#include <text/method/basekeylistener.h>
#include <text/inputtype.h>
#include <view/keyevent.h>
#include <gtest/gtest.h>

using namespace cdroid;

namespace {

// AOSP: new BaseKeyListener() { getInputType() { ... } }
class TestBaseKeyListener : public BaseKeyListener {
public:
    int getInputType() const override {
        return InputType::TYPE_CLASS_TEXT | InputType::TYPE_TEXT_VARIATION_NORMAL;
    }
};

class ForwardDeleteTest : public testing::Test {
protected:
    // See BackspaceTest: drive BaseKeyListener::forwardDelete directly.
    void forwardDelete(EditorState& state, int modifiers) {
        Selection::setSelection(dynamic_cast<Spannable*>(state.mText),
                state.mSelectionStart, state.mSelectionEnd);

        const nsecs_t now = SystemClock::uptimeMillis();
        KeyEvent* keyEvent = KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN,
                KeyEvent::KEYCODE_FORWARD_DEL, 0, modifiers, 0, 0, 0, 0);
        TestBaseKeyListener keyListener;
        EditText host(&App::getInstance());
        keyListener.forwardDelete(host, *state.mText, KeyEvent::KEYCODE_FORWARD_DEL, *keyEvent);
        delete keyEvent;

        state.mSelectionStart = Selection::getSelectionStart(state.mText);
        state.mSelectionEnd = Selection::getSelectionEnd(state.mText);
    }
};

TEST_F(ForwardDeleteTest, testCombiningEnclosingKeycaps) {
    EditorState state;

    // multiple COMBINING ENCLOSING KEYCAP
    state.setByString("| '1' U+20E3 U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated COMBINING ENCLOSING KEYCAP
    state.setByString("| U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated multiple COMBINING ENCLOSING KEYCAP
    state.setByString("| U+20E3 U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");
}

TEST_F(ForwardDeleteTest, testVariationSelector) {
    EditorState state;

    // Isolated variation selectors
    state.setByString("| U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+E0100");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated multiple variation selectors
    state.setByString("| U+FE0F U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+FE0F U+E0100");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+E0100 U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+E0100 U+E0100");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Multiple variation selectors
    state.setByString("| '#' U+FE0F U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| '#' U+FE0F U+E0100");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+845B U+E0100 U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+845B U+E0100 U+E0100");
    forwardDelete(state, 0);
    state.assertEquals("|");
}

TEST_F(ForwardDeleteTest, testEmojiZeroWidthJoinerSequence) {
    EditorState state;

    // U+200D is ZERO WIDTH JOINER.
    state.setByString("| U+1F441 U+200D U+1F5E8");
    forwardDelete(state, 0);
    state.assertEquals("|");

    state.setByString("| U+1F468 U+200D U+2764 U+FE0F U+200D U+1F48B U+200D U+1F468");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // End with ZERO WIDTH JOINER
    state.setByString("| U+1F441 U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER
    state.setByString("| U+200D U+1F5E8");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F5E8");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Multiple ZERO WIDTH JOINER
    state.setByString("| U+1F441 U+200D U+200D U+1F5E8");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F5E8");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated ZERO WIDTH JOINER
    state.setByString("| U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated multiple ZERO WIDTH JOINER
    state.setByString("| U+200D U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");
}

TEST_F(ForwardDeleteTest, testFlags) {
    EditorState state;

    // Isolated regional indicator symbol
    state.setByString("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Odd numbered regional indicator symbols
    state.setByString("| U+1F1FA U+1F1F8 U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Incomplete sequence. (no tag_term:U+E007E)
    state.setByString("| 'a' U+1F3F4 U+E0067 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F3F4 U+E0067 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| 'b'");

    // No tag_base
    state.setByString("| 'a' U+E0067 U+E007F 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| 'b'");

    // Isolated tag chars
    state.setByString("| 'a' U+E0067 U+E0067 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| 'b'");

    // Isolated tag base.
    state.setByString("| 'a' U+1F3F4 U+1F3F4 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F3F4 U+1F3F4 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F3F4 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| 'b'");

    // Isolated tab term.
    state.setByString("| 'a' U+E007F U+E007F 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| 'b'");

    // Immediate tag_term after tag_base
    state.setByString("| 'a' U+1F3F4 U+E007F U+1F3F4 U+E007F 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F3F4 U+E007F U+1F3F4 U+E007F 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F3F4 U+E007F 'b'");
    forwardDelete(state, 0);
    state.assertEquals("| 'b'");
}

TEST_F(ForwardDeleteTest, testEmojiModifier) {
    EditorState state;

    // U+1F3FB is EMOJI MODIFIER FITZPATRICK TYPE-1-2.
    state.setByString("| U+1F466 U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated emoji modifier
    state.setByString("| U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Isolated multiple emoji modifier
    state.setByString("| U+1F3FB U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Multiple emoji modifiers
    state.setByString("| U+1F466 U+1F3FB U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");
}

TEST_F(ForwardDeleteTest, testMixedEdgeCases) {
    EditorState state;

    // COMBINING ENCLOSING KEYCAP + variation selector
    state.setByString("| '1' U+20E3 U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Variation selector + COMBINING ENCLOSING KEYCAP
    state.setByString("| U+2665 U+FE0F U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + ending with ZERO WIDTH JOINER
    state.setByString("| '1' U+20E3 U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + ZERO WIDTH JOINER
    state.setByString("| '1' U+20E3 U+200D U+1F5E8");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F5E8 ");

    // Start with ZERO WIDTH JOINER + COMBINING ENCLOSING KEYCAP
    state.setByString("| U+200D U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + COMBINING ENCLOSING KEYCAP
    state.setByString("| U+1F441 U+200D U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + regional indicator symbol
    state.setByString("| '1' U+20E3 U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + emoji modifier
    state.setByString("| '1' U+20E3 U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Emoji modifier + COMBINING ENCLOSING KEYCAP
    state.setByString("| U+1F466 U+1F3FB U+20E3");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Variation selector + end with ZERO WIDTH JOINER
    state.setByString("| U+2665 U+FE0F U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Variation selector + ZERO WIDTH JOINER
    state.setByString("| U+1F469 U+200D U+2764 U+FE0F U+200D U+1F469");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER + variation selector
    state.setByString("| U+200D U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + variation selector
    state.setByString("| U+1F469 U+200D U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Variation selector + regional indicator symbol
    state.setByString("| U+2665 U+FE0F U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + variation selector
    state.setByString("| U+1F1FA U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Variation selector + emoji modifier
    state.setByString("| U+2665 U+FE0F U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Emoji modifier + variation selector
    state.setByString("| U+1F466 U+1F3FB U+FE0F");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER + regional indicator symbol
    state.setByString("| U+200D U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + regional indicator symbol
    state.setByString("| U+1F469 U+200D U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + end with ZERO WIDTH JOINER
    state.setByString("| U+1F1FA U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + ZERO WIDTH JOINER
    state.setByString("| U+1F1FA U+200D U+1F469");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F469");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER + emoji modifier
    state.setByString("| U+200D U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + emoji modifier
    state.setByString("| U+1F469 U+200D U+1F3FB");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Emoji modifier + end with ZERO WIDTH JOINER
    state.setByString("| U+1F466 U+1F3FB U+200D");
    forwardDelete(state, 0);
    state.assertEquals("|");

    // Emoji modifier + ZERO WIDTH JOINER
    // TODO(nona): Revive this test once HarfBuzz is updated to 2.0.2 (b/117953171)
    // state.setByString("| U+1F466 U+1F3FB U+200D U+1F469");
    // forwardDelete(state, 0);
    // state.assertEquals("|");

    // Emoji modifier + regional indicator symbol
    state.setByString("| U+1F466 U+1F3FB U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("| U+1F1FA");
    forwardDelete(state, 0);
    state.assertEquals("|");
}

} // namespace
