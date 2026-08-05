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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_TRANSITION_H__
#define __CDROID_TRANSITION_TRANSITION_H__

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include <core/callbackbase.h>   // EventSet, CallbackBase (TransitionListener value type)
#include <core/rect.h>       // Rect (getEpicenter return type)

#include <transition/arraymap.h>                 // ArrayMap
#include <transition/pathmotion.h>               // PathMotion
#include <transition/transitionpropagation.h>    // TransitionPropagation
#include <transition/transitionvalues.h>         // TransitionValues, TransitionValuesPtr
#include <transition/transitionvaluesmaps.h>     // TransitionValuesMaps

namespace cdroid {

class Animator;
class Context;
class AttributeSet;
class TimeInterpolator;
class View;
class ViewGroup;
class ListView;

/**
 * A Transition holds information about animations that will be run on its targets
 * during a scene change. Ported from android-36 android.transition.Transition.
 *
 * A Transition has two main jobs: (1) capture property values, and (2) play
 * animations based on changes to captured property values. This abstract base
 * implements the target matching, capture, lifecycle and overlap-cancellation
 * machinery; subclasses implement captureStartValues/captureEndValues and
 * createAnimator.
 *
 * API-mapping notes (deviations forced by CDROID's C++/cairo substrate):
 *  - WindowId: android.view.WindowId is replaced by void* (View::getWindowId()
 *    returns a stable per-window pointer). Equality is pointer identity.
 *  - java.lang.Class (targetType) is represented by std::type_index; isInstance
 *    is an exact typeid(*view) match (does NOT include subclasses — see #isValidTarget).
 *  - sRunningAnimators is a ThreadLocal<ArrayMap> in android; CDROID runs all
 *    transitions on the UI thread, so it is a process-local static ArrayMap.
 *  - Nullable java ArrayLists that exist only as an allocation optimization are
 *    represented by empty std::vectors (empty == null behaviorally).
 */
class Transition {
  public:
    /** With setMatchOrder, match by View instance. */
    static constexpr int MATCH_INSTANCE = 0x1;
    static constexpr int MATCH_FIRST    = MATCH_INSTANCE;
    /** With setMatchOrder, match by View#getTransitionName(). Empty names are not matched. */
    static constexpr int MATCH_NAME   = 0x2;
    /** With setMatchOrder, match by View#getId(). Negative ids are not matched. */
    static constexpr int MATCH_ID     = 0x3;
    /** With setMatchOrder, match by Adapter item id. Requires hasStableIds(). */
    static constexpr int MATCH_ITEM_ID = 0x4;
    static constexpr int MATCH_LAST    = MATCH_ITEM_ID;

    /**
     * A transition listener receives lifecycle notifications from a transition.
     * Ported from android.transition.Transition.TransitionListener, but using the
     * codebase's EventSet + CallbackBase value semantics (same shape as
     * Animator::AnimatorListener): addListener/removeListener take it by value, so
     * the Transition owns its listeners (no manual new/delete, no leak). Each
     * CallbackBase member is a no-op until assigned. Identity for removeListener is
     * EventSet's shared mID (a copy compares equal to its original).
     */
    class TransitionListener : public EventSet {
      public:
        CallbackBase<void, Transition&> onTransitionStart;
        CallbackBase<void, Transition&> onTransitionEnd;
        CallbackBase<void, Transition&> onTransitionCancel;
        CallbackBase<void, Transition&> onTransitionPause;
        CallbackBase<void, Transition&> onTransitionResume;
    };

    /**
     * Class to get the epicenter of a Transition. Ported from
     * android.transition.Transition.EpicenterCallback.
     */
    class EpicenterCallback {
      public:
        virtual ~EpicenterCallback() = default;
        virtual Rect onGetEpicenter(Transition& transition) = 0;
    };

    /**
     * Holds information about each animator used when a new transition starts while
     * other transitions are still running, to decide whether a running animation
     * should be canceled or a new animation noop'd. @hide-equivalent internal struct.
     */
    struct AnimationInfo {
        View* view = nullptr;
        std::string name;
        TransitionValuesPtr values; // nullable: empty shared_ptr == null
        void* windowId = nullptr;   // android.view.WindowId -> void*
        Transition* transition = nullptr;
        AnimationInfo() = default;
        AnimationInfo(View* view, const std::string& name, Transition* transition,
                      void* windowId, const TransitionValuesPtr& values);
    };

    Transition();
    Transition(Context* context, AttributeSet* attrs);
    virtual ~Transition();

    // ---- duration / delay / interpolator ----
    virtual Transition& setDuration(int64_t duration);
    virtual int64_t getDuration() const;
    virtual Transition& setStartDelay(int64_t startDelay);
    virtual int64_t getStartDelay() const;
    virtual Transition& setInterpolator(const TimeInterpolator* interpolator);
    virtual const TimeInterpolator* getInterpolator() const;

    /** Property names this transition captures (for overlap cancellation). Default: null/empty. */
    virtual std::vector<std::string> getTransitionProperties();

    /** Create the animation for one target's start/end values. Default returns null. */
    virtual Animator* createAnimator(ViewGroup* sceneRoot,
                                     TransitionValues* startValues, TransitionValues* endValues);

    // ---- match order ----
    void setMatchOrder(const std::vector<int>& matches);

    // ---- capture (subclass contract) ----
    virtual void captureStartValues(TransitionValues& transitionValues) = 0;
    virtual void captureEndValues(TransitionValues& transitionValues) = 0;

    // ---- targets ----
    virtual Transition& addTarget(int targetId);
    virtual Transition& addTarget(const std::string& targetName);
    virtual Transition& addTarget(const std::type_index& targetType);
    virtual Transition& addTarget(View* target);
    virtual Transition& removeTarget(int targetId);
    virtual Transition& removeTarget(const std::string& targetName);
    virtual Transition& removeTarget(const std::type_index& targetType);
    virtual Transition& removeTarget(View* target);

    virtual Transition& excludeTarget(int targetId, bool exclude);
    virtual Transition& excludeTarget(const std::string& targetName, bool exclude);
    virtual Transition& excludeTarget(const std::type_index& targetType, bool exclude);
    virtual Transition& excludeTarget(View* target, bool exclude);
    Transition& excludeChildren(int targetId, bool exclude);
    Transition& excludeChildren(const std::type_index& targetType, bool exclude);
    Transition& excludeChildren(View* target, bool exclude);

    const std::vector<int>&          getTargetIds() const;
    const std::vector<View*>&        getTargets() const;
    const std::vector<std::string>&  getTargetNames() const;
    const std::vector<std::type_index>& getTargetTypes() const;
    const std::vector<std::string>&  getTargetViewNames() const; // deprecated alias

    bool isValidTarget(View* target) const;

    // ---- capture / matching (package-private in android; public here for the engine) ----
    void captureValues(ViewGroup* sceneRoot, bool start);
    static void addViewValues(TransitionValuesMaps& transitionValuesMaps,
                              View* view, const TransitionValuesPtr& transitionValues);
    void clearValues(bool start);
    TransitionValues* getTransitionValues(View* view, bool start);
    TransitionValues* getMatchedTransitionValues(View* view, bool viewInStart);

    /** Whether an Animator should be created given start/end values. */
    virtual bool isTransitionRequired(TransitionValues* startValues, TransitionValues* endValues);

    // ---- lifecycle ----
    virtual void pause(View* sceneRoot);
    virtual void resume(View* sceneRoot);
    void playTransition(ViewGroup* sceneRoot);
    // Public (android package-private) so TransitionSet's sequential chaining can start
    // the next child via next->runAnimators().
    virtual void runAnimators();
    virtual void forceToEnd(ViewGroup* sceneRoot);
    virtual void cancel();

    // addListener/removeListener take TransitionListener by value (EventSet identity via
    // shared mID: a copy compares equal to its original, so removeListener finds it).
    // The Transition owns its listeners (stored in mListeners) and frees them on destruction.
    Transition& addListener(const TransitionListener& listener);
    Transition& removeListener(const TransitionListener& listener);

    // CDROID ownership: clones created per transition run (beginDelayedTransition/go -> clone)
    // are throwaway (java reclaims via GC). Mark a clone so end() defers delete-this to the next
    // UI-thread looper iteration via a heap Handler (self-deleting); only the top-level clone is
    // marked — children of a TransitionSet clone are owned by the set.
    void setDeleteWhenEnded(bool b);

    // ---- propagation / epicenter / path motion ----
    virtual void setEpicenterCallback(EpicenterCallback* epicenterCallback);
    EpicenterCallback* getEpicenterCallback() const;
    Rect getEpicenter() const;
    virtual void setPathMotion(PathMotion* pathMotion);
    PathMotion* getPathMotion() const;
    virtual void setPropagation(TransitionPropagation* transitionPropagation);
    TransitionPropagation* getPropagation() const;

    // ---- scene-root / removal / name overrides (engine hooks) ----
    virtual Transition* setSceneRoot(ViewGroup* sceneRoot);
    virtual void setCanRemoveViews(bool canRemoveViews);
    bool canRemoveViews() const;
    void setNameOverrides(const ArrayMap<std::string, std::string>& overrides);
    ArrayMap<std::string, std::string>& getNameOverrides();

    std::string getName() const;
    std::string toString();

    // android: static final boolean DBG (package-private). Public so TransitionManager
    // (same "package" conceptually) can read it.
    static constexpr bool DBG = false;

    virtual Transition* clone() const;

  protected:
    // Engine entry points called by TransitionManager / TransitionSet.
    virtual void createAnimators(ViewGroup* sceneRoot, TransitionValuesMaps& startValues,
                                 TransitionValuesMaps& endValues, std::vector<TransitionValuesPtr>& startValuesList,
                                 std::vector<TransitionValuesPtr>& endValuesList);
    void animate(Animator* animator);
    void start();
    void end();
    virtual void capturePropagationValues(TransitionValues& transitionValues);

    // Shared shallow-copy helper used by subclass clone() implementations.
    void copyCloneFields(Transition* clone) const;

    virtual std::string toString(const std::string& indent);

    // ---- fields (mirror android.transition.Transition; package-private there) ----
    static constexpr int DEFAULT_MATCH_ORDER_ARRAY[4] = {MATCH_NAME, MATCH_INSTANCE, MATCH_ID, MATCH_ITEM_ID};

    std::string mName;
    int64_t mStartDelay = -1;
    int64_t mDuration = -1;
    const TimeInterpolator* mInterpolator = nullptr;
    std::vector<int>             mTargetIds;
    std::vector<View*>           mTargets;
    std::vector<std::string>     mTargetNames;
    std::vector<std::type_index> mTargetTypes;
    std::vector<int>             mTargetIdExcludes;
    std::vector<View*>           mTargetExcludes;
    std::vector<std::type_index> mTargetTypeExcludes;
    std::vector<std::string>     mTargetNameExcludes;
    std::vector<int>             mTargetIdChildExcludes;
    std::vector<View*>           mTargetChildExcludes;
    std::vector<std::type_index> mTargetTypeChildExcludes;
    TransitionValuesMaps mStartValues;
    TransitionValuesMaps mEndValues;
    Transition* mParent = nullptr;     // TransitionSet* in practice
    std::vector<int> mMatchOrder{MATCH_NAME, MATCH_INSTANCE, MATCH_ID, MATCH_ITEM_ID};
    std::vector<TransitionValuesPtr>* mStartValuesList = nullptr; // valid only after playTransition
    std::vector<TransitionValuesPtr>* mEndValuesList = nullptr;
    ViewGroup* mSceneRoot = nullptr;
    bool mCanRemoveViews = false;
    std::vector<Animator*> mCurrentAnimators;
    int  mNumInstances = 0;
    bool mPaused = false;
    bool mEnded = false;
    bool mDeleteWhenEnded = false; // throwaway clone: queue for deferred deletion on end()
    std::vector<TransitionListener> mListeners;
    std::vector<Animator*> mAnimators;
    std::vector<Animator*> mOwnedAnimators; // created by createAnimators; owned, freed in ~Transition
    TransitionPropagation* mPropagation = nullptr;
    EpicenterCallback* mEpicenterCallback = nullptr;
    ArrayMap<std::string, std::string> mNameOverrides; // empty == null (value semantics; copy-safe)
    PathMotion* mPathMotion; // set in ctor to STRAIGHT_PATH_MOTION instance

  private:
    friend class TransitionSet; // addTransition sets child->mParent

    // The straight-line default PathMotion (android: STRAIGHT_PATH_MOTION anonymous subclass).
    static PathMotion* straightPathMotion();

    void matchInstances(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
                        ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd);
    void matchItemIds(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
                      ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd,
                      LongSparseArray<View*>& startItemIds, LongSparseArray<View*>& endItemIds);
    void matchIds(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
                  ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd,
                  SparseArray<View*>& startIds, SparseArray<View*>& endIds);
    void matchNames(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
                    ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd,
                    ArrayMap<std::string, View*>& startNames, ArrayMap<std::string, View*>& endNames);
    void addUnmatched(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
                      ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd);
    void matchStartAndEnd(TransitionValuesMaps& startValues, TransitionValuesMaps& endValues);
    void captureHierarchy(View* view, bool start);
    void runAnimator(Animator* animator);

    static bool isValidMatch(int match);
    static bool alreadyContains(const std::vector<int>& array, int searchIndex);
    static std::vector<int> parseMatchOrder(const std::string& matchOrderString);
    static bool isValueChanged(const TransitionValues& oldValues,
                               const TransitionValues& newValues, const std::string& key);

    // Returns the per-process map of currently running animators (ThreadLocal in android).
    static ArrayMap<Animator*, AnimationInfo>& getRunningAnimators();

    /**
     * Utility class for managing ArrayLists efficiently (android.transition.Transition
     * .ArrayListManager). Kept as a nested class to mirror the java source. The lists
     * it manages are non-null std::vectors where empty == android's null.
     */
    class ArrayListManager {
      public:
        template<typename T>
        static std::vector<T>& add(std::vector<T>& list, const T& item) {
            if (!Transition::contains(list, item)) {
                list.push_back(item);
            }
            return list;
        }
        template<typename T>
        static std::vector<T>& remove(std::vector<T>& list, const T& item) {
            list.erase(std::remove(list.begin(), list.end(), item), list.end());
            return list;
        }
    };

    template<typename T>
    static void excludeObject(std::vector<T>& list, const T& target, bool exclude) {
        if (exclude) {
            ArrayListManager::add(list, target);
        } else {
            ArrayListManager::remove(list, target);
        }
    }
    template<typename T>
    static bool contains(const std::vector<T>& list, const T& item) {
        return std::find(list.begin(), list.end(), item) != list.end();
    }
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITION_H__
