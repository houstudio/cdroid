/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __IME_SELECTION_CONTROLLER_H__
#define __IME_SELECTION_CONTROLLER_H__
#include <functional>
#include <string>
#include <vector>

namespace cdroid{

class CandidateView;
class InputMethod;

/* Reusable 1/2-level input-method selection logic, decoupled from any window or
 * keyboard. A keyboard host forwards it the handful of events it cannot handle
 * itself (a printable key, backspace, a candidate-strip tap), and the controller
 * drives the bound InputMethod engine plus the candidate strip:
 *
 *   - level-1 engines (e.g. the English/qwerty method, whose search() returns
 *     <0): every printable key is committed straight to the editor and the
 *     candidate strip is left untouched;
 *   - level-2 engines (GooglePinyin): the full search -> choose -> predict flow
 *     with two-level/continuous phrase selection, the composing prefix shown in
 *     the strip, and next-word predictions after each committed phrase.
 *
 * The host supplies the CandidateView to render into and a Committer that knows
 * how to deliver a committed UTF-8 string to the editor (a View::commitText, an
 * EditText, etc.), so the controller is not welded to any particular commit
 * path. */
class ImeSelectionController{
public:
    using Committer = std::function<void(const std::string& utf8)>;

    ImeSelectionController(CandidateView* cv, Committer commit);

    /* The active engine changed (method switch). Resets any composition. */
    void setInputMethod(InputMethod* im);

    /* A printable key was pressed (letter/digit/punct/space). Space while a
     * pinyin is being composed picks the best candidate instead of inserting a
     * literal space into the composing buffer. */
    void onChar(int primaryCode);

    /* A long-press popup accent was picked: commit it directly (the base char
     * was aborted by the long-press), flushing any composing, then offer
     * next-word predictions like a candidate pick. */
    void pickChar(int primaryCode);

    /* The delete key. Returns true when it edited the composing buffer (so the
     * caller does NOT also delete in the editor); false when nothing is
     * composing and the caller should forward a real editor delete. */
    bool onBackspace();

    /* A suggestion was tapped. `displayed` is the text shown in the strip at
     * `id`. While composing, the tap fixes candidate `id` (two-level selection
     * -- `displayed` is ignored, only the index matters); after a commit
     * (next-word prediction strip) it commits `displayed` directly and chains,
     * since the engine workspace is already reset and choose() would no-op. */
    void onCandidateSelected(const std::string& displayed, int id);

    /* Abandon the current composition and clear the strip (hide/switch). */
    void reset();

    bool isComposing() const { return !mComposing.empty(); }
private:
    CandidateView* mCandidates;
    InputMethod*   mIm;
    Committer      mCommit;
    std::wstring   mComposing;   // raw letters typed (pinyin or english prefix)
    /* Last candidate list shown in the strip; a conversion method's
     * space-while-composing finalizes the best (front) one. */
    std::vector<std::string> mLastCandidates;

    void refreshCandidates(const std::vector<std::string>& cands);
    void commitAndPredict(const std::string& committed);
    /* Conversion methods (pinyin) finalize the best candidate on space. */
    void commitBestCandidate();
    /* Single-level (English) only: commit the composing text as-is followed by
     * the separator char, then reset -- a word boundary flushes the buffer so
     * typing reads normally while completions stay optional. */
    void flushComposing(int separatorChar);
};

}/*endof namespace*/
#endif
