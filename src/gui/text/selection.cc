#include <text/layout.h>
#include <text/selection.h>
#include <text/parcelablespan.h>
namespace cdroid{

namespace {
// Selection cursor markers. NoCopySpan so the owning Spannable treats them as
// BORROWED: it never deletes them (they are process-lifetime singletons) and
// never clones them on text copy — matching Android's
// `static final class START/END/MEMORY implements NoCopySpan`. Only pointer
// identity matters, which the singleton instance guarantees.
struct SelectionStartMarker  : public NoCopySpan {};
struct SelectionEndMarker    : public NoCopySpan {};
struct SelectionMemoryMarker : public NoCopySpan {};
} // namespace

// ODR-use definitions of the selection marker spans. Declared as `const static`
// pointers in the header but never defined before, which is why the Selection
// API could not actually be linked/called. Distinct process-lifetime singletons;
// only pointer identity matters.
const ParcelableSpan* Selection::SELECTION_MEMORY = new SelectionMemoryMarker();
const ParcelableSpan* Selection::SELECTION_START  = new SelectionStartMarker();
const ParcelableSpan* Selection::SELECTION_END    = new SelectionEndMarker();

int Selection::getSelectionStart(CharSequence* text) {
    Spanned*spanned = dynamic_cast<Spanned*>(text);
    if (spanned) {
        return spanned->getSpanStart(SELECTION_START);
    }
    return -1;
}

int Selection::getSelectionEnd(CharSequence* text) {
    Spanned*spanned = dynamic_cast<Spanned*>(text);
    if (spanned) {
        return spanned->getSpanStart(SELECTION_END);
    }
    return -1;
}

int Selection::getSelectionMemory(CharSequence* text) {
    Spanned*spanned = dynamic_cast<Spanned*>(text);
    if (spanned) {
        return spanned->getSpanStart(SELECTION_MEMORY);
    }
    return -1;
}

void Selection::setSelection(Spannable* text, int start, int stop) {
    setSelection(text, start, stop, -1);
}

void Selection::setSelection(Spannable* text, int start, int stop, int memory) {
    // int len = text.length();
    // start = pin(start, 0, len);  XXX remove unless we really need it
    // stop = pin(stop, 0, len);

    int ostart = getSelectionStart(text);
    int oend = getSelectionEnd(text);

    if (ostart != start || oend != stop) {
        text->setSpan(SELECTION_START, start, start,
                Spanned::SPAN_POINT_POINT | Spanned::SPAN_INTERMEDIATE);
        text->setSpan(SELECTION_END, stop, stop, Spanned::SPAN_POINT_POINT);
        updateMemory(text, memory);
    }
}

void Selection::updateMemory(Spannable* text, int memory) {
    if (memory > -1) {
        int currentMemory = getSelectionMemory(text);
        if (memory != currentMemory) {
            text->setSpan(SELECTION_MEMORY, memory, memory, Spanned::SPAN_POINT_POINT);
            if (currentMemory == -1) {
                // This is the first value, create a watcher.
                TextWatcher* watcher = new MemoryTextWatcher();
                text->setSpan(watcher, 0, text->length(), Spanned::SPAN_INCLUSIVE_INCLUSIVE);
            }
        }
    } else {
        removeMemory(text);
    }
}

void Selection::removeMemory(Spannable* text) {
    text->removeSpan(SELECTION_MEMORY);
    auto watchers = text->getSpans(0, text->length(), make_span_filter<MemoryTextWatcher>());
    for (auto watcher : watchers) {
        text->removeSpan(watcher);
        // MemoryTextWatcher is a NoCopySpan (via TextWatcher), so the Spannable
        // treats it as borrowed and never deletes it. Selection owns it and
        // frees it here. The object was a non-const `new MemoryTextWatcher()`,
        // so casting away const to delete is well-defined.
        delete const_cast<MemoryTextWatcher*>(
                dynamic_cast<const MemoryTextWatcher*>(watcher));
    }
}

void Selection::setSelection(Spannable* text, int index) {
    setSelection(text, index, index);
}

void Selection::selectAll(Spannable* text) {
    setSelection(text, 0, text->length());
}

void Selection::extendSelection(Spannable* text, int index) {
    extendSelection(text, index, -1);
}

void Selection::extendSelection(Spannable* text, int index, int memory) {
    if (text->getSpanStart(SELECTION_END) != index) {
        text->setSpan(SELECTION_END, index, index, Spanned::SPAN_POINT_POINT);
    }
    updateMemory(text, memory);
}

void Selection::removeSelection(Spannable* text) {
    text->removeSpan(SELECTION_START/*, Spanned::SPAN_INTERMEDIATE*/);
    text->removeSpan(SELECTION_END);
    removeMemory(text);
}

bool Selection::moveUp(Spannable* text, Layout* layout) {
    int start = getSelectionStart(text);
    int end = getSelectionEnd(text);

    if (start != end) {
        int min = std::min(start, end);
        int max = std::max(start, end);

        setSelection(text, min);

        if (min == 0 && max == text->length()) {
            return false;
        }

        return true;
    } else {
        int line = layout->getLineForOffset(end);

        if (line > 0) {
            setSelectionAndMemory(
                    text, layout, line, end, -1 /* direction */, false /* extend */);
            return true;
        } else if (end != 0) {
            setSelection(text, 0);
            return true;
        }
    }

    return false;
}

void Selection::setSelectionAndMemory(Spannable* text, Layout* layout, int line, int end,
        int direction, bool extend) {
    int move;
    int newMemory;

    if (layout->getParagraphDirection(line)
            == layout->getParagraphDirection(line + direction)) {
        int memory = getSelectionMemory(text);
        if (memory > -1) {
            // We have a memory position
            float h = layout->getPrimaryHorizontal(memory);
            move = layout->getOffsetForHorizontal(line + direction, h);
            newMemory = memory;
        } else {
            // Create a new memory position
            float h = layout->getPrimaryHorizontal(end);
            move = layout->getOffsetForHorizontal(line + direction, h);
            newMemory = end;
        }
    } else {
        move = layout->getLineStart(line + direction);
        newMemory = -1;
    }

    if (extend) {
        extendSelection(text, move, newMemory);
    } else {
        setSelection(text, move, move, newMemory);
    }
}

bool Selection::moveDown(Spannable* text, Layout* layout) {
    int start = getSelectionStart(text);
    int end = getSelectionEnd(text);

    if (start != end) {
        int min = std::min(start, end);
        int max = std::max(start, end);

        setSelection(text, max);

        if (min == 0 && max == text->length()) {
            return false;
        }

        return true;
    } else {
        int line = layout->getLineForOffset(end);

        if (line < layout->getLineCount() - 1) {
            setSelectionAndMemory(
                    text, layout, line, end, 1 /* direction */, false /* extend */);
            return true;
        } else if (end != text->length()) {
            setSelection(text, text->length());
            return true;
        }
    }

    return false;
}

bool Selection::moveLeft(Spannable* text, Layout* layout) {
    int start = getSelectionStart(text);
    int end = getSelectionEnd(text);

    if (start != end) {
        setSelection(text, chooseHorizontal(layout, -1, start, end));
        return true;
    } else {
        int to = layout->getOffsetToLeftOf(end);

        if (to != end) {
            setSelection(text, to);
            return true;
        }
    }

    return false;
}

bool Selection::moveRight(Spannable* text, Layout* layout) {
    int start = getSelectionStart(text);
    int end = getSelectionEnd(text);

    if (start != end) {
        setSelection(text, chooseHorizontal(layout, 1, start, end));
        return true;
    } else {
        int to = layout->getOffsetToRightOf(end);

        if (to != end) {
            setSelection(text, to);
            return true;
        }
    }

    return false;
}

bool Selection::moveToParagraphStart(Spannable* text, Layout* layout) {
    int start = getSelectionStart(text);
    int end = getSelectionEnd(text);

    if (start != end) {
        setSelection(text, chooseHorizontal(layout, -1, start, end));
        return true;
    } else {
        int to = TextUtils::lastIndexOf(text, PARAGRAPH_SEPARATOR, start - 1);
        if (to == -1) {
            to = 0;  // If not found, use the document start offset as a paragraph start.
        }
        if (to != end) {
            setSelection(text, to);
            return true;
        }
    }
    return false;
}

bool Selection::moveToParagraphEnd(Spannable* text, Layout* layout) {
    int start = getSelectionStart(text);
    int end = getSelectionEnd(text);

    if (start != end) {
        setSelection(text, chooseHorizontal(layout, 1, start, end));
        return true;
    } else {
        int to = TextUtils::indexOf(text, PARAGRAPH_SEPARATOR, end + 1);
        if (to == -1) {
            to = text->length();
        }
        if (to != end) {
            setSelection(text, to);
            return true;
        }
    }
    return false;
}

bool Selection::extendToParagraphStart(Spannable* text) {
    int end = getSelectionEnd(text);
    int to = TextUtils::lastIndexOf(text, PARAGRAPH_SEPARATOR, end - 1);
    if (to == -1) {
        to = 0;  // If not found, use the document start offset as a paragraph start.
    }
    if (to != end) {
        extendSelection(text, to);
        return true;
    }
    return false;
}

bool Selection::extendToParagraphEnd(Spannable* text) {
    int end = getSelectionEnd(text);
    int to = TextUtils::indexOf(text, PARAGRAPH_SEPARATOR, end + 1);
    if (to == -1) {
        to = text->length();
    }
    if (to != end) {
        extendSelection(text, to);
        return true;
    }
    return false;
}

bool Selection::extendUp(Spannable* text, Layout* layout) {
    int end = getSelectionEnd(text);
    int line = layout->getLineForOffset(end);

    if (line > 0) {
        setSelectionAndMemory(text, layout, line, end, -1 /* direction */, true /* extend */);
        return true;
    } else if (end != 0) {
        extendSelection(text, 0);
        return true;
    }

    return true;
}

bool Selection::extendDown(Spannable* text, Layout* layout) {
    int end = getSelectionEnd(text);
    int line = layout->getLineForOffset(end);

    if (line < layout->getLineCount() - 1) {
        setSelectionAndMemory(text, layout, line, end, 1 /* direction */, true /* extend */);
        return true;
    } else if (end != text->length()) {
        extendSelection(text, text->length(), -1);
        return true;
    }

    return true;
}

bool Selection::extendLeft(Spannable* text, Layout* layout) {
    int end = getSelectionEnd(text);
    int to = layout->getOffsetToLeftOf(end);

    if (to != end) {
        extendSelection(text, to);
        return true;
    }

    return true;
}

bool Selection::extendRight(Spannable* text, Layout* layout) {
    int end = getSelectionEnd(text);
    int to = layout->getOffsetToRightOf(end);

    if (to != end) {
        extendSelection(text, to);
        return true;
    }

    return true;
}

bool Selection::extendToLeftEdge(Spannable* text, Layout* layout) {
    int where = findEdge(text, layout, -1);
    extendSelection(text, where);
    return true;
}

bool Selection::extendToRightEdge(Spannable* text, Layout* layout) {
    int where = findEdge(text, layout, 1);
    extendSelection(text, where);
    return true;
}

bool Selection::moveToLeftEdge(Spannable* text, Layout* layout) {
    int where = findEdge(text, layout, -1);
    setSelection(text, where);
    return true;
}

bool Selection::moveToRightEdge(Spannable* text, Layout* layout) {
    int where = findEdge(text, layout, 1);
    setSelection(text, where);
    return true;
}

bool Selection::moveToPreceding(Spannable* text, PositionIterator& iter, bool _extendSelection) {
    /*const int offset = iter.preceding(getSelectionEnd(text));
    if (offset != PositionIterator.DONE) {
        if (_extendSelection) {
            extendSelection(text, offset);
        } else {
            setSelection(text, offset);
        }
    }*/
    return true;
}

bool Selection::moveToFollowing( Spannable* text, PositionIterator& iter, bool _extendSelection) {
    /*const int offset = iter.following(getSelectionEnd(text));
    if (offset != PositionIterator.DONE) {
        if (_extendSelection) {
            extendSelection(text, offset);
        } else {
            setSelection(text, offset);
        }
    }*/
    return true;
}

int Selection::findEdge(Spannable* text, Layout* layout, int dir) {
    int pt = getSelectionEnd(text);
    int line = layout->getLineForOffset(pt);
    int pdir = layout->getParagraphDirection(line);

    if (dir * pdir < 0) {
        return layout->getLineStart(line);
    } else {
        int end = layout->getLineEnd(line);

        if (line == layout->getLineCount() - 1)
            return end;
        else
            return end - 1;
    }
}

int Selection::chooseHorizontal(Layout* layout, int direction, int off1, int off2) {
    int line1 = layout->getLineForOffset(off1);
    int line2 = layout->getLineForOffset(off2);

    if (line1 == line2) {
        // same line, so it goes by pure physical direction

        float h1 = layout->getPrimaryHorizontal(off1);
        float h2 = layout->getPrimaryHorizontal(off2);

        if (direction < 0) {
            // to left

            if (h1 < h2)
                return off1;
            else
                return off2;
        } else {
            // to right

            if (h1 > h2)
                return off1;
            else
                return off2;
        }
    } else {
        // different line, so which line is "left" and which is "right"
        // depends upon the directionality of the text

        // This only checks at one end, but it's not clear what the
        // right thing to do is if the ends don't agree.  Even if it
        // is wrong it should still not be too bad.
        int line = layout->getLineForOffset(off1);
        int textdir = layout->getParagraphDirection(line);

        if (textdir == direction)
            return std::max(off1, off2);
        else
            return std::min(off1, off2);
    }
}
#if 0
private static final class START implements NoCopySpan { }
private static final class END implements NoCopySpan { }
private static final class MEMORY implements NoCopySpan { }
private static final Object SELECTION_MEMORY = new MEMORY();

public static final Object SELECTION_START = new START();
public static final Object SELECTION_END = new END();
#endif
}/*endof namespace*/
