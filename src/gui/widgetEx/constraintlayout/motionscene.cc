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
// The static constexpr click-action flags are cast to int once (a prvalue) to avoid odr-using them.
const int FLAG_TOGGLE_             = (int)MotionScene::Transition::FLAG_TOGGLE;
const int FLAG_TRANSITION_TO_END_  = (int)MotionScene::Transition::FLAG_TRANSITION_TO_END;
const int FLAG_TRANSITION_TO_START_= (int)MotionScene::Transition::FLAG_TRANSITION_TO_START;
const int FLAG_JUMP_TO_END_        = (int)MotionScene::Transition::FLAG_JUMP_TO_END;
const int FLAG_JUMP_TO_START_      = (int)MotionScene::Transition::FLAG_JUMP_TO_START;

const std::unordered_map<std::string, int> kClickAction = {
    {"toggle",            FLAG_TOGGLE_},
    {"transitionToEnd",   FLAG_TRANSITION_TO_END_},
    {"transitionToStart", FLAG_TRANSITION_TO_START_},
    {"jumpToEnd",         FLAG_JUMP_TO_END_},
    {"jumpToStart",       FLAG_JUMP_TO_START_}
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

int MotionScene::getId(const std::string& idString) {
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
