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
 *
 * Adaptive layouts for ConstraintLayout: a <StateSet> maps (state id, layout dimensions) to a
 * ConstraintSet, so the layout can swap its constraints when its size or a logical state changes
 * (responsive/adaptive design). Each <State> holds a default ConstraintSet plus size-banded
 * <Variant>s; the Variant whose width/height region contains the current dimensions wins. The
 * `constraints` attribute references a ConstraintSet — inline <ConstraintSet> in the same file
 * (resolved here) or a layout resource (clone deferred to a later chunk).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_STATES_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_STATES_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cdroid {

class ConstraintLayout;
class ConstraintSet;
class Context;
class XmlPullParser;

class ConstraintLayoutStates {
public:
    // A size-banded entry: applies when the layout's dimensions fall in [min,max] (NaN = unbounded).
    struct Variant {
        float mMinWidth  = NAN;
        float mMinHeight = NAN;
        float mMaxWidth  = NAN;
        float mMaxHeight = NAN;
        int mConstraintID = -1;
        ConstraintSet* mConstraintSet = nullptr; // borrowed (owned by mConstraintSetMap)
        bool match(float widthDp, float heightDp) const;
    };

    // One logical state: a default ConstraintSet plus ordered Variants (first match wins).
    struct State {
        int mId = -1;
        std::vector<Variant> mVariants;
        int mConstraintID = -1;
        ConstraintSet* mConstraintSet = nullptr; // default (borrowed)
        // Index of the first matching Variant, or -1 if none.
        int findMatch(float widthDp, float heightDp) const;
    };

    // Load from a resource path (e.g. "xml/states" or "@xml/states").
    ConstraintLayoutStates(Context* ctx, ConstraintLayout* layout, const std::string& resourceId);
    // Load directly from a parser (for tests / pre-opened streams).
    ConstraintLayoutStates(Context* ctx, ConstraintLayout* layout, XmlPullParser& parser);

    // True if the layout should switch ConstraintSets for (id, width, height).
    bool needsToChange(int id, float width, float height) const;

    // Select the ConstraintSet for (stateId, width, height). If the currently-applied set
    // (currentConstraintSetId) still matches the dimensions, it is kept (avoids flapping). Returns
    // nullptr if the state is unknown. Pass width/height < 0 to skip dimension matching.
    ConstraintSet* convertToConstraintSet(int currentConstraintSetId, int stateId,
                                          float width, float height) const;

    int getDefaultState() const { return mDefaultState; }
    int getCurrentStateId() const { return mCurrentStateId; }

    // Select the ConstraintSet for (id, width, height) and apply it to the bound layout. Tracks the
    // current state/constraint so a no-op (same state, dimensions still matching) skips applyTo.
    void updateConstraints(int id, float width, float height);

    // Stable scene-local id for a name (e.g. "@+id/s1" / "s1" -> int). Mirrors MotionScene.
    int getId(const std::string& idString) const;
    static std::string stripId(const std::string& idString);

private:
    void parse(Context* ctx, XmlPullParser& parser);
    int parseConstraintSet(Context* ctx, XmlPullParser& parser); // inline <ConstraintSet> -> map
    // After all elements are parsed, wire each State/Variant `constraints` ref to its ConstraintSet.
    void resolveConstraintRefs();

    State* findState(int id);
    const State* findState(int id) const;

    ConstraintLayout* mLayout = nullptr;
    int mDefaultState = -1;
    int mCurrentStateId = -1;
    int mCurrentConstraintNumber = -1;
    std::vector<State> mStates;
    std::unordered_map<int, std::shared_ptr<ConstraintSet>> mConstraintSetMap; // inline sets (owned)
    mutable std::unordered_map<std::string, int> mIdMap;
    mutable int mNextLocalId = 0x10000; // base for scene-local ids (avoids R.id collision)
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_STATES_H
