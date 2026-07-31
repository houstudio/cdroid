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
#include <widgetEx/constraintlayout/motion/motionscene.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/motion/viewtransition.h>
#include <widgetEx/constraintlayout/motion/viewtransitioncontroller.h>

#include <core/xmlpullparser.h>
#include <porting/cdlog.h>

namespace cdroid {

namespace {
// clickAction flag-name -> value map. The static constexpr flags are cast to int inline at the use
// site (a prvalue) to avoid odr-using them.
const std::unordered_map<std::string, int> kClickAction = {
    {"toggle",            (int)MotionScene::Transition::FLAG_TOGGLE},
    {"transitionToEnd",   (int)MotionScene::Transition::FLAG_TRANSITION_TO_END},
    {"transitionToStart", (int)MotionScene::Transition::FLAG_TRANSITION_TO_START},
    {"jumpToEnd",         (int)MotionScene::Transition::FLAG_JUMP_TO_END},
    {"jumpToStart",       (int)MotionScene::Transition::FLAG_JUMP_TO_START}
};
const std::unordered_map<std::string, int> kDragDirection = {
    {"dragUp",         (int)MotionScene::OnSwipe::DRAG_UP},
    {"dragDown",       (int)MotionScene::OnSwipe::DRAG_DOWN},
    {"dragLeft",       (int)MotionScene::OnSwipe::DRAG_LEFT},
    {"dragRight",      (int)MotionScene::OnSwipe::DRAG_RIGHT},
    {"dragStart",      (int)MotionScene::OnSwipe::DRAG_START},
    {"dragEnd",        (int)MotionScene::OnSwipe::DRAG_END}
};
const std::unordered_map<std::string, int> kTouchUp = {
    {"autoComplete",         (int)MotionScene::OnSwipe::ON_UP_AUTOCOMPLETE},
    {"autoCompleteToStart",  (int)MotionScene::OnSwipe::ON_UP_AUTOCOMPLETE_TO_START},
    {"autoCompleteToEnd",    (int)MotionScene::OnSwipe::ON_UP_AUTOCOMPLETE_TO_END},
    {"stop",                 (int)MotionScene::OnSwipe::ON_UP_STOP},
    {"decelerate",           (int)MotionScene::OnSwipe::ON_UP_DECELERATE}
};
const std::unordered_map<std::string, int> kAnchorSide = {
    {"top",    (int)MotionScene::OnSwipe::SIDE_TOP},
    {"left",   (int)MotionScene::OnSwipe::SIDE_LEFT},
    {"right",  (int)MotionScene::OnSwipe::SIDE_RIGHT},
    {"bottom", (int)MotionScene::OnSwipe::SIDE_BOTTOM},
    {"middle", (int)MotionScene::OnSwipe::SIDE_MIDDLE},
    {"start",  (int)MotionScene::OnSwipe::SIDE_START},
    {"end",    (int)MotionScene::OnSwipe::SIDE_END}
};
const std::unordered_map<std::string, int> kAutoComplete = {
    {"continuousVelocity", (int)MotionScene::OnSwipe::COMPLETE_CONTINUOUS_VELOCITY},
    {"spring",             (int)MotionScene::OnSwipe::COMPLETE_SPRING}
};
const std::unordered_map<std::string, int> kSpringBoundary = {
    {"overshoot",   (int)MotionScene::OnSwipe::SPRING_OVERSHOOT},
    {"bounceStart", (int)MotionScene::OnSwipe::SPRING_BOUNCE_START},
    {"bounceEnd",   (int)MotionScene::OnSwipe::SPRING_BOUNCE_END},
    {"bounceBoth",  (int)MotionScene::OnSwipe::SPRING_BOUNCE_BOTH}
};
const std::unordered_map<std::string, int> kAutoTransition = {
    {"none",           (int)MotionScene::Transition::AUTO_NONE},
    {"jumpToStart",    (int)MotionScene::Transition::AUTO_JUMP_TO_START},
    {"jumpToEnd",      (int)MotionScene::Transition::AUTO_JUMP_TO_END},
    {"animateToStart", (int)MotionScene::Transition::AUTO_ANIMATE_TO_START},
    {"animateToEnd",   (int)MotionScene::Transition::AUTO_ANIMATE_TO_END}
};
} // namespace

// ===========================================================================
// MotionScene::Transition
// ===========================================================================
MotionScene::Transition::Transition(MotionScene& scene, const AttributeSet& a)
    : mDuration(scene.mDefaultDuration) {
    mId = scene.getId(a.getString("id", "")); // <Transition android:id="@+id/...">
    mConstraintSetStart = scene.getId(a.getString("constraintSetStart", ""));
    mConstraintSetEnd   = scene.getId(a.getString("constraintSetEnd", ""));
    mDuration = a.getInt("duration", mDuration);
    if (mDuration < 8) mDuration = 8;
    mStagger = a.getFloat("staggered", mStagger);
    mDefaultInterpolatorString = a.getString("motionInterpolator", mDefaultInterpolatorString);
    mPathMotionArc = a.getInt("pathMotionArc", mPathMotionArc);
    mAutoTransition = a.getInt("autoTransition", kAutoTransition, mAutoTransition);
    if (mConstraintSetStart == UNSET) mIsAbstract = true;
}

// ===========================================================================
// MotionScene
// ===========================================================================
MotionScene::MotionScene(MotionLayout* layout)
    : mMotionLayout(layout)
    , mViewTransitionController(std::make_unique<ViewTransitionController>(layout)) {}

MotionScene::MotionScene(Context* ctx, MotionLayout* layout, const std::string& resourceId)
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
    const std::string name = stripId(idString);
    // AndroidX: MotionScene ids are R.id, shared with layout XML. Resolve via the host Context's
    // R.id pool first (the same pool AttributeSet::getResourceId uses), so ids referenced here agree
    // with ids referenced from layout XML — e.g. Carousel's carousel_nextState. Fall back to a
    // scene-local id only for names not registered in the R.id pool.
    if (mMotionLayout != nullptr) {
        Context* ctx = mMotionLayout->getContext();
        if (ctx != nullptr) {
            const int rid = ctx->getId(name);
            if (rid != -1) return rid;
        }
    }
    auto it = mConstraintSetIdMap.find(name);
    if (it != mConstraintSetIdMap.end()) return it->second;
    const int id = mNextLocalId++;
    mConstraintSetIdMap[name] = id;
    return id;
}

int MotionScene::parseConstraintSet(Context* ctx, XmlPullParser& parser) {
    // <ConstraintSet android:id="@+id/start" deriveConstraintsFrom="@id/..."> ...children... </ConstraintSet>
    // Read the element's own attributes before load() consumes the tag through its END_TAG.
    const std::string idStr = parser.getAttributeValue("id");
    const std::string deriveStr = parser.getAttributeValue("deriveConstraintsFrom");
    const int id = getId(idStr);
    if (id == UNSET) return UNSET;
    auto set = std::make_unique<ConstraintSet>();
    set->load(ctx, parser); // consumes through </ConstraintSet>
    mConstraintSetMap[id] = std::move(set);
    if (!deriveStr.empty()) {
        mDeriveFrom[id] = getId(deriveStr); // base merged lazily in getConstraintSet (order-independent)
    }
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

bool MotionScene::autoTransition(MotionLayout* layout, int currentState) {
    if (layout == nullptr) return false;
    for (const auto& t : mTransitionList) {
        const int mode = t->getAutoTransition();
        if (mode == Transition::AUTO_NONE) continue;
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

void MotionScene::load(Context* ctx, const std::string& resourceId) {
    XmlPullParser parser(ctx, resourceId);
    load(ctx, parser);
}

void MotionScene::load(Context* ctx, XmlPullParser& parser) {
    Transition* currentTransition = nullptr;

    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "MotionScene") {
                mDefaultDuration = parser.getInt("defaultDuration", mDefaultDuration);
            } else if (tag == "Transition") {
                auto t = std::make_unique<Transition>(*this, parser);
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
                oc.targetId = parser.getResourceId("targetId", UNSET);
                oc.clickAction = parser.getInt("clickAction", kClickAction, Transition::FLAG_TOGGLE);
                currentTransition->addOnClick(oc);
            } else if (tag == "OnSwipe" && currentTransition != nullptr) {
                auto os = std::make_unique<OnSwipe>();
                os->dragDirection = parser.getInt("dragDirection", kDragDirection, os->dragDirection);
                os->dragScale     = parser.getFloat("dragScale", os->dragScale);
                os->touchAnchorSide = parser.getInt("touchAnchorSide", kAnchorSide, os->touchAnchorSide);
                os->touchAnchorId  = parser.getResourceId("touchAnchorId", os->touchAnchorId);
                os->onTouchUp     = parser.getInt("onTouchUp", kTouchUp, os->onTouchUp);
                os->maxVelocity   = parser.getFloat("maxVelocity", os->maxVelocity);
                os->maxAcceleration = parser.getFloat("maxAcceleration", os->maxAcceleration);
                os->autoCompleteMode    = parser.getInt("autoCompleteMode", kAutoComplete, os->autoCompleteMode);
                os->springMass          = parser.getFloat("springMass", os->springMass);
                os->springStiffness     = parser.getFloat("springStiffness", os->springStiffness);
                os->springDamping       = parser.getFloat("springDamping", os->springDamping);
                os->springStopThreshold = parser.getFloat("springStopThreshold", os->springStopThreshold);
                os->springBoundary      = parser.getInt("springBoundary", kSpringBoundary, os->springBoundary);
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
