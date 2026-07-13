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
#include <core/imeselectioncontroller.h>
#include <core/inputmethod.h>
#include <utility>
#include <text/character.h>
#include <widget/candidateview.h>
#include <utils/textutils.h>
#include <porting/cdlog.h>

namespace cdroid{

ImeSelectionController::ImeSelectionController(CandidateView* cv, Committer commit)
    : mCandidates(cv)
    , mIm(nullptr)
    , mCommit(std::move(commit)){
}

void ImeSelectionController::setInputMethod(InputMethod* im){
    if(mIm != im){
        reset();
        mIm = im;
    }
}

void ImeSelectionController::reset(){
    if(mIm && isComposing()){
        mIm->closeSearch();
        mComposing.clear();
    }
    if(mCandidates) mCandidates->clear();
}

void ImeSelectionController::onChar(int primaryCode){
    if(mIm == nullptr) return;
    const bool conversion = mIm->isConversionMethod();

    // Space while composing: a conversion method (pinyin) finalizes the best
    // candidate; a literal method (english) treats it as a word boundary.
    if(primaryCode == ' ' && isComposing()){
        if(conversion) commitBestCandidate();
        else flushComposing(primaryCode);
        return;
    }
    // Only composable characters go to the engine/buffer: pinyin letters for a
    // conversion method, letters+digits for a literal method. Anything else
    // (space when not composing, punctuation, a digit in pinyin) is a separator:
    // commit it directly and never feed it to search -- pinyin only takes
    // letters, and a stray space/punct sent to the engine leaves it stuck so
    // subsequent pinyin won't update the candidate list.
    // Character::isLetter/isLetterOrDigit are Unicode-aware (any codepoint,
    // including non-BMP), unlike std::isalpha/isalnum which are UB past 0xFF.
    const bool composable = conversion ? Character::isLetter(primaryCode)
                                       : Character::isLetterOrDigit(primaryCode);
    if(!composable){
        if(!conversion && isComposing()){
            flushComposing(primaryCode);  // literal: separator follows the word
        } else {
            // Commit the codepoint as proper UTF-8 (a (char) cast would truncate
            // any codepoint > 0xFF).
            mCommit(TextUtils::unicode2utf8(std::wstring(1, (wchar_t)primaryCode)));
        }
        return;
    }
    mComposing.append(1, primaryCode);
    const std::string u8txt = TextUtils::unicode2utf8(mComposing);
    std::vector<std::string> candidates;
    const int rc = mIm->search(u8txt, candidates);
    if(rc < 0){
        // Engine with no candidate search: commit the typed character straight
        // to the editor.
        mCommit(u8txt);
        mComposing.clear();
    } else {
        refreshCandidates(candidates);
    }
    LOGD("onChar txt=%s primaryCode=%x/%c rc=%d", u8txt.c_str(), primaryCode, primaryCode, rc);
}

bool ImeSelectionController::onBackspace(){
    if(mIm == nullptr || !isComposing()) return false;
    // One-level: drop the last typed char and re-search.
    mComposing.pop_back();
    if(mComposing.empty()){
        mIm->closeSearch();
        if(mCandidates) mCandidates->clear();
        mLastCandidates.clear();
    } else {
        std::vector<std::string> candidates;
        mIm->search(TextUtils::unicode2utf8(mComposing), candidates);
        refreshCandidates(candidates);
    }
    return true;
}

void ImeSelectionController::onCandidateSelected(const std::string& displayed, int /*id*/){
    // One-level selection: a tap commits the shown candidate (the strip mirrors
    // the engine's candidate list, so `displayed` is the word/phrase itself)
    // and chains to next-word predictions. Works the same for a composing
    // candidate and a post-commit prediction.
    if(mIm) mIm->closeSearch();
    mComposing.clear();
    commitAndPredict(displayed);
}

/* Show `candidates` in the strip (cached so a conversion method's space-while-
 * composing can finalize the best one). */
void ImeSelectionController::refreshCandidates(const std::vector<std::string>& candidates){
    if(mCandidates == nullptr) return;
    mLastCandidates = candidates;
    std::vector<std::string> shown = candidates;
    /* For a conversion method (pinyin) the engine only returns hanzi candidates,
     * so prepend the raw typed pinyin as the first item -- the user sees what
     * they entered, and backspace edits it (onBackspace drops a composing char).
     * A literal method's engine already includes the typed prefix as
     * candidates[0], so it needs no prepending. mLastCandidates keeps the raw
     * engine list so space-while-composing still finalizes the best hanzi. */
    if(mIm && mIm->isConversionMethod() && !mComposing.empty()){
        shown.insert(shown.begin(), TextUtils::unicode2utf8(mComposing));
    }
    mCandidates->setSuggestions(shown, true, true);
    LOGD("%d suggestions (pinyin='%ls')", (int)shown.size(), mComposing.c_str());
    mCandidates->invalidate(true);
}

/* Conversion methods (pinyin) use space-while-composing to finalize the best
 * candidate (the pinyin convention) instead of inserting a literal space. */
void ImeSelectionController::commitBestCandidate(){
    if(mLastCandidates.empty()) return;
    if(mIm) mIm->closeSearch();
    mComposing.clear();
    commitAndPredict(mLastCandidates.front());
}

/* A long-press popup accent pick: commit the chosen char directly (the base
 * char was aborted by the long-press), flushing any composing, then offer
 * next-word predictions like a candidate pick. */
void ImeSelectionController::pickChar(int primaryCode){
    if(mIm) mIm->closeSearch();
    mComposing.clear();
    std::wstring w(1, (wchar_t)primaryCode);
    commitAndPredict(TextUtils::unicode2utf8(w));
}

/* Commit a finished phrase/word to the editor, then offer next-word predictions
 * based on it. Used both when a composing choice completes the whole pinyin and
 * when the user taps a prediction from the strip. */
void ImeSelectionController::commitAndPredict(const std::string& committed){
    mCommit(committed);
    if(mCandidates == nullptr) return;
    std::vector<std::string> predicts;
    if(mIm && mIm->getPredicts(committed, predicts) >= 0)
        mCandidates->setSuggestions(predicts, true, true);
    else
        mCandidates->clear();
    LOGD("committed '%s'", committed.c_str());
}

void ImeSelectionController::flushComposing(int separatorChar){
    if(mComposing.empty()) return;
    std::wstring all = mComposing;
    all += (wchar_t)separatorChar;
    mCommit(TextUtils::unicode2utf8(all));
    mComposing.clear();
    if(mIm) mIm->closeSearch();
    if(mCandidates) mCandidates->clear();
}

}/*endof namespace*/
