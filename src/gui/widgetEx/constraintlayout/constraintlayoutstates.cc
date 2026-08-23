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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintLayoutStates.
 */
#include <widgetEx/constraintlayout/constraintlayoutstates.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/widgetex_styleable.h>

#include <core/xmlpullparser.h>

namespace cdroid {
using namespace cdroid::internal;

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
// id helpers
// ===========================================================================
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
    : mLayout(layout), mContext(ctx) {
    XmlPullParser parser(ctx, resourceId);
    parse(ctx, parser);
}

ConstraintLayoutStates::ConstraintLayoutStates(Context* ctx, ConstraintLayout* layout,
        XmlPullParser& parser)
    : mLayout(layout), mContext(ctx) {
    parse(ctx, parser);
}

int ConstraintLayoutStates::parseConstraintSet(Context* ctx, XmlPullParser& parser) {
    // <ConstraintSet android:id="@+id/cs1"> ...children... </ConstraintSet>
    // androidx ConstraintLayoutStates.parseConstraintSet scans parser attribute names for "id";
    // binary AXML stores the @+id ref as a typed value, so read it by index (name-based
    // getAttributeValue cannot decode a reference).
    int id = -1;
    const int acount = parser.getAttributeCount();
    for (int i = 0; i < acount; i++) {
        if (parser.getAttributeName(i) == "id") {
            id = parser.getAttributeResourceValue(i, -1);
            break;
        }
    }
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
                // androidx's StateSet case reads no attrs; defaultState is the CDROID extension.
                auto ta = ctx->obtainStyledAttributes(parser, R::styleable::StateSet);
                if (ta) {
                    mDefaultState = (int)ta->getResourceId(R::styleable::StateSet_defaultState,
                                                            mDefaultState);
                }
            } else if (tag == "State") {
                // androidx State ctor: TypedArray with android:id + constraints resource ids.
                State s;
                auto ta = ctx->obtainStyledAttributes(parser, R::styleable::State);
                if (ta) {
                    namespace ST = R::styleable;
                    s.mId           = (int)ta->getResourceId(ST::State_id, s.mId);
                    s.mConstraintID = (int)ta->getResourceId(ST::State_constraints, s.mConstraintID);
                }
                mStates.push_back(s);
                currentState = &mStates.back();
            } else if (tag == "Variant") {
                if (currentState != nullptr) {
                    // androidx Variant ctor: TypedArray getDimension on the region_* attrs.
                    Variant v;
                    auto ta = ctx->obtainStyledAttributes(parser, R::styleable::Variant);
                    if (ta) {
                        namespace VA = R::styleable;
                        v.mConstraintID = (int)ta->getResourceId(VA::Variant_constraints,
                                                                 v.mConstraintID);
                        v.mMinWidth  = ta->getDimension(VA::Variant_region_widthMoreThan,  v.mMinWidth);
                        v.mMaxWidth  = ta->getDimension(VA::Variant_region_widthLessThan,  v.mMaxWidth);
                        v.mMinHeight = ta->getDimension(VA::Variant_region_heightMoreThan, v.mMinHeight);
                        v.mMaxHeight = ta->getDimension(VA::Variant_region_heightLessThan, v.mMaxHeight);
                    }
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

ConstraintSet* ConstraintLayoutStates::resolveConstraintRef(int constraintId) {
    if (constraintId == -1) return nullptr;
    auto it = mConstraintSetMap.find(constraintId);
    if (it != mConstraintSetMap.end()) return it->second.get();
    // Not an inline set: a layout resource ref (androidx checks getResourceTypeName=="layout"
    // in the State/Variant ctors and clones there; CDROID resolves lazily after the parse).
    if (mContext != nullptr) {
        std::string type, entry;
        if (mContext->getResources().getResourceTypeName(constraintId, &type) && type == "layout"
                && mContext->getResources().getResourceEntryName(constraintId, &entry)) {
            auto set = std::make_shared<ConstraintSet>();
            set->clone(mContext, "layout/" + entry);
            ConstraintSet* raw = set.get();
            mClonedSets.push_back(std::move(set));
            return raw;
        }
    }
    return nullptr;
}

void ConstraintLayoutStates::resolveConstraintRefs() {
    // Wire each State/Variant `constraints` ref to its ConstraintSet (inline ref or layout-resource
    // clone). Done after the full parse so inline sets defined later in the file resolve too.
    for (auto& s : mStates) {
        s.mConstraintSet = resolveConstraintRef(s.mConstraintID);
        for (auto& v : s.mVariants) {
            v.mConstraintSet = resolveConstraintRef(v.mConstraintID);
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
