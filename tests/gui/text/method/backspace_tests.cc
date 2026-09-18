/*********************************************************************************
 * Ported from AOSP coretests android.text.method.BackspaceTest (Apache 2.0).
 * Test backspace key handling of android.text.method.BaseKeyListener.
 *
 * Only contains edge cases. For normal cases, see the CTS BackspaceTest.
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization):
 * CDROID's BaseKeyListener::getOffsetForBackspaceKey is a Phase-1 stub that
 * deletes one BMP UTF-16 unit instead of running Android's grapheme/emoji
 * (ZWJ/VS/RIS/keycap/tag) cluster state machine. Every case below whose
 * expectation deletes a whole cluster (astral chars, keycap/VS/ZWJ/RIS/tag
 * sequences) therefore deletes only one code unit and mismatches. Isolated
 * single-BMP-unit cases still pass. Same root cause as the recorded Emoji
 * cluster gap family (StaticLayoutTest.testEmojiOffset).
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

class BackspaceTest : public testing::Test {
protected:
    // AOSP syncs the state into an EditText and calls TextView.onKeyDown, which
    // routes KEYCODE_DEL to BaseKeyListener through Editor. CDROID's Editor
    // cursor-invalidation path cannot run on an unattached view
    // (getTextCursorDrawable() fabricates a drawable where AOSP's
    // instrumentation context resolves none, and there is no text layout), so
    // this port drives the unit under test — BaseKeyListener::backspace, per
    // the AOSP class comment — directly on the state's Editable. The host View
    // only satisfies the listener signature (deleteLineFromCursor needs ALT,
    // which the corpus never uses).
    void backspace(EditorState& state, int modifiers) {
        Selection::setSelection(dynamic_cast<Spannable*>(state.mText),
                state.mSelectionStart, state.mSelectionEnd);

        const nsecs_t now = SystemClock::uptimeMillis();
        KeyEvent* keyEvent = KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN,
                KeyEvent::KEYCODE_DEL, 0, modifiers, 0, 0, 0, 0);
        TestBaseKeyListener keyListener;
        EditText host(&App::getInstance());
        keyListener.backspace(host, *state.mText, KeyEvent::KEYCODE_DEL, *keyEvent);
        delete keyEvent;

        state.mSelectionStart = Selection::getSelectionStart(state.mText);
        state.mSelectionEnd = Selection::getSelectionEnd(state.mText);
    }
};

TEST_F(BackspaceTest, testCombiningEnclosingKeycaps) {
    EditorState state;

    state.setByString("'1' U+E0101 U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // multiple COMBINING ENCLOSING KEYCAP
    state.setByString("'1' U+20E3 U+20E3 |");
    backspace(state, 0);
    state.assertEquals("'1' U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated COMBINING ENCLOSING KEYCAP
    state.setByString("U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated multiple COMBINING ENCLOSING KEYCAP
    state.setByString("U+20E3 U+20E3 |");
    backspace(state, 0);
    state.assertEquals("U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");
}

TEST_F(BackspaceTest, testVariationSelector) {
    EditorState state;

    // Isolated variation selector
    state.setByString("U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+E0100 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated multiple variation selectors
    state.setByString("U+FE0F U+FE0F |");
    backspace(state, 0);
    state.assertEquals("U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+FE0F U+E0100 |");
    backspace(state, 0);
    state.assertEquals("U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+E0100 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("U+E0100 |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+E0100 U+E0100 |");
    backspace(state, 0);
    state.assertEquals("U+E0100 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Multiple variation selectors
    state.setByString("'#' U+FE0F U+FE0F |");
    backspace(state, 0);
    state.assertEquals("'#' U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("'#' U+FE0F U+E0100 |");
    backspace(state, 0);
    state.assertEquals("'#' U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+845B U+E0100 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("U+845B U+E0100 |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+845B U+E0100 U+E0100 |");
    backspace(state, 0);
    state.assertEquals("U+845B U+E0100 |");
    backspace(state, 0);
    state.assertEquals("|");
}

TEST_F(BackspaceTest, testEmojiZWJSequence) {
    EditorState state;

    // U+200D is ZERO WIDTH JOINER.
    state.setByString("U+1F441 U+200D U+1F5E8 |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+1F441 U+200D U+1F5E8 U+FE0E |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+1F469 U+200D U+1F373 |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+1F487 U+200D U+2640 |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+1F487 U+200D U+2640 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+1F468 U+200D U+2764 U+FE0F U+200D U+1F48B U+200D U+1F468 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Emoji modifier can be appended to the first emoji.
    state.setByString("U+1F469 U+1F3FB U+200D U+1F4BC |");
    backspace(state, 0);
    state.assertEquals("|");

    // End with ZERO WIDTH JOINER
    state.setByString("U+1F441 U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F441 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER
    state.setByString("U+200D U+1F5E8 |");
    backspace(state, 0);
    state.assertEquals("U+200D |");
    backspace(state, 0);
    state.assertEquals("|");

    state.setByString("U+FE0E U+200D U+1F5E8 |");
    backspace(state, 0);
    state.assertEquals("U+FE0E U+200D |");
    backspace(state, 0);
    state.assertEquals("U+FE0E |");
    backspace(state, 0);
    state.assertEquals("|");

    // Multiple ZERO WIDTH JOINER
    state.setByString("U+1F441 U+200D U+200D U+1F5E8 |");
    backspace(state, 0);
    state.assertEquals("U+1F441 U+200D U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F441 U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F441 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated ZERO WIDTH JOINER
    state.setByString("U+200D |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated multiple ZERO WIDTH JOINER
    state.setByString("U+200D U+200D |");
    backspace(state, 0);
    state.assertEquals("U+200D |");
    backspace(state, 0);
    state.assertEquals("|");
}

TEST_F(BackspaceTest, testFlags) {
    EditorState state;

    // Isolated regional indicator symbol
    state.setByString("U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("|");

    // Odd numbered regional indicator symbols
    state.setByString("U+1F1FA U+1F1F8 U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("U+1F1FA U+1F1F8 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Incomplete sequence. (no tag_term: U+E007E)
    state.setByString("'a' U+1F3F4 U+E0067 'b' |");
    backspace(state, 0);
    state.assertEquals("'a' U+1F3F4 U+E0067 |");
    backspace(state, 0);
    state.assertEquals("'a' U+1F3F4 |");
    backspace(state, 0);
    state.assertEquals("'a' |");

    // No tag_base
    state.setByString("'a' U+E0067 U+E007F 'b' |");
    backspace(state, 0);
    state.assertEquals("'a' U+E0067 U+E007F |");
    backspace(state, 0);
    state.assertEquals("'a' U+E0067 |");
    backspace(state, 0);
    state.assertEquals("'a' |");

    // Isolated tag chars
    state.setByString("'a' U+E0067 U+E0067 'b' |");
    backspace(state, 0);
    state.assertEquals("'a' U+E0067 U+E0067 |");
    backspace(state, 0);
    state.assertEquals("'a' U+E0067 |");
    backspace(state, 0);
    state.assertEquals("'a' |");

    // Isolated tab term.
    state.setByString("'a' U+E007F U+E007F 'b' |");
    backspace(state, 0);
    state.assertEquals("'a' U+E007F U+E007F |");
    backspace(state, 0);
    state.assertEquals("'a' U+E007F |");
    backspace(state, 0);
    state.assertEquals("'a' |");

    // Immediate tag_term after tag_base
    state.setByString("'a' U+1F3F4 U+E007F U+1F3F4 U+E007F 'b' |");
    backspace(state, 0);
    state.assertEquals("'a' U+1F3F4 U+E007F U+1F3F4 U+E007F |");
    backspace(state, 0);
    state.assertEquals("'a' U+1F3F4 U+E007F |");
    backspace(state, 0);
    state.assertEquals("'a' |");
}

TEST_F(BackspaceTest, testEmojiModifier) {
    EditorState state;

    // U+1F3FB is EMOJI MODIFIER FITZPATRICK TYPE-1-2.
    state.setByString("U+1F466 U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated emoji modifier
    state.setByString("U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");

    // Isolated multiple emoji modifier
    state.setByString("U+1F3FB U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");

    // Multiple emoji modifiers
    state.setByString("U+1F466 U+1F3FB U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("U+1F466 U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");
}

TEST_F(BackspaceTest, testMixedEdgeCases) {
    EditorState state;

    // COMBINING ENCLOSING KEYCAP + variation selector
    state.setByString("'1' U+20E3 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("'1' |");
    backspace(state, 0);
    state.assertEquals("|");

    // Variation selector + COMBINING ENCLOSING KEYCAP
    state.setByString("U+2665 U+FE0F U+20E3 |");
    backspace(state, 0);
    state.assertEquals("U+2665 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + ending with ZERO WIDTH JOINER
    state.setByString("'1' U+20E3 U+200D |");
    backspace(state, 0);
    state.assertEquals("'1' U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + ZERO WIDTH JOINER
    state.setByString("'1' U+20E3 U+200D U+1F5E8 |");
    backspace(state, 0);
    state.assertEquals("'1' U+20E3 U+200D |");
    backspace(state, 0);
    state.assertEquals("'1' U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER + COMBINING ENCLOSING KEYCAP
    state.setByString("U+200D U+20E3 |");
    backspace(state, 0);
    state.assertEquals("U+200D |");
    backspace(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + COMBINING ENCLOSING KEYCAP
    state.setByString("U+1F441 U+200D U+20E3 |");
    backspace(state, 0);
    state.assertEquals("U+1F441 U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F441 |");
    backspace(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + regional indicator symbol
    state.setByString("'1' U+20E3 U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("'1' U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + COMBINING ENCLOSING KEYCAP
    state.setByString("U+1F1FA U+20E3 |");
    backspace(state, 0);
    state.assertEquals("U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("|");

    // COMBINING ENCLOSING KEYCAP + emoji modifier
    state.setByString("'1' U+20E3 U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("'1' U+20E3 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Emoji modifier + COMBINING ENCLOSING KEYCAP
    state.setByString("U+1F466 U+1F3FB U+20E3 |");
    backspace(state, 0);
    state.assertEquals("U+1f466 U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");

    // Variation selector + end with ZERO WIDTH JOINER
    state.setByString("U+2665 U+FE0F U+200D |");
    backspace(state, 0);
    state.assertEquals("U+2665 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    // Variation selector + ZERO WIDTH JOINER
    state.setByString("U+1F469 U+200D U+2764 U+FE0F U+200D U+1F469 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER + variation selector
    state.setByString("U+200D U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + variation selector
    state.setByString("U+1F469 U+200D U+FE0F |");
    backspace(state, 0);
    state.assertEquals("U+1F469 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Variation selector + regional indicator symbol
    state.setByString("U+2665 U+FE0F U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("U+2665 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + variation selector
    state.setByString("U+1F1FA U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    // Variation selector + emoji modifier
    state.setByString("U+2665 U+FE0F U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("U+2665 U+FE0F |");
    backspace(state, 0);
    state.assertEquals("|");

    // Emoji modifier + variation selector
    state.setByString("U+1F466 U+1F3FB U+FE0F |");
    backspace(state, 0);
    state.assertEquals("U+1F466 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Start withj ZERO WIDTH JOINER + regional indicator symbol
    state.setByString("U+200D U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("U+200D |");
    backspace(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + Regional indicator symbol
    state.setByString("U+1F469 U+200D U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("U+1F469 U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F469 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + end with ZERO WIDTH JOINER
    state.setByString("U+1F1FA U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + ZERO WIDTH JOINER
    state.setByString("U+1F1FA U+200D U+1F469 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Start with ZERO WIDTH JOINER + emoji modifier
    state.setByString("U+200D U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("U+200D |");
    backspace(state, 0);
    state.assertEquals("|");

    // ZERO WIDTH JOINER + emoji modifier
    state.setByString("U+1F469 U+200D U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("U+1F469 U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F469 |");
    backspace(state, 0);
    state.assertEquals("|");

    // Emoji modifier + end with ZERO WIDTH JOINER
    state.setByString("U+1F466 U+1F3FB U+200D |");
    backspace(state, 0);
    state.assertEquals("U+1F466 U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");

    // Regional indicator symbol + Emoji modifier
    state.setByString("U+1F1FA U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("|");

    // Emoji modifier + regional indicator symbol
    state.setByString("U+1F466 U+1F3FB U+1F1FA |");
    backspace(state, 0);
    state.assertEquals("U+1F466 U+1F3FB |");
    backspace(state, 0);
    state.assertEquals("|");

    // RIS + LF
    state.setByString("U+1F1E6 U+000A |");
    backspace(state, 0);
    state.assertEquals("U+1F1E6 |");
    backspace(state, 0);
    state.assertEquals("|");
}

} // namespace
