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
#ifndef __CDROID_TRANSITION_TRANSITIONSET_H__
#define __CDROID_TRANSITION_TRANSITIONSET_H__

#include <vector>

#include <transition/transition.h>
#include <transition/transitionlisteneradapter.h>

namespace cdroid {

class Context;
class AttributeSet;

/**
 * A TransitionSet is a parent of child transitions (including other TransitionSets).
 * Child transitions play {@link #ORDERING_TOGETHER} or {@link #ORDERING_SEQUENTIAL}.
 * Ported from android-36 android.transition.TransitionSet.
 *
 * Ownership: children added via addTransition() are owned by the set (deleted at
 * destruction, deep-cloned by clone()). This mirrors the java GC reference the set
 * holds and lets `set.addTransition(new Fade())` be leak-free.
 */
class TransitionSet: public Transition {
  public:
    static constexpr int ORDERING_TOGETHER  = 0;
    static constexpr int ORDERING_SEQUENTIAL = 1;

    TransitionSet();
    TransitionSet(Context* context, AttributeSet* attrs);
    ~TransitionSet() override;

    TransitionSet& setOrdering(int ordering);
    int getOrdering() const {
        return mPlayTogether ? ORDERING_TOGETHER : ORDERING_SEQUENTIAL;
    }

    TransitionSet& addTransition(Transition* transition);
    TransitionSet& removeTransition(Transition* transition);
    int getTransitionCount() const {
        return (int)mTransitions.size();
    }
    Transition* getTransitionAt(int index);

    TransitionSet& setDuration(int64_t duration) override;
    TransitionSet& setStartDelay(int64_t startDelay) override {
        return (TransitionSet&)Transition::setStartDelay(startDelay);
    }
    TransitionSet& setInterpolator(const TimeInterpolator* interpolator) override;
    void setPathMotion(PathMotion* pathMotion) override;
    void setPropagation(TransitionPropagation* transitionPropagation) override;
    void setEpicenterCallback(EpicenterCallback* epicenterCallback) override;

    Transition& addTarget(View* target) override;
    Transition& addTarget(int targetId) override;
    Transition& addTarget(const std::string& targetName) override;
    Transition& addTarget(const std::type_index& targetType) override;

    Transition& removeTarget(int targetId) override;
    Transition& removeTarget(View* target) override;
    Transition& removeTarget(const std::type_index& targetType) override;
    Transition& removeTarget(const std::string& target) override;

    Transition& excludeTarget(View* target, bool exclude) override;
    Transition& excludeTarget(const std::string& targetName, bool exclude) override;
    Transition& excludeTarget(int targetId, bool exclude) override;
    Transition& excludeTarget(const std::type_index& type, bool exclude) override;

    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;
    void capturePropagationValues(TransitionValues& transitionValues) override;

    void pause(View* sceneRoot) override;
    void resume(View* sceneRoot) override;
    void cancel() override;
    void forceToEnd(ViewGroup* sceneRoot) override;
    Transition* setSceneRoot(ViewGroup* sceneRoot) override;
    void setCanRemoveViews(bool canRemoveViews) override;

    TransitionSet* clone() const override {
        TransitionSet* c = new TransitionSet(*this);
        copyCloneFields(c);
        cloneChildrenInto(*c);
        return c;
    }

  protected:
    void createAnimators(ViewGroup* sceneRoot, TransitionValuesMaps& startValues,
                         TransitionValuesMaps& endValues, std::vector<TransitionValuesPtr>& startValuesList,
                         std::vector<TransitionValuesPtr>& endValuesList) override;
    void runAnimators() override;
    std::string toString(const std::string& indent) override;

  private:
    static constexpr int FLAG_CHANGE_INTERPOLATOR = 0x01;
    static constexpr int FLAG_CHANGE_PROPAGATION = 0x02;
    static constexpr int FLAG_CHANGE_PATH_MOTION  = 0x04;
    static constexpr int FLAG_CHANGE_EPICENTER    = 0x08;

    void addTransitionInternal(Transition* transition);
    void setupStartEndListeners();
    void cloneChildrenInto(TransitionSet& clone) const;

    // android: static nested TransitionSetListener tracks "all children done" by being
    // added to every child. Now an EventSet TransitionListener VALUE member (mSetListener)
    // whose onTransitionStart/End lambdas are wired in setupStartEndListeners and copied
    // onto each child (copies share EventSet mID). No subclass / no new.

    std::vector<Transition*> mTransitions; // owned children
    bool mPlayTogether = true;
    int  mCurrentListeners = 0;
    bool mStarted = false;
    int  mChangeFlags = 0;
    Transition::TransitionListener mSetListener; // value; callbacks wired per runAnimators, copied onto each child
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONSET_H__
