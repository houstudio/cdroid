/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionScene.
 *
 * Parses a <MotionScene> XML resource (referenced by MotionLayout's layoutDescription). A scene
 * holds one or more <Transition>s (each naming a start/end ConstraintSet, a duration, optional
 * <KeyFrameSet> and <OnClick> handlers) plus the <ConstraintSet> definitions they reference.
 * MotionLayout builds a MotionScene from XML, then drives its current transition: captures the
 * start/end ConstraintSets, applies the KeyFrames to each child Motion, and wires OnClick targets.
 *
 * MVP scope: <MotionScene>/<Transition>/<ConstraintSet>(inline or id-ref)/<KeyFrameSet>/<OnClick>.
 * Deferred (faithful stubs): <OnSwipe>/<TouchResponse> (swipe-driven), <StateSet>, <ViewTransition>.
 *
 * ConstraintSet ids declared in the scene (e.g. "@+id/start") are resolved scene-locally by name
 * (getId assigns a stable int per name) — they are not R.id constants, so resolution does not
 * depend on idgen.py scanning the xml/ folder.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_SCENE_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_SCENE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/keyframes.h>

namespace cdroid {

class Context;
class MotionLayout;
class XmlPullParser;

class MotionScene {
public:
    static constexpr int UNSET = -1;

    // A click handler: when `targetId` is clicked, perform `clickAction` on the transition.
    struct OnClick {
        int targetId = UNSET;
        int clickAction = 0; // flag: toggle / transitionToEnd / transitionToStart / jumpToEnd / jumpToStart
    };

    // A swipe handler (<OnSwipe>): dragging in `dragDirection` drives the transition progress, with
    // auto-complete (snap to an end) on release. (Java: motion.widget.OnSwipe + TouchResponse.)
    // MVP: linear drag-to-progress over the layout's own dimension + snap-to-nearest on touch-up.
    // Deferred: anchor geometry, spring physics, velocity-based completion, nested scroll.
    struct OnSwipe {
        static constexpr int DRAG_UP = 0, DRAG_DOWN = 1, DRAG_LEFT = 2, DRAG_RIGHT = 3,
                             DRAG_START = 4, DRAG_END = 5;
        static constexpr int SIDE_TOP = 0, SIDE_LEFT = 1, SIDE_RIGHT = 2, SIDE_BOTTOM = 3,
                             SIDE_MIDDLE = 4, SIDE_START = 5, SIDE_END = 6;
        static constexpr int ON_UP_AUTOCOMPLETE = 0, ON_UP_AUTOCOMPLETE_TO_START = 1,
                             ON_UP_AUTOCOMPLETE_TO_END = 2, ON_UP_STOP = 3, ON_UP_DECELERATE = 4;

        int   dragDirection  = DRAG_RIGHT;   // dragging this way increases progress
        float dragScale      = 1.0f;         // progress-per-pixel multiplier
        int   touchAnchorSide= SIDE_MIDDLE;  // which point on the anchor is the drag reference
        int   touchAnchorId  = UNSET;        // view whose start->end travel is the drag range
        int   onTouchUp      = ON_UP_AUTOCOMPLETE;
        float maxVelocity    = 4.0f;         // (MVP: unused — snap is progress-based, not velocity)
    };

    // One transition between two ConstraintSets. (Java: MotionScene.Transition.)
    class Transition {
    public:
        // clickAction flag values (match attrs.xml OnClick clickAction flags).
        static constexpr int FLAG_TOGGLE             = 0x0011;
        static constexpr int FLAG_TRANSITION_TO_END  = 0x0001;
        static constexpr int FLAG_TRANSITION_TO_START= 0x0010;
        static constexpr int FLAG_JUMP_TO_END        = 0x0100;
        static constexpr int FLAG_JUMP_TO_START      = 0x1000;

        // Read the <Transition> element's own attributes from `attrs` (the parser is at the
        // START_TAG). Child elements (<KeyFrameSet>/<OnClick>) are handled by MotionScene::load
        // via setKeyFrames()/addOnClick().
        Transition(MotionScene& scene, const AttributeSet& attrs);

        int getDuration() const { return mDuration; }
        float getStagger() const { return mStagger; }
        int getId() const { return mId; }
        int getStartId() const { return mConstraintSetStart; }
        int getEndId() const { return mConstraintSetEnd; }
        const std::string& getInterpolatorString() const { return mDefaultInterpolatorString; }
        int getPathMotionArc() const { return mPathMotionArc; }
        bool isAbstract() const { return mIsAbstract; }
        KeyFrames* getKeyFrames() const { return mKeyFrames.get(); }
        const std::vector<OnClick>& getOnClicks() const { return mOnClicks; }
        const OnSwipe* getOnSwipe() const { return mOnSwipe.get(); }

        void setKeyFrames(std::unique_ptr<KeyFrames> kf) { mKeyFrames = std::move(kf); }
        void addOnClick(OnClick oc) { mOnClicks.push_back(oc); }
        void setOnSwipe(std::unique_ptr<OnSwipe> os) { mOnSwipe = std::move(os); }

    private:
        int mId = UNSET;
        int mConstraintSetStart = UNSET;
        int mConstraintSetEnd = UNSET;
        int mDuration = 400;
        float mStagger = 0;
        std::string mDefaultInterpolatorString;
        int mPathMotionArc = UNSET;
        bool mIsAbstract = false;
        std::unique_ptr<KeyFrames> mKeyFrames;
        std::vector<OnClick> mOnClicks;
        std::unique_ptr<OnSwipe> mOnSwipe;
    };

    MotionScene(MotionLayout* layout);
    // Load and parse the <MotionScene> at resource `resourceId` (e.g. "xml/my_scene" or "@xml/...").
    MotionScene(Context* ctx, MotionLayout* layout, const std::string& resourceId);

    void load(Context* ctx, const std::string& resourceId);
    // Parse a <MotionScene> from an already-constructed pull parser (positioned anywhere before the
    // <MotionScene> START_TAG). Test-friendly: lets a unit test feed a stringstream-backed parser.
    void load(Context* ctx, XmlPullParser& parser);

    // The first non-abstract transition (the active one), or nullptr.
    Transition* getCurrentTransition() const { return mCurrentTransition; }
    Transition* getTransitionById(int id) const;              // by <Transition android:id>
    Transition* findTransition(int startId, int endId) const;  // matching both endpoints
    void setCurrentTransition(Transition* t) { mCurrentTransition = t; }
    // ConstraintSet registered under `id` (from a <ConstraintSet>), or nullptr.
    ConstraintSet* getConstraintSet(int id) const;

private:
    // Resolve a "@id/name" / "@+id/name" / "name" reference to a stable scene-local int, assigning
    // one lazily per name (so <ConstraintSet id> and <Transition constraintSet*> agree). Logically a
    // query; the name->id cache is mutable so this can be const.
    int getId(const std::string& idString) const;
    static std::string stripId(const std::string& idString); // "@+id/start" -> "start"
    // Parse a <ConstraintSet> element (id + ConstraintSet.load). `parser` at the START_TAG.
    int parseConstraintSet(Context* ctx, XmlPullParser& parser);

    MotionLayout* mMotionLayout;
    int mDefaultDuration = 400;
    std::vector<std::unique_ptr<Transition>> mTransitionList;
    Transition* mCurrentTransition = nullptr;
    std::unordered_map<int, std::unique_ptr<ConstraintSet>> mConstraintSetMap; // id -> set
    mutable std::unordered_map<std::string, int> mConstraintSetIdMap;          // name -> id (lazy cache)
    mutable int mNextLocalId = 0x10000; // base for scene-local ConstraintSet ids (avoids R.id collision)
    mutable std::unordered_map<int,int> mDeriveFrom; // deriveConstraintsFrom: id -> baseId (lazy merge)
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_SCENE_H
