#ifndef __EDITOR_INFO_H__
#define __EDITOR_INFO_H__

#include <text/inputtype.h>

namespace cdroid {

// Ported from android.view.inputmethod.EditorInfo (android-36).
//
// Android's EditorInfo is declared as
//   public final class EditorInfo implements InputType, Parcelable
// — it inherits all of android.text.InputType's TYPE_* constants, so e.g.
// EditorInfo.TYPE_NULL / EditorInfo.TYPE_CLASS_TEXT are valid. We mirror that by
// deriving from InputType. CDROID runs its IME in-process and intentionally has
// no InputConnection / EditorInfo / ExtractedText protocol (see
// InputMethodManager), so the Parcelable body, the Bundle extras, and the
// per-editor instance fields are NOT ported. Only the IME action / flag
// constants are kept here — in their Android names and numeric values — so
// callers can write setImeOptions(EditorInfo::IME_ACTION_DONE) and TextView can
// mask the action out of imeOptions with IME_MASK_ACTION exactly as AOSP does.
// The per-editor data (imeOptions, actionLabel, actionId) lives in
// Editor::InputContentType; inputType lives in Editor + InputType.
class EditorInfo : public InputType {
public:
    // Set of bits in imeOptions that encode the "enter"-key action.
    static constexpr int IME_MASK_ACTION = 0x000000ff;

    // Bits of IME_MASK_ACTION: no specific action; let the editor decide.
    static constexpr int IME_ACTION_UNSPECIFIED = 0x00000000;
    // Bits of IME_MASK_ACTION: there is no available action.
    static constexpr int IME_ACTION_NONE = 0x00000001;
    // Bits of IME_MASK_ACTION: "go" — take the user to the target of the text
    // they typed (e.g. entering a URL).
    static constexpr int IME_ACTION_GO = 0x00000002;
    // Bits of IME_MASK_ACTION: "search" — go to the results of searching for the
    // text the user typed.
    static constexpr int IME_ACTION_SEARCH = 0x00000003;
    // Bits of IME_MASK_ACTION: "send" — deliver the text to its target (e.g.
    // composing an IM/SMS where sending is immediate).
    static constexpr int IME_ACTION_SEND = 0x00000004;
    // Bits of IME_MASK_ACTION: "next" — move to the next field that accepts text.
    static constexpr int IME_ACTION_NEXT = 0x00000005;
    // Bits of IME_MASK_ACTION: "done" — nothing more to input; the IME will close.
    static constexpr int IME_ACTION_DONE = 0x00000006;
    // Bits of IME_MASK_ACTION: like IME_ACTION_NEXT, but move to the previous
    // field. Normally not used to specify an action; returned to the app when
    // IME_FLAG_NAVIGATE_PREVIOUS is set.
    static constexpr int IME_ACTION_PREVIOUS = 0x00000007;

    // Flag of imeOptions: the IME should not update personalized data (typing
    // history, personalized language model) based on what the user typed here.
    static constexpr int IME_FLAG_NO_PERSONALIZED_LEARNING = 0x1000000;
    // Flag of imeOptions: the IME should never go into fullscreen mode.
    static constexpr int IME_FLAG_NO_FULLSCREEN = 0x2000000;
    // Flag of imeOptions: there is something a backward navigation can focus on
    // (surfaces to the app as IME_ACTION_PREVIOUS).
    static constexpr int IME_FLAG_NAVIGATE_PREVIOUS = 0x4000000;
    // Flag of imeOptions: there is something a forward navigation can focus on.
    // Like IME_ACTION_NEXT but allows multi-line + forward navigation (surfaces
    // to the app as IME_ACTION_NEXT).
    static constexpr int IME_FLAG_NAVIGATE_NEXT = 0x8000000;
    // Flag of imeOptions: the IME need not show its extracted-text UI.
    static constexpr int IME_FLAG_NO_EXTRACT_UI = 0x10000000;
    // Flag of imeOptions: the action should not appear as an accessory button
    // next to the extracted text in fullscreen mode.
    static constexpr int IME_FLAG_NO_ACCESSORY_ACTION = 0x20000000;
    // Flag of imeOptions: the action should not replace the "enter" key inline
    // (e.g. for irreversible actions like sending). TextView auto-sets this on
    // multi-line text views.
    static constexpr int IME_FLAG_NO_ENTER_ACTION = 0x40000000;
    // Flag of imeOptions: request an IME capable of ASCII input (typically used
    // for account ID / password fields).
    static constexpr int IME_FLAG_FORCE_ASCII = 0x80000000;

    // Flag of internalImeOptions: the app window containing this EditorInfo is
    // in portrait orientation. (@hide)
    static constexpr int IME_INTERNAL_FLAG_APP_WINDOW_PORTRAIT = 0x00000001;

    // Generic unspecified value for imeOptions.
    static constexpr int IME_NULL = 0x00000000;
};

} // namespace cdroid

#endif // __EDITOR_INFO_H__
