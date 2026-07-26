/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionScene.
 */
#include <widgetEx/constraintlayout/motionscene.h>

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
} // namespace

// ===========================================================================
// MotionScene::Transition
// ===========================================================================
MotionScene::Transition::Transition(MotionScene& scene, const AttributeSet& a)
    : mDuration(scene.mDefaultDuration) {
    mConstraintSetStart = scene.getId(a.getString("constraintSetStart", ""));
    mConstraintSetEnd   = scene.getId(a.getString("constraintSetEnd", ""));
    mDuration = a.getInt("duration", mDuration);
    if (mDuration < 8) mDuration = 8;
    mStagger = a.getFloat("staggered", mStagger);
    mDefaultInterpolatorString = a.getString("motionInterpolator", mDefaultInterpolatorString);
    mPathMotionArc = a.getInt("pathMotionArc", mPathMotionArc);
    if (mConstraintSetStart == UNSET) mIsAbstract = true;
}

// ===========================================================================
// MotionScene
// ===========================================================================
MotionScene::MotionScene(MotionLayout* layout)
    : mMotionLayout(layout) {}

MotionScene::MotionScene(Context* ctx, MotionLayout* layout, const std::string& resourceId)
    : mMotionLayout(layout) {
    load(ctx, resourceId);
}

std::string MotionScene::stripId(const std::string& idString) {
    // "@+id/start", "@id/start", "start" -> "start".
    std::string s = idString;
    if (s.empty()) return s;
    if (s[0] == '@') {
        const size_t slash = s.find('/');
        if (slash != std::string::npos) return s.substr(slash + 1);
        return s.substr(1);
    }
    return s;
}

int MotionScene::getId(const std::string& idString) const {
    if (idString.empty()) return UNSET;
    const std::string name = stripId(idString);
    auto it = mConstraintSetIdMap.find(name);
    if (it != mConstraintSetIdMap.end()) return it->second;
    const int id = mNextLocalId++;
    mConstraintSetIdMap[name] = id;
    return id;
}

int MotionScene::parseConstraintSet(Context* ctx, XmlPullParser& parser) {
    // <ConstraintSet android:id="@+id/start" deriveConstraintsFrom="@id/..."> ...children... </ConstraintSet>
    const std::string idStr = parser.getAttributeValue("id");
    const int id = getId(idStr);
    if (id == UNSET) return UNSET;
    auto set = std::make_unique<ConstraintSet>();
    set->load(ctx, parser); // consumes through </ConstraintSet>
    mConstraintSetMap[id] = std::move(set);
    return id;
}

ConstraintSet* MotionScene::getConstraintSet(int id) const {
    auto it = mConstraintSetMap.find(id);
    return (it != mConstraintSetMap.end()) ? it->second.get() : nullptr;
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
                currentTransition->setOnSwipe(std::move(os));
            }
            // OnSwipe / TouchResponse / StateSet / ViewTransition: deferred (swipe-driven / per-view).
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
