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
#include <transition/transitionset.h>

#include <algorithm>
#include <stdexcept>

#include <core/attributeset.h>
#include <core/context.h>
#include <view/view.h>
#include <view/viewgroup.h>

namespace cdroid {

// android: anonymous TransitionListener in runAnimators (sequential) that starts the next
// child when the previous one ends. Now wired inline in runAnimators as an EventSet
// TransitionListener value (no subclass / no new).

TransitionSet::TransitionSet() = default;

TransitionSet::TransitionSet(Context* context, AttributeSet* attrs)
    : Transition(context, attrs) {
    // android: obtainStyledAttributes(attrs, R.styleable.TransitionSet) → transitionOrdering.
    // CDROID reads the attribute directly.
    int ordering = ORDERING_TOGETHER;
    if (attrs != nullptr) {
        std::string ord = attrs->getAttributeValue("transitionOrdering");
        if (ord == "sequential") ordering = ORDERING_SEQUENTIAL;
        else if (ord == "together") ordering = ORDERING_TOGETHER;
    }
    setOrdering(ordering);
}

TransitionSet::~TransitionSet() {
    for (Transition* child : mTransitions) {
        delete child; // set owns its children
    }
    // mSetListener is a value member (EventSet) — freed automatically.
}

TransitionSet& TransitionSet::setOrdering(int ordering) {
    switch (ordering) {
    case ORDERING_SEQUENTIAL:
        mPlayTogether = false;
        break;
    case ORDERING_TOGETHER:
        mPlayTogether = true;
        break;
    default:
        throw std::invalid_argument("Invalid parameter for TransitionSet ordering");
    }
    return *this;
}

void TransitionSet::addTransitionInternal(Transition* transition) {
    mTransitions.push_back(transition);
    transition->mParent = this;
}

TransitionSet& TransitionSet::addTransition(Transition* transition) {
    if (transition != nullptr) {
        addTransitionInternal(transition);
        if (mDuration >= 0) {
            transition->setDuration(mDuration);
        }
        if ((mChangeFlags & FLAG_CHANGE_INTERPOLATOR) != 0) {
            transition->setInterpolator(getInterpolator());
        }
        if ((mChangeFlags & FLAG_CHANGE_PROPAGATION) != 0) {
            transition->setPropagation(getPropagation());
        }
        if ((mChangeFlags & FLAG_CHANGE_PATH_MOTION) != 0) {
            transition->setPathMotion(getPathMotion());
        }
        if ((mChangeFlags & FLAG_CHANGE_EPICENTER) != 0) {
            transition->setEpicenterCallback(getEpicenterCallback());
        }
    }
    return *this;
}

TransitionSet& TransitionSet::removeTransition(Transition* transition) {
    auto it = std::find(mTransitions.begin(), mTransitions.end(), transition);
    if (it != mTransitions.end()) {
        mTransitions.erase(it);
        if (transition) transition->mParent = nullptr;
        // Ownership returns to the caller (android detaches; GC frees later).
    }
    return *this;
}

Transition* TransitionSet::getTransitionAt(int index) {
    if (index < 0 || index >= (int)mTransitions.size()) {
        return nullptr;
    }
    return mTransitions[index];
}

TransitionSet& TransitionSet::setDuration(int64_t duration) {
    Transition::setDuration(duration);
    if (mDuration >= 0) {
        for (Transition* child : mTransitions) {
            child->setDuration(mDuration);
        }
    }
    return *this;
}

TransitionSet& TransitionSet::setInterpolator(const TimeInterpolator* interpolator) {
    mChangeFlags |= FLAG_CHANGE_INTERPOLATOR;
    for (Transition* child : mTransitions) {
        child->setInterpolator(interpolator);
    }
    Transition::setInterpolator(interpolator);
    return *this;
}

void TransitionSet::setPathMotion(PathMotion* pathMotion) {
    Transition::setPathMotion(pathMotion);
    mChangeFlags |= FLAG_CHANGE_PATH_MOTION;
    for (Transition* child : mTransitions) {
        child->setPathMotion(pathMotion);
    }
}

void TransitionSet::setPropagation(TransitionPropagation* transitionPropagation) {
    Transition::setPropagation(transitionPropagation);
    mChangeFlags |= FLAG_CHANGE_PROPAGATION;
    for (Transition* child : mTransitions) {
        child->setPropagation(transitionPropagation);
    }
}

void TransitionSet::setEpicenterCallback(EpicenterCallback* epicenterCallback) {
    Transition::setEpicenterCallback(epicenterCallback);
    mChangeFlags |= FLAG_CHANGE_EPICENTER;
    for (Transition* child : mTransitions) {
        child->setEpicenterCallback(epicenterCallback);
    }
}

// ---- target delegation ----
Transition& TransitionSet::addTarget(View* target) {
    for (Transition* child : mTransitions) child->addTarget(target);
    return Transition::addTarget(target);
}
Transition& TransitionSet::addTarget(int targetId) {
    for (Transition* child : mTransitions) child->addTarget(targetId);
    return Transition::addTarget(targetId);
}
Transition& TransitionSet::addTarget(const std::string& targetName) {
    for (Transition* child : mTransitions) child->addTarget(targetName);
    return Transition::addTarget(targetName);
}
Transition& TransitionSet::addTarget(const std::type_index& targetType) {
    for (Transition* child : mTransitions) child->addTarget(targetType);
    return Transition::addTarget(targetType);
}

Transition& TransitionSet::removeTarget(int targetId) {
    for (Transition* child : mTransitions) child->removeTarget(targetId);
    return Transition::removeTarget(targetId);
}
Transition& TransitionSet::removeTarget(View* target) {
    for (Transition* child : mTransitions) child->removeTarget(target);
    return Transition::removeTarget(target);
}
Transition& TransitionSet::removeTarget(const std::type_index& targetType) {
    for (Transition* child : mTransitions) child->removeTarget(targetType);
    return Transition::removeTarget(targetType);
}
Transition& TransitionSet::removeTarget(const std::string& target) {
    for (Transition* child : mTransitions) child->removeTarget(target);
    return Transition::removeTarget(target);
}

Transition& TransitionSet::excludeTarget(View* target, bool exclude) {
    for (Transition* child : mTransitions) child->excludeTarget(target, exclude);
    return Transition::excludeTarget(target, exclude);
}
Transition& TransitionSet::excludeTarget(const std::string& targetName, bool exclude) {
    for (Transition* child : mTransitions) child->excludeTarget(targetName, exclude);
    return Transition::excludeTarget(targetName, exclude);
}
Transition& TransitionSet::excludeTarget(int targetId, bool exclude) {
    for (Transition* child : mTransitions) child->excludeTarget(targetId, exclude);
    return Transition::excludeTarget(targetId, exclude);
}
Transition& TransitionSet::excludeTarget(const std::type_index& type, bool exclude) {
    for (Transition* child : mTransitions) child->excludeTarget(type, exclude);
    return Transition::excludeTarget(type, exclude);
}

// ---- capture ----
void TransitionSet::captureStartValues(TransitionValues& transitionValues) {
    if (isValidTarget(transitionValues.view)) {
        for (Transition* child : mTransitions) {
            if (child->isValidTarget(transitionValues.view)) {
                child->captureStartValues(transitionValues);
                transitionValues.targetedTransitions.push_back(child);
            }
        }
    }
}

void TransitionSet::captureEndValues(TransitionValues& transitionValues) {
    if (isValidTarget(transitionValues.view)) {
        for (Transition* child : mTransitions) {
            if (child->isValidTarget(transitionValues.view)) {
                child->captureEndValues(transitionValues);
                transitionValues.targetedTransitions.push_back(child);
            }
        }
    }
}

void TransitionSet::capturePropagationValues(TransitionValues& transitionValues) {
    Transition::capturePropagationValues(transitionValues);
    for (Transition* child : mTransitions) {
        child->capturePropagationValues(transitionValues);
    }
}

// ---- createAnimators / runAnimators ----
void TransitionSet::createAnimators(ViewGroup* sceneRoot, TransitionValuesMaps& startValues,
                                    TransitionValuesMaps& endValues, std::vector<TransitionValuesPtr>& startValuesList,
                                    std::vector<TransitionValuesPtr>& endValuesList) {
    int64_t startDelay = getStartDelay();
    int numTransitions = (int)mTransitions.size();
    for (int i = 0; i < numTransitions; i++) {
        Transition* childTransition = mTransitions[i];
        if (startDelay > 0 && (mPlayTogether || i == 0)) {
            int64_t childStartDelay = childTransition->getStartDelay();
            if (childStartDelay > 0) {
                childTransition->setStartDelay(startDelay + childStartDelay);
            } else {
                childTransition->setStartDelay(startDelay);
            }
        }
        childTransition->createAnimators(sceneRoot, startValues, endValues, startValuesList, endValuesList);
    }
}

void TransitionSet::setupStartEndListeners() {
    // mSetListener is a value member; wire its callbacks to this set, then copy it onto
    // every child (EventSet copies share mID; each child's onTransitionEnd decrements the
    // shared counter). android: a single shared listener tracks "all children done".
    TransitionSet* self = this;
    mSetListener.onTransitionStart = [self](Transition&) {
        if (!self->mStarted) {
            self->start();
            self->mStarted = true;
        }
    };
    mSetListener.onTransitionEnd = [self](Transition&) {
        if (--self->mCurrentListeners == 0) {
            // All child transitions are done.
            self->mStarted = false;
            self->end();
        }
    };
    for (Transition* child : mTransitions) {
        child->addListener(mSetListener);
    }
    mCurrentListeners = (int)mTransitions.size();
}

void TransitionSet::runAnimators() {
    if (mTransitions.empty()) {
        start();
        end();
        return;
    }
    setupStartEndListeners();
    int numTransitions = (int)mTransitions.size();
    if (!mPlayTogether) {
        for (int i = 1; i < numTransitions; ++i) {
            Transition* previousTransition = mTransitions[i - 1];
            Transition* nextTransition = mTransitions[i];
            Transition::TransitionListener sl;
            sl.onTransitionEnd = [nextTransition](Transition&) {
                nextTransition->runAnimators();
            };
            previousTransition->addListener(sl);
        }
        Transition* firstTransition = mTransitions[0];
        if (firstTransition != nullptr) {
            firstTransition->runAnimators();
        }
    } else {
        for (int i = 0; i < numTransitions; ++i) {
            mTransitions[i]->runAnimators();
        }
    }
}

// ---- lifecycle delegation ----
void TransitionSet::pause(View* sceneRoot) {
    Transition::pause(sceneRoot);
    for (Transition* child : mTransitions) {
        child->pause(sceneRoot);
    }
}

void TransitionSet::resume(View* sceneRoot) {
    Transition::resume(sceneRoot);
    for (Transition* child : mTransitions) {
        child->resume(sceneRoot);
    }
}

void TransitionSet::cancel() {
    Transition::cancel();
    for (Transition* child : mTransitions) {
        child->cancel();
    }
}

void TransitionSet::forceToEnd(ViewGroup* sceneRoot) {
    Transition::forceToEnd(sceneRoot);
    for (Transition* child : mTransitions) {
        child->forceToEnd(sceneRoot);
    }
}

Transition* TransitionSet::setSceneRoot(ViewGroup* sceneRoot) {
    Transition::setSceneRoot(sceneRoot);
    for (Transition* child : mTransitions) {
        child->setSceneRoot(sceneRoot);
    }
    return this;
}

void TransitionSet::setCanRemoveViews(bool canRemoveViews) {
    Transition::setCanRemoveViews(canRemoveViews);
    for (Transition* child : mTransitions) {
        child->setCanRemoveViews(canRemoveViews);
    }
}

std::string TransitionSet::toString(const std::string& indent) {
    std::string result = Transition::toString(indent);
    for (Transition* child : mTransitions) {
        result += "\n" + child->toString(indent + "  ");
    }
    return result;
}

// ---- clone ----
void TransitionSet::cloneChildrenInto(TransitionSet& clone) const {
    clone.mTransitions.clear(); // drop shallow copies (the original still owns them)
    clone.mStarted = false;
    clone.mCurrentListeners = 0;
    for (Transition* child : mTransitions) {
        clone.addTransitionInternal(child->clone());
    }
}

} // namespace cdroid
