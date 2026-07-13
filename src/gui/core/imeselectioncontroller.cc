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
#include <cctype>
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
    const bool twoLevel = mIm->supportsTwoLevel();

    if(twoLevel && primaryCode == ' ' && isComposing()){
        // Pinyin convention: space while composing finalizes the best phrase.
        chooseCandidate(0);
        return;
    }
    if(!twoLevel){
        // Single-level (English word-completion): a non-alphanumeric key is a
        // word boundary. With text being composed it flushes the typed text +
        // the separator; with nothing composed the separator is just committed
        // directly (a standalone space/punct). This keeps plain typing normal
        // while completions stay an optional tap in the strip.
        if(isComposing() && !std::isalnum(primaryCode)){
            flushComposing(primaryCode);
            return;
        }
        if(!isComposing() && !std::isalnum(primaryCode)){
            mCommit(std::string(1, (char)primaryCode));
            return;
        }
    }
    mComposing.append(1, primaryCode);
    const std::string u8txt = TextUtils::unicode2utf8(mComposing);
    std::vector<std::string> candidates;
    const int rc = mIm->search(u8txt, candidates);
    if(rc < 0){
        // Level-1 engine with no candidate search: commit the typed character
        // straight to the editor.
        mCommit(u8txt);
        mComposing.clear();
    } else {
        refreshCandidates(candidates);
    }
    LOGD("onChar txt=%s primaryCode=%x/%c rc=%d", u8txt.c_str(), primaryCode, primaryCode, rc);
}

bool ImeSelectionController::onBackspace(){
    if(mIm == nullptr || !isComposing()) return false;
    if(!mIm->fixedString().empty()){
        // choose phase: revert the last fixed word
        std::vector<std::string> candidates;
        mIm->cancelLastChoice(candidates);
        refreshCandidates(candidates);
    } else {
        // search phase: drop the last typed char and re-search
        mComposing.pop_back();
        if(mComposing.empty()){
            mIm->closeSearch();
            if(mCandidates) mCandidates->clear();
        } else {
            std::vector<std::string> candidates;
            mIm->search(TextUtils::unicode2utf8(mComposing), candidates);
            refreshCandidates(candidates);
        }
    }
    return true;
}

void ImeSelectionController::onCandidateSelected(const std::string& displayed, int id){
    if(isComposing() && mIm && !mIm->supportsTwoLevel()){
        // Single-level completion (English): a tap commits the whole shown word
        // (the strip mirrors the engine's candidates, so `displayed` is the word
        // itself) and resets the composition.
        mIm->closeSearch();
        mComposing.clear();
        commitAndPredict(displayed);
        return;
    }
    if(isComposing()){
        // Two-level composing: the suggestions are search/choose candidates;
        // tapping one fixes it (continuous selection). The strip mirrors the
        // engine's candidate list in order, so `id` indexes it directly.
        chooseCandidate((size_t)id);
    } else {
        // Prediction strip (after a commit): there is no search/choose workspace
        // (it was reset on commit), so the tap must commit the shown suggestion
        // directly and chain to the next round of next-word predictions. Routing
        // it through choose() would no-op on the reset engine.
        commitAndPredict(displayed);
    }
}

/* Show `candidates` in the strip, each prefixed with the engine's already-fixed
 * composing text so the user sees the phrase being built (empty prefix during
 * plain typing -> the raw candidates). */
void ImeSelectionController::refreshCandidates(const std::vector<std::string>& candidates){
    if(mCandidates == nullptr) return;
    const std::string fixed = mIm ? mIm->fixedString() : std::string();
    std::vector<std::string> shown;
    shown.reserve(candidates.size());
    for(const auto& c : candidates) shown.push_back(fixed + c);
    mCandidates->setSuggestions(shown, true, true);
    LOGD("%d suggestions (fixed='%s')", (int)candidates.size(), fixed.c_str());
    mCandidates->invalidate(true);
}

/* Two-level selection: fix candidate[id] as the composing prefix. If the whole
 * pinyin is now fixed (the engine then returns exactly one candidate -- the
 * complete phrase), commit it and offer next-word predictions; otherwise
 * refresh the strip with the candidates for the still-unfixed tail. */
void ImeSelectionController::chooseCandidate(size_t id){
    if(mIm == nullptr) return;
    std::vector<std::string> candidates;
    const int rc = mIm->choose(id, candidates);
    if(rc < 0) return; // method has no in-composition selection (level-1)
    if(rc <= 1 && !candidates.empty()){
        // Fully fixed: candidates[0] is the whole phrase. Reset the composing
        // workspace before committing so a later prediction tap does not drive a
        // reset engine through choose().
        mIm->closeSearch();
        mComposing.clear();
        commitAndPredict(candidates[0]);
    } else {
        refreshCandidates(candidates);
    }
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
