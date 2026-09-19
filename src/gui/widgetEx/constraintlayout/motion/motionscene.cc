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
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionScene.
 */
#include <widget/internal_R.h>
#include <core/context.h>
#include <widgetEx/constraintlayout/motion/motionscene.h>
#include <widgetEx/widgetex_styleable.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/motion/touchresponse.h>
#include <widgetEx/constraintlayout/motion/viewtransition.h>
#include <widgetEx/constraintlayout/motion/viewtransitioncontroller.h>

#include <core/xmlpullparser.h>
#include <porting/cdlog.h>

namespace cdroid {
using namespace cdroid::internal;

// ===========================================================================
// MotionScene::Transition
// ===========================================================================
MotionScene::Transition::Transition(MotionScene& scene, Context* ctx, const AttributeSet& a)
    : mDuration(scene.mDefaultDuration) {
    // TypedArray reads typed binary AXML values directly (AOSP pattern); the default
    // arg covers an absent attr, so no name-based fallback is needed.
    auto ta = ctx->obtainStyledAttributes(a, R::styleable::Transition);
    if (ta) {
        namespace TR = R::styleable;
        mId = (int)ta->getResourceId(TR::Transition_id, UNSET);                     // <Transition android:id="@+id/...">
        // AndroidX Transition fillFromAttributeList (MotionScene.java:1104-1126): a
        // constraintSetStart/End reference is dispatched on its resource type — "@layout/x"
        // loads the ConstraintSet into the scene's map right here (without this the
        // transition never animates and nothing warns), "@xml/x" expands via parseInclude.
        auto constraintSetRef = [&](size_t attr, int& target) {
            target = (int)ta->getResourceId(attr, (uint32_t)target);
            if (target == UNSET) return;
            std::string type;
            if (!ctx->getResources().getResourceTypeName(target, &type)) return;
            if (type == "layout") {
                auto set = std::make_unique<ConstraintSet>();
                auto parser = ctx->getResources().getXml(target);
                set->load(ctx, *parser);
                scene.mConstraintSetMap[target] = std::move(set);
            }
            // "xml" (MotionScene.parseInclude) is not ported yet.
        };
        constraintSetRef(TR::Transition_constraintSetStart, mConstraintSetStart);
        constraintSetRef(TR::Transition_constraintSetEnd, mConstraintSetEnd);
        mDuration = ta->getInt(TR::Transition_duration, mDuration);
        if (mDuration < 8) mDuration = 8;
        mStagger = ta->getFloat(TR::Transition_staggered, mStagger);
        // AndroidX Transition_motionInterpolator (MotionScene.java:1126-1142): a TYPE_REFERENCE
        // (or path-ish string) resolves to an interpolator resource id — the old bare getString
        // turned "@anim/..." into an empty string and the transition silently kept the default.
        {
            TypedValue type;
            if (ta->peekValue(TR::Transition_motionInterpolator, &type)) {
                if (type.type == TypedValue::TYPE_REFERENCE) {
                    mDefaultInterpolatorID = (int)ta->getResourceId(
                            TR::Transition_motionInterpolator, (uint32_t)-1);
                    mDefaultInterpolatorString.clear();
                } else if (type.type == TypedValue::TYPE_STRING) {
                    mDefaultInterpolatorString = ta->getString(TR::Transition_motionInterpolator);
                    if (mDefaultInterpolatorString.find('/') != std::string::npos) {
                        mDefaultInterpolatorID = (int)ta->getResourceId(
                                TR::Transition_motionInterpolator, (uint32_t)-1);
                        mDefaultInterpolatorString.clear();
                    }
                }
            }
        }
        mPathMotionArc = ta->getInt(TR::Transition_pathMotionArc, mPathMotionArc);
        mAutoTransition = ta->getInt(TR::Transition_autoTransition, mAutoTransition);
        mTransitionFlags = ta->getInt(TR::Transition_transitionFlags, mTransitionFlags);
    }
    if (mConstraintSetStart == UNSET) mIsAbstract = true;
}

// ===========================================================================
// MotionScene
// ===========================================================================
MotionScene::MotionScene(MotionLayout* layout)
    : mMotionLayout(layout)
    , mViewTransitionController(std::make_unique<ViewTransitionController>(layout)) {}

MotionScene::MotionScene(Context* ctx, MotionLayout* layout, int resourceId)
    : mMotionLayout(layout)
    , mViewTransitionController(std::make_unique<ViewTransitionController>(layout)) {
    load(ctx, resourceId);
}

// Defined here so the unique_ptr<ViewTransition> member destroys with a complete type.
MotionScene::~MotionScene() = default;

ViewTransition* MotionScene::getViewTransitionById(int id) const {
    for (const auto& vt : mViewTransitions) {
        if (vt->getId() == id) return vt.get();
    }
    return nullptr;
}

std::vector<int> MotionScene::getConstraintSetIds() const {
    std::vector<int> ids;
    ids.reserve(mConstraintSetMap.size());
    for (const auto& kv : mConstraintSetMap) ids.push_back(kv.first);
    return ids;
}

void MotionScene::viewTransition(int id, const std::vector<View*>& views) {
    if (mViewTransitionController) mViewTransitionController->viewTransition(id, views);
}

void MotionScene::enableViewTransition(int id, bool enable) {
    if (mViewTransitionController) mViewTransitionController->enableViewTransition(id, enable);
}

bool MotionScene::isViewTransitionEnabled(int id) const {
    return mViewTransitionController && mViewTransitionController->isViewTransitionEnabled(id);
}

bool MotionScene::applyViewTransition(int id, Motion* mc) {
    return mViewTransitionController && mViewTransitionController->applyViewTransition(id, mc);
}

std::string MotionScene::stripId(const std::string& idString) {
    // "@+id/start", "@id/start" is error, "start" -> "start".
    std::string s = idString;
    if (s.empty()) return s;
    const size_t slash = s.find('/');
    if (slash != std::string::npos) return s.substr(slash + 1);
    return s;
}

int MotionScene::getId(const std::string& idString) const {
    if (idString.empty()) return UNSET;
    // stripId -> bare name (scene-local cache key). Resolve as a real resource id via the arsc
    // (Resources.getIdentifier); when unregistered it returns 0, so scene-only ids fall through
    // to the allocator below. NB: must check != 0, not -1 — getIdentifier's not-found is 0, and
    // 0 == PARENT_ID, so returning it would collapse every scene-only ConstraintSet id onto one key.
    const std::string name = stripId(idString);
    if (mMotionLayout != nullptr) {
        Context* ctx = mMotionLayout->getContext();
        if (ctx != nullptr) {
            const int rid = ctx->getResources().getIdentifier(name, "id", "");
            if (rid != 0) return rid;
        }
    }
    auto it = mConstraintSetIdMap.find(name);
    if (it != mConstraintSetIdMap.end()) return it->second;
    const int id = mNextLocalId++;
    mConstraintSetIdMap[name] = id;
    return id;
}

int MotionScene::parseConstraintSet(Context* ctx, XmlPullParser& parser) {
    // androidx scans (getAttributeName(i), getAttributeValue(i)) and decodes the
    // id strings by hand (getId / stripID; stateLabels/constraintRotate unported
    // — ConstraintSet has neither feature). CDROID's string render drops the
    // package and getPackageName() may be a path, so the re-resolution cannot
    // work; the equivalent is the typed index read, which serves both parsers
    // (binary: the aapt2 resId — the SAME int the Transition's
    // constraintSetStart/End resolve to, so the map keys match; text: the
    // by-name reference resolver). stripID still keys the by-name map.
    auto set = std::make_unique<ConstraintSet>();
    const int acount = parser.getAttributeCount();
    int id = UNSET, derivedId = UNSET;
    for (int i = 0; i < acount; i++) {
        const std::string name = parser.getAttributeName(i);
        if      (name == "id") {
            id = parser.getAttributeResourceValue(i, UNSET);
            mConstraintSetIdMap[stripId(parser.getAttributeValue(i))] = id;
        }
        else if (name == "deriveConstraintsFrom") derivedId = parser.getAttributeResourceValue(i, UNSET);
    }
    if (id == UNSET) return UNSET;
    set->load(ctx, parser); // consumes through </ConstraintSet>
    mConstraintSetMap[id] = std::move(set);
    if (derivedId != UNSET) mDeriveFrom[id] = derivedId; // base merged lazily in getConstraintSet
    return id;
}

ConstraintSet* MotionScene::getConstraintSet(int id) const {
    auto it = mConstraintSetMap.find(id);
    if (it == mConstraintSetMap.end()) return nullptr;
    // Lazy merge of deriveConstraintsFrom: base's constraints are copied in (derived wins). Done on
    // first access so the base may be defined after the derived set in the XML. Erase before the
    // recursive call so a cycle (A derives B, B derives A) terminates.
    auto dit = mDeriveFrom.find(id);
    if (dit != mDeriveFrom.end()) {
        const int baseId = dit->second;
        mDeriveFrom.erase(dit);
        if (ConstraintSet* base = getConstraintSet(baseId)) {
            it->second->mergeFrom(*base);
        }
    }
    return it->second.get();
}

MotionScene::Transition* MotionScene::getTransitionById(int id) const {
    for (const auto& t : mTransitionList) {
        if (t->getId() == id) return t.get();
    }
    return nullptr;
}

MotionScene::Transition* MotionScene::findTransition(int startId, int endId) const {
    for (const auto& t : mTransitionList) {
        if (t->getStartId() == startId && t->getEndId() == endId) return t.get();
    }
    return nullptr;
}

std::vector<MotionScene::Transition*> MotionScene::getDefinedTransitions() const {
    std::vector<Transition*> out;
    out.reserve(mTransitionList.size());
    for (const auto& t : mTransitionList) out.push_back(t.get());
    return out;
}

std::vector<MotionScene::Transition*> MotionScene::getTransitionsWithState(int stateId) const {
    std::vector<Transition*> out;
    for (const auto& t : mTransitionList) {
        if (t->getStartId() == stateId || t->getEndId() == stateId) out.push_back(t.get());
    }
    return out;
}

MotionScene::Transition* MotionScene::bestTransitionFor(int currentState, float dx, float dy) const {
    // Mirror androidx MotionScene.bestTransitionFor: among enabled transitions touching the current
    // state, score each by how much its <OnSwipe> drag direction aligns with the gesture (dot product),
    // flip the score for transitions we'd run backwards (their end == current state), and slightly
    // bias toward start-over-end. The max wins.
    Transition* best = nullptr;
    float max = 0;
    for (Transition* t : getTransitionsWithState(currentState)) {
        if (!t->isEnabled()) continue;
        const OnSwipe* sw = t->getOnSwipe();
        if (sw == nullptr) continue;
        float dirX, dirY;
        TouchResponse::directionVector(sw->dragDirection, dirX, dirY);
        float val = dirX * dx + dirY * dy;
        if (t->getEndId() == currentState) val *= -1;   // running this transition backwards
        else                               val *= 1.1f;  // prefer the transition whose start == state
        if (val > max) {
            max = val;
            best = t;
        }
    }
    return best;
}

bool MotionScene::autoTransition(MotionLayout* layout, int currentState) {
    // AndroidX MotionScene.autoTransition (MotionScene.java:441-451): no auto firing while a
    // touch sequence is being processed (its velocity tracker is alive) or when disabled.
    if (layout == nullptr) return false;
    if (layout->isProcessingTouch()) return false;
    if (mDisableAutoTransition) return false;
    for (const auto& t : mTransitionList) {
        const int mode = t->getAutoTransition();
        if (mode == Transition::AUTO_NONE) continue;
        // A transition flagged intraAuto must not feed itself while current (java:453-456).
        if (mCurrentTransition == t.get()
                && t->isTransitionFlag(Transition::TRANSITION_FLAG_INTRA_AUTO)) {
            continue;
        }
        if (currentState == t->getStartId()
                && (mode == Transition::AUTO_ANIMATE_TO_END || mode == Transition::AUTO_JUMP_TO_END)) {
            layout->applyTransitionForAuto(t.get(), /*toEnd=*/true,
                                           /*jump=*/(mode == Transition::AUTO_JUMP_TO_END));
            return true;
        }
        if (currentState == t->getEndId()
                && (mode == Transition::AUTO_ANIMATE_TO_START || mode == Transition::AUTO_JUMP_TO_START)) {
            layout->applyTransitionForAuto(t.get(), /*toEnd=*/false,
                                           /*jump=*/(mode == Transition::AUTO_JUMP_TO_START));
            return true;
        }
    }
    return false;
}

void MotionScene::load(Context* ctx, int resourceId) {
    auto parser = ctx->getResources().getXml(resourceId);
    load(ctx, *parser);
}

void MotionScene::load(Context* ctx, XmlPullParser& parser) {
    Transition* currentTransition = nullptr;

    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "MotionScene") {
                mDefaultDuration = parser.getAttributeIntValue(std::string(), "defaultDuration", mDefaultDuration);
            } else if (tag == "Transition") {
                auto t = std::make_unique<Transition>(*this, ctx, parser);
                Transition* raw = t.get();
                mTransitionList.push_back(std::move(t));
                currentTransition = raw;
                if (mCurrentTransition == nullptr && !raw->isAbstract()) {
                    mCurrentTransition = raw;
                }
            } else if (tag == "ConstraintSet") {
                parseConstraintSet(ctx, parser);
            } else if (tag == "KeyFrameSet" && currentTransition != nullptr) {
                currentTransition->setKeyFrames(std::make_unique<KeyFrames>(ctx, parser));
            } else if (tag == "OnClick" && currentTransition != nullptr) {
                OnClick oc;
                // TypedArray: binary AXML stores targetId (ref) / clickAction (flags)
                // as typed values the name-based read cannot decode.
                auto ta = ctx->obtainStyledAttributes(parser, R::styleable::OnClick);
                if (ta) {
                    oc.targetId = (int)ta->getResourceId(R::styleable::OnClick_targetId, UNSET);
                    oc.clickAction = ta->getInt(R::styleable::OnClick_clickAction, Transition::FLAG_TOGGLE);
                }
                currentTransition->addOnClick(oc);
            } else if (tag == "OnSwipe" && currentTransition != nullptr) {
                auto os = std::make_unique<OnSwipe>();
                // Binary AXML stores enums/floats/refs as typed Res_values; read them via
                // TypedArray (AOSP MotionScene pattern). getInt/getFloat/getResourceId return
                // the passed default when the attr is absent — no name-based fallback needed.
                auto ta = ctx->obtainStyledAttributes(parser, R::styleable::OnSwipe);
                if (ta) {
                    namespace SW = R::styleable;
                    os->dragDirection    = ta->getInt(SW::OnSwipe_dragDirection, os->dragDirection);
                    os->dragScale        = ta->getFloat(SW::OnSwipe_dragScale, os->dragScale);
                    os->touchAnchorSide  = ta->getInt(SW::OnSwipe_touchAnchorSide, os->touchAnchorSide);
                    os->touchAnchorId    = (int)ta->getResourceId(SW::OnSwipe_touchAnchorId, os->touchAnchorId);
                    os->onTouchUp        = ta->getInt(SW::OnSwipe_onTouchUp, os->onTouchUp);
                    os->maxVelocity      = ta->getFloat(SW::OnSwipe_maxVelocity, os->maxVelocity);
                    os->maxAcceleration  = ta->getFloat(SW::OnSwipe_maxAcceleration, os->maxAcceleration);
                    os->autoCompleteMode = ta->getInt(SW::OnSwipe_autoCompleteMode, os->autoCompleteMode);
                    os->springMass          = ta->getFloat(SW::OnSwipe_springMass, os->springMass);
                    os->springStiffness     = ta->getFloat(SW::OnSwipe_springStiffness, os->springStiffness);
                    os->springDamping       = ta->getFloat(SW::OnSwipe_springDamping, os->springDamping);
                    os->springStopThreshold = ta->getFloat(SW::OnSwipe_springStopThreshold, os->springStopThreshold);
                    os->springBoundary   = ta->getInt(SW::OnSwipe_springBoundary, os->springBoundary);
                }
                currentTransition->setOnSwipe(std::move(os));
            } else if (tag == "ViewTransition") {
                auto vt = std::make_unique<ViewTransition>(*this, ctx, parser);
                ViewTransition* raw = vt.get();
                mViewTransitions.push_back(std::move(vt));
                if (mViewTransitionController) mViewTransitionController->add(raw);
            }
            // <OnSwipe>/<TouchResponse> and <StateSet> are parsed by their own handlers
            // (TouchResponse is wired to MotionLayout's swipe; StateSet drives ConstraintLayoutStates).
        } else if (eventType == XmlPullParser::END_TAG) {
            if (parser.getName() == "Transition") {
                currentTransition = nullptr;
            } else if (parser.getName() == "MotionScene") {
                return;
            }
        }
        parser.next();
    }
}

} // namespace cdroid
