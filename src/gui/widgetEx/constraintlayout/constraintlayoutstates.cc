/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintLayoutStates.
 */
#include <widgetEx/constraintlayout/constraintlayoutstates.h>
#include <widgetEx/constraintlayout/constraintset.h>

#include <core/xmlpullparser.h>

namespace cdroid {

// ===========================================================================
// Variant / State (dimension matching)
// ===========================================================================
bool ConstraintLayoutStates::Variant::match(float widthDp, float heightDp) const {
    if (!std::isnan(mMinWidth)  && widthDp  < mMinWidth)  return false;
    if (!std::isnan(mMinHeight) && heightDp < mMinHeight) return false;
    if (!std::isnan(mMaxWidth)  && widthDp  > mMaxWidth)  return false;
    if (!std::isnan(mMaxHeight) && heightDp > mMaxHeight) return false;
    return true;
}

int ConstraintLayoutStates::State::findMatch(float widthDp, float heightDp) const {
    for (size_t i = 0; i < mVariants.size(); i++) {
        if (mVariants[i].match(widthDp, heightDp)) return (int) i;
    }
    return -1;
}

// ===========================================================================
// id helpers (mirror MotionScene: stable scene-local ints from "@+id/name")
// ===========================================================================
std::string ConstraintLayoutStates::stripId(const std::string& idString) {
    // "@+id/name", "@id/name", "name" -> "name". CDROID may encode a resource ref's leading '@' as
    // ':' in attribute values, so accept both markers.
    std::string s = idString;
    if (s.empty()) return s;
    if (s[0] == '@' || s[0] == ':') {
        const size_t slash = s.find('/');
        if (slash != std::string::npos) return s.substr(slash + 1);
        return s.substr(1);
    }
    return s;
}

int ConstraintLayoutStates::getId(const std::string& idString) const {
    if (idString.empty()) return -1;
    const std::string name = stripId(idString);
    auto it = mIdMap.find(name);
    if (it != mIdMap.end()) return it->second;
    const int id = mNextLocalId++;
    mIdMap[name] = id;
    return id;
}

ConstraintLayoutStates::State* ConstraintLayoutStates::findState(int id) {
    for (auto& s : mStates) if (s.mId == id) return &s;
    return nullptr;
}

const ConstraintLayoutStates::State* ConstraintLayoutStates::findState(int id) const {
    for (const auto& s : mStates) if (s.mId == id) return &s;
    return nullptr;
}

// ===========================================================================
// construction / parse
// ===========================================================================
ConstraintLayoutStates::ConstraintLayoutStates(Context* ctx, ConstraintLayout* layout,
                                               const std::string& resourceId)
    : mLayout(layout) {
    XmlPullParser parser(ctx, resourceId);
    parse(ctx, parser);
}

ConstraintLayoutStates::ConstraintLayoutStates(Context* ctx, ConstraintLayout* layout,
                                               XmlPullParser& parser)
    : mLayout(layout) {
    parse(ctx, parser);
}

int ConstraintLayoutStates::parseConstraintSet(Context* ctx, XmlPullParser& parser) {
    // <ConstraintSet android:id="@+id/cs1"> ...children... </ConstraintSet>
    const std::string idStr = parser.getAttributeValue("id");
    const int id = getId(idStr);
    if (id == -1) return -1;
    auto set = std::make_unique<ConstraintSet>();
    set->load(ctx, parser); // consumes through </ConstraintSet>
    mConstraintSetMap[id] = std::move(set);
    return id;
}

void ConstraintLayoutStates::parse(Context* ctx, XmlPullParser& parser) {
    State* currentState = nullptr;
    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
           parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "StateSet" || tag == "layoutDescription" || tag == "ConstraintLayoutStates") {
                mDefaultState = getId(parser.getAttributeValue("defaultState"));
            } else if (tag == "State") {
                State s;
                s.mId = getId(parser.getAttributeValue("id"));
                s.mConstraintID = getId(parser.getAttributeValue("constraints"));
                mStates.push_back(s);
                currentState = &mStates.back();
            } else if (tag == "Variant") {
                if (currentState != nullptr) {
                    Variant v;
                    v.mConstraintID = getId(parser.getAttributeValue("constraints"));
                    v.mMinWidth  = parser.getFloat("region_widthMoreThan",  v.mMinWidth);
                    v.mMaxWidth  = parser.getFloat("region_widthLessThan",  v.mMaxWidth);
                    v.mMinHeight = parser.getFloat("region_heightMoreThan", v.mMinHeight);
                    v.mMaxHeight = parser.getFloat("region_heightLessThan", v.mMaxHeight);
                    currentState->mVariants.push_back(v);
                }
            } else if (tag == "ConstraintSet") {
                parseConstraintSet(ctx, parser); // load() leaves parser at </ConstraintSet>
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            if (parser.getName() == "State") {
                currentState = nullptr;
            } else if (parser.getName() == "StateSet") {
                break;
            }
        }
        parser.next();
    }
    resolveConstraintRefs();
}

void ConstraintLayoutStates::resolveConstraintRefs() {
    // Wire each State/Variant `constraints` ref to its inline <ConstraintSet> (defined anywhere in
    // the file, so resolved after the full parse). Layout-resource refs stay nullptr (clone deferred).
    for (auto& s : mStates) {
        if (s.mConstraintID != -1) {
            auto it = mConstraintSetMap.find(s.mConstraintID);
            if (it != mConstraintSetMap.end()) s.mConstraintSet = it->second.get();
        }
        for (auto& v : s.mVariants) {
            if (v.mConstraintID != -1) {
                auto it = mConstraintSetMap.find(v.mConstraintID);
                if (it != mConstraintSetMap.end()) v.mConstraintSet = it->second.get();
            }
        }
    }
}

// ===========================================================================
// selection
// ===========================================================================
bool ConstraintLayoutStates::needsToChange(int id, float width, float height) const {
    if (mCurrentStateId != id) return true;
    const State* state = (id == -1) ? (mStates.empty() ? nullptr : &mStates[0])
                                    : findState(mCurrentStateId);
    if (state == nullptr) return false;
    if (mCurrentConstraintNumber != -1 &&
        mCurrentConstraintNumber < (int) state->mVariants.size()) {
        if (state->mVariants[mCurrentConstraintNumber].match(width, height)) return false;
    }
    if (mCurrentConstraintNumber == state->findMatch(width, height)) return false;
    return true;
}

ConstraintSet* ConstraintLayoutStates::convertToConstraintSet(int currentConstraintSetId,
        int stateId, float width, float height) const {
    const State* state = findState(stateId);
    if (state == nullptr) return nullptr;
    if (width < 0 || height < 0) { // dimension-independent: keep the current set if it is this state's
        if (state->mConstraintSet != nullptr && state->mConstraintID == currentConstraintSetId) {
            return state->mConstraintSet;
        }
        for (const auto& v : state->mVariants) {
            if (v.mConstraintSet != nullptr && v.mConstraintID == currentConstraintSetId) {
                return v.mConstraintSet;
            }
        }
        return state->mConstraintSet;
    }
    const Variant* match = nullptr;
    for (const auto& v : state->mVariants) {
        if (v.match(width, height)) {
            if (v.mConstraintSet != nullptr && v.mConstraintID == currentConstraintSetId) {
                return v.mConstraintSet; // current set still matches -> keep (no flap)
            }
            match = &v;
        }
    }
    if (match != nullptr) return match->mConstraintSet;
    return state->mConstraintSet; // state default
}

void ConstraintLayoutStates::updateConstraints(int id, float width, float height) {
    if (mCurrentStateId == id) {
        const State* state = (id == -1) ? (mStates.empty() ? nullptr : &mStates[0])
                                        : findState(mCurrentStateId);
        if (state == nullptr) return;
        if (mCurrentConstraintNumber != -1 &&
            mCurrentConstraintNumber < (int) state->mVariants.size()) {
            if (state->mVariants[mCurrentConstraintNumber].match(width, height)) return; // still fits
        }
        const int match = state->findMatch(width, height);
        if (mCurrentConstraintNumber == match) return;
        ConstraintSet* cs = (match == -1) ? state->mConstraintSet
                                          : state->mVariants[match].mConstraintSet;
        if (cs == nullptr) return;
        mCurrentConstraintNumber = match;
        cs->applyTo(mLayout);
    } else {
        mCurrentStateId = id;
        const State* state = findState(mCurrentStateId);
        const int match = (state == nullptr) ? -1 : state->findMatch(width, height);
        ConstraintSet* cs = (state == nullptr) ? nullptr
            : ((match == -1) ? state->mConstraintSet : state->mVariants[match].mConstraintSet);
        if (cs == nullptr) return;
        mCurrentConstraintNumber = match;
        cs->applyTo(mLayout);
    }
}

} // namespace cdroid
