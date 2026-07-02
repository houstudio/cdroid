#ifndef __SELECTION_H__
#define __SELECTION_H__
#include <text/editable.h>
#include <text/textwatcher.h>
namespace cdroid{
class Layout;   // used only as Layout* in the move/extend signatures below
class Selection {
private:
    static constexpr char16_t PARAGRAPH_SEPARATOR = '\n';
    Selection()=default;
    static int getSelectionMemory(CharSequence* text);
    static void setSelection(Spannable* text, int start, int stop, int memory);
    static void updateMemory(Spannable* text, int memory);
    static void removeMemory(Spannable* text);
    static void extendSelection(Spannable* text, int index, int memory);
    static void setSelectionAndMemory(Spannable* text, Layout* layout, int line, int end,
            int direction, bool extend);
    static int findEdge(Spannable* text, Layout* layout, int dir);
    static int chooseHorizontal(Layout* layout, int direction, int off1, int off2);
public:
    class MemoryTextWatcher:public TextWatcher {
    public:
        /*void beforeTextChanged(CharSequence& s, int start, int count, int after)override {}
        void onTextChanged(CharSequence& s, int start, int before, int count)override {}
        void afterTextChanged(Editable& s)override {
            s.removeSpan(SELECTION_MEMORY);
            s.removeSpan(this);
        }*/
    };
public:
    /**
     * Return the offset of the selection anchor or cursor, or -1 if
     * there is no selection or cursor.
     */
    static int getSelectionStart(CharSequence* text);
    /**
     * Return the offset of the selection edge or cursor, or -1 if
     * there is no selection or cursor.
     */
    static int getSelectionEnd(CharSequence* text);

    static void setSelection(Spannable* text, int start, int stop);

    /**
     * Move the cursor to offset <code>index</code>.
     */
    static void setSelection(Spannable* text, int index);

    /**
     * Select the entire text.
     */
    static void selectAll(Spannable* text);
    /**
     * Move the selection edge to offset <code>index</code>.
     */
    static void extendSelection(Spannable* text, int index);
    /**
     * Remove the selection or cursor, if any, from the text.
     */
    static void removeSelection(Spannable* text);

    /*
     * Moving the selection within the layout
     */

    /**
     * Move the cursor to the buffer offset physically above the current
     * offset, to the beginning if it is on the top line but not at the
     * start, or return false if the cursor is already on the top line.
     */
    static bool moveUp(Spannable* text, Layout* layout);
    /**
     * Move the cursor to the buffer offset physically below the current
     * offset, to the end of the buffer if it is on the bottom line but
     * not at the end, or return false if the cursor is already at the
     * end of the buffer.
     */
    static bool moveDown(Spannable* text, Layout* layout);

    /**
     * Move the cursor to the buffer offset physically to the left of
     * the current offset, or return false if the cursor is already
     * at the left edge of the line and there is not another line to move it to.
     */
    static bool moveLeft(Spannable* text, Layout* layout);
    /**
     * Move the cursor to the buffer offset physically to the right of
     * the current offset, or return false if the cursor is already at
     * at the right edge of the line and there is not another line
     * to move it to.
     */
    static bool moveRight(Spannable* text, Layout* layout);

    /**
     * Move the cursor to the closest paragraph start offset.
     *
     * @param text the spannable text
     * @param layout layout to be used for drawing.
     * @return true if the cursor is moved, otherwise false.
     */
    static bool moveToParagraphStart(Spannable* text, Layout* layout);
    /**
     * Move the cursor to the closest paragraph end offset.
     *
     * @param text the spannable text
     * @param layout layout to be used for drawing.
     * @return true if the cursor is moved, otherwise false.
     */
    static bool moveToParagraphEnd(Spannable* text, Layout* layout);

    /**
     * Extend the selection to the closest paragraph start offset.
     *
     * @param text the spannable text
     * @return true if the selection is extended, otherwise false
     */
    static bool extendToParagraphStart(Spannable* text);
    /**
     * Extend the selection to the closest paragraph end offset.
     *
     * @param text the spannable text
     * @return true if the selection is extended, otherwise false
     */
    static bool extendToParagraphEnd(Spannable* text);

    /**
     * Move the selection end to the buffer offset physically above
     * the current selection end.
     */
    static bool extendUp(Spannable* text, Layout* layout);
    /**
     * Move the selection end to the buffer offset physically below
     * the current selection end.
     */
    static bool extendDown(Spannable* text, Layout* layout);

    /**
     * Move the selection end to the buffer offset physically to the left of
     * the current selection end.
     */
    static bool extendLeft(Spannable* text, Layout* layout);
    /**
     * Move the selection end to the buffer offset physically to the right of
     * the current selection end.
     */
    static bool extendRight(Spannable* text, Layout* layout);

    static bool extendToLeftEdge(Spannable* text, Layout* layout);

    static bool extendToRightEdge(Spannable* text, Layout* layout);
    static bool moveToLeftEdge(Spannable* text, Layout* layout);
    static bool moveToRightEdge(Spannable* text, Layout* layout);

    class PositionIterator {
    public:
        //static int DONE = BreakIterator::DONE;
        virtual int preceding(int position) const = 0;
        virtual int following(int position) const = 0;
    };

    static bool moveToPreceding(Spannable* text, PositionIterator& iter, bool extendSelection);
    static bool moveToFollowing( Spannable* text, PositionIterator& iter, bool extendSelection);

private:
    /*static class START implements NoCopySpan { }
    static class END implements NoCopySpan { }
    static class MEMORY implements NoCopySpan { }*/
    const static ParcelableSpan* SELECTION_MEMORY;// = new MEMORY();
public:
    const static ParcelableSpan* SELECTION_START;// = new START();
    const static ParcelableSpan* SELECTION_END;// = new END();
};
}/*endof namespace*/
#endif/*__SELECTION_H__*/
