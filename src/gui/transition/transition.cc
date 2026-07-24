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
#include <transition/transition.h>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <animation/animator.h>
#include <animation/interpolators.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/abslistview.h>
#include <widget/adapter.h>
#include <widget/listview.h>

namespace cdroid{

namespace {
constexpr const char* LOG_TAG = "Transition";

// android: anonymous PathMotion subclass STRAIGHT_PATH_MOTION. Kept as a named
// concrete subclass (C++ has no anonymous classes).
class StraightPathMotion: public PathMotion{
public:
    Path getPath(float startX, float startY, float endX, float endY) override{
        Path path;
        path.moveTo(startX, startY);
        path.lineTo(endX, endY);
        return path;
    }
};
} // anonymous namespace

// android: STRAIGHT_PATH_MOTION = new PathMotion(){...}. Singleton accessor.
PathMotion* Transition::straightPathMotion(){
    static StraightPathMotion instance;
    return &instance;
}

// Out-of-class definitions for ODR-used static constexpr members. In C++14 (unlike
// C++17's implicit inline) a constexpr static data member that is ODR-used (e.g.
// MATCH_* bound to a const ref in vector::push_back, or DEFAULT_MATCH_ORDER_ARRAY
// decaying to a pointer in vector::assign) requires a separate definition.
constexpr int Transition::MATCH_INSTANCE;
constexpr int Transition::MATCH_NAME;
constexpr int Transition::MATCH_ID;
constexpr int Transition::MATCH_ITEM_ID;
constexpr int Transition::DEFAULT_MATCH_ORDER_ARRAY[4];

// ---- AnimationInfo ----
Transition::AnimationInfo::AnimationInfo(View* v, const std::string& n, Transition* t,
        void* wid, const TransitionValuesPtr& vals)
    : view(v), name(n), values(vals), windowId(wid), transition(t){
}

// ---- construction ----
Transition::Transition(){
    mPathMotion = straightPathMotion();
}

Transition::Transition(Context* /*context*/, AttributeSet* attrs){
    mPathMotion = straightPathMotion();
    // android uses context.obtainStyledAttributes(attrs, R.styleable.Transition).
    // CDROID reads attributes directly from AttributeSet (TypedArray is rarely used).
    if (attrs == nullptr){
        return;
    }
    std::string d = attrs->getAttributeValue("duration");
    if (!d.empty()){
        long long duration = atoll(d.c_str());
        if (duration >= 0){
            setDuration(duration);
        }
    }
    std::string sd = attrs->getAttributeValue("startDelay");
    if (!sd.empty()){
        long long startDelay = atoll(sd.c_str());
        if (startDelay > 0){
            setStartDelay(startDelay);
        }
    }
    // interpolator: android loads via AnimationUtils.loadInterpolator(context, resID).
    // CDROID resource->interpolator wiring is deferred (TODO: wire when needed).
    std::string matchOrder = attrs->getAttributeValue("matchOrder");
    if (!matchOrder.empty()){
        setMatchOrder(parseMatchOrder(matchOrder));
    }
}

Transition::~Transition(){
    delete mStartValuesList;
    delete mEndValuesList;
    // Listeners are NOT owned by the Transition (java reference/GC semantics): the same
    // listener may be registered on several transitions (e.g. TransitionSet's shared
    // TransitionSetListener), so deleting here would double-free. Listeners are caller-
    // managed; in practice they are transient (self-removing on end) and leak per
    // transition run like java objects awaiting GC — a known, bounded consequence of
    // mirroring the GC'd API. (A proper ownership pass can switch to shared_ptr later.)
}

std::vector<int> Transition::parseMatchOrder(const std::string& matchOrderString){
    // android: StringTokenizer(matchOrderString, ","). C++: manual split on ','.
    std::vector<int> matches;
    std::string token;
    std::istringstream iss(matchOrderString);
    while (std::getline(iss, token, ',')){
        // trim
        size_t b = token.find_first_not_of(" \t");
        size_t e = token.find_last_not_of(" \t");
        std::string t = (b == std::string::npos) ? std::string() : token.substr(b, e - b + 1);
        if (t == "id")             matches.push_back(Transition::MATCH_ID);
        else if (t == "instance")  matches.push_back(Transition::MATCH_INSTANCE);
        else if (t == "name")      matches.push_back(Transition::MATCH_NAME);
        else if (t == "viewName")  matches.push_back(Transition::MATCH_NAME); // deprecated alias
        else if (t == "itemId")    matches.push_back(Transition::MATCH_ITEM_ID);
        else if (t.empty())        { /* skip empty token */ }
        else                       matches.push_back(-1); // invalid marker; setMatchOrder rejects
    }
    return matches;
}

// ---- duration / delay / interpolator ----
Transition& Transition::setDuration(int64_t duration){ mDuration = duration; return *this; }
int64_t Transition::getDuration() const{ return mDuration; }
Transition& Transition::setStartDelay(int64_t startDelay){ mStartDelay = startDelay; return *this; }
int64_t Transition::getStartDelay() const{ return mStartDelay; }
Transition& Transition::setInterpolator(const TimeInterpolator* interpolator){ mInterpolator = interpolator; return *this; }
const TimeInterpolator* Transition::getInterpolator() const{ return mInterpolator; }

std::vector<std::string> Transition::getTransitionProperties(){
    return {}; // android: null
}

Animator* Transition::createAnimator(ViewGroup* /*sceneRoot*/,
        TransitionValues* /*startValues*/, TransitionValues* /*endValues*/){
    return nullptr;
}

// ---- match order ----
void Transition::setMatchOrder(const std::vector<int>& matches){
    if (matches.empty()){
        mMatchOrder.assign(DEFAULT_MATCH_ORDER_ARRAY, DEFAULT_MATCH_ORDER_ARRAY + 4);
        return;
    }
    for (size_t i = 0; i < matches.size(); i++){
        int match = matches[i];
        if (!isValidMatch(match)){
            throw std::invalid_argument("matches contains invalid value");
        }
        if (alreadyContains(matches, (int)i)){
            throw std::invalid_argument("matches contains a duplicate value");
        }
    }
    mMatchOrder = matches;
}

bool Transition::isValidMatch(int match){
    return (match >= MATCH_FIRST && match <= MATCH_LAST);
}

bool Transition::alreadyContains(const std::vector<int>& array, int searchIndex){
    int value = array[searchIndex];
    for (int i = 0; i < searchIndex; i++){
        if (array[i] == value){
            return true;
        }
    }
    return false;
}

// ---- matching (private) ----
void Transition::matchInstances(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
        ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd){
    // Walk in reverse and mutate via removeAt (android does the same).
    for (int i = unmatchedStart.size() - 1; i >= 0; i--){
        View* view = unmatchedStart.keyAt(i);
        if (view != nullptr && isValidTarget(view)){
            int endIdx = unmatchedEnd.indexOfKey(view);
            if (endIdx >= 0){
                TransitionValuesPtr end = unmatchedEnd.valueAt(endIdx);
                if (end && isValidTarget(end->view)){
                    TransitionValuesPtr start = unmatchedStart.valueAt(i);
                    unmatchedStart.removeAt(i);
                    unmatchedEnd.removeAt(endIdx);
                    mStartValuesList->push_back(start);
                    mEndValuesList->push_back(end);
                }
            }
        }
    }
}

void Transition::matchItemIds(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
        ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd,
        LongSparseArray<View*>& startItemIds, LongSparseArray<View*>& endItemIds){
    int numStartIds = (int)startItemIds.size();
    for (int i = 0; i < numStartIds; i++){
        View* startView = startItemIds.valueAt(i);
        if (startView != nullptr && isValidTarget(startView)){
            View* endView = endItemIds.get(startItemIds.keyAt(i));
            if (endView != nullptr && isValidTarget(endView)){
                TransitionValuesPtr* startValues = unmatchedStart.get(startView);
                TransitionValuesPtr* endValues = unmatchedEnd.get(endView);
                if (startValues && *startValues && endValues && *endValues){
                    mStartValuesList->push_back(*startValues);
                    mEndValuesList->push_back(*endValues);
                    unmatchedStart.remove(startView);
                    unmatchedEnd.remove(endView);
                }
            }
        }
    }
}

void Transition::matchIds(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
        ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd,
        SparseArray<View*>& startIds, SparseArray<View*>& endIds){
    int numStartIds = (int)startIds.size();
    for (int i = 0; i < numStartIds; i++){
        View* startView = startIds.valueAt(i);
        if (startView != nullptr && isValidTarget(startView)){
            View* endView = endIds.get(startIds.keyAt(i));
            if (endView != nullptr && isValidTarget(endView)){
                TransitionValuesPtr* startValues = unmatchedStart.get(startView);
                TransitionValuesPtr* endValues = unmatchedEnd.get(endView);
                if (startValues && *startValues && endValues && *endValues){
                    mStartValuesList->push_back(*startValues);
                    mEndValuesList->push_back(*endValues);
                    unmatchedStart.remove(startView);
                    unmatchedEnd.remove(endView);
                }
            }
        }
    }
}

void Transition::matchNames(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
        ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd,
        ArrayMap<std::string, View*>& startNames, ArrayMap<std::string, View*>& endNames){
    int numStartNames = (int)startNames.size();
    for (int i = 0; i < numStartNames; i++){
        View* startView = startNames.valueAt(i);
        if (startView != nullptr && isValidTarget(startView)){
            View** endViewPtr = endNames.get(startNames.keyAt(i));
            View* endView = endViewPtr ? *endViewPtr : nullptr;
            if (endView != nullptr && isValidTarget(endView)){
                TransitionValuesPtr* startValues = unmatchedStart.get(startView);
                TransitionValuesPtr* endValues = unmatchedEnd.get(endView);
                if (startValues && *startValues && endValues && *endValues){
                    mStartValuesList->push_back(*startValues);
                    mEndValuesList->push_back(*endValues);
                    unmatchedStart.remove(startView);
                    unmatchedEnd.remove(endView);
                }
            }
        }
    }
}

void Transition::addUnmatched(ArrayMap<View*, TransitionValuesPtr>& unmatchedStart,
        ArrayMap<View*, TransitionValuesPtr>& unmatchedEnd){
    // Views that only exist in the start Scene
    for (int i = 0; i < unmatchedStart.size(); i++){
        const TransitionValuesPtr& start = unmatchedStart.valueAt(i);
        if (start && isValidTarget(start->view)){
            mStartValuesList->push_back(start);
            mEndValuesList->push_back(TransitionValuesPtr()); // null
        }
    }
    // Views that only exist in the end Scene
    for (int i = 0; i < unmatchedEnd.size(); i++){
        const TransitionValuesPtr& end = unmatchedEnd.valueAt(i);
        if (end && isValidTarget(end->view)){
            mEndValuesList->push_back(end);
            mStartValuesList->push_back(TransitionValuesPtr()); // null
        }
    }
}

void Transition::matchStartAndEnd(TransitionValuesMaps& startValues, TransitionValuesMaps& endValues){
    ArrayMap<View*, TransitionValuesPtr> unmatchedStart(startValues.viewValues);
    ArrayMap<View*, TransitionValuesPtr> unmatchedEnd(endValues.viewValues);

    for (int match : mMatchOrder){
        switch (match){
            case MATCH_INSTANCE:
                matchInstances(unmatchedStart, unmatchedEnd);
                break;
            case MATCH_NAME:
                matchNames(unmatchedStart, unmatchedEnd, startValues.nameValues, endValues.nameValues);
                break;
            case MATCH_ID:
                matchIds(unmatchedStart, unmatchedEnd, startValues.idValues, endValues.idValues);
                break;
            case MATCH_ITEM_ID:
                matchItemIds(unmatchedStart, unmatchedEnd, startValues.itemIdValues, endValues.itemIdValues);
                break;
            default: break;
        }
    }
    addUnmatched(unmatchedStart, unmatchedEnd);
}

// ---- createAnimators ----
void Transition::createAnimators(ViewGroup* sceneRoot, TransitionValuesMaps& startValues,
        TransitionValuesMaps& endValues, std::vector<TransitionValuesPtr>& startValuesList,
        std::vector<TransitionValuesPtr>& endValuesList){
    if (DBG){ LOGD("createAnimators() for %s", toString().c_str()); }
    ArrayMap<Animator*, AnimationInfo>& runningAnimators = getRunningAnimators();
    int64_t minStartDelay = std::numeric_limits<int64_t>::max();
    SparseLongArray startDelays;
    int startValuesListCount = (int)startValuesList.size();
    for (int i = 0; i < startValuesListCount; ++i){
        TransitionValuesPtr start = startValuesList[i];
        TransitionValuesPtr end = endValuesList[i];
        if (start && !contains(start->targetedTransitions, this)){
            start.reset();
        }
        if (end && !contains(end->targetedTransitions, this)){
            end.reset();
        }
        if (!start && !end){
            continue;
        }
        // Only bother trying to animate with values that differ between start/end
        bool isChanged = !start || !end || isTransitionRequired(start.get(), end.get());
        if (isChanged){
            if (DBG){
                View* view = end ? end->view : (start ? start->view : nullptr);
                LOGD("  differing start/end values for view %p", (void*)view);
            }
            Animator* animator = createAnimator(sceneRoot, start.get(), end.get());
            if (animator != nullptr){
                View* view = nullptr;
                TransitionValuesPtr infoValues;
                if (end){
                    view = end->view;
                    std::vector<std::string> properties = getTransitionProperties();
                    if (!properties.empty()){
                        infoValues = std::make_shared<TransitionValues>(view);
                        TransitionValuesPtr* newValues = endValues.viewValues.get(view);
                        if (newValues && *newValues){
                            for (const std::string& prop : properties){
                                auto vit = (*newValues)->values.find(prop);
                                if (vit != (*newValues)->values.end()){
                                    infoValues->values[prop] = vit->second; // copy the any
                                }
                            }
                        }
                        int numExistingAnims = runningAnimators.size();
                        for (int j = 0; j < numExistingAnims; ++j){
                            Animator* anim = runningAnimators.keyAt(j);
                            AnimationInfo* info = runningAnimators.get(anim);
                            if (info && info->values && info->view == view &&
                                    ((info->name.empty() && getName().empty()) || info->name == getName())){
                                if (info->values->equals(*infoValues)){
                                    // Favor the old animator
                                    animator = nullptr;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    view = start ? start->view : nullptr;
                }
                if (animator != nullptr){
                    if (mPropagation != nullptr){
                        int64_t delay = mPropagation->getStartDelay(sceneRoot, this,
                                start.get(), end.get());
                        startDelays.put((int)mAnimators.size(), delay);
                        minStartDelay = std::min(delay, minStartDelay);
                    }
                    AnimationInfo info(view, getName(), this, sceneRoot->getWindowId(), infoValues);
                    runningAnimators.put(animator, info);
                    mAnimators.push_back(animator);
                }
            }
        }
    }
    if (startDelays.size() != 0){
        for (size_t i = 0; i < startDelays.size(); i++){
            int index = (int)startDelays.keyAt(i);
            Animator* animator = mAnimators[index];
            int64_t delay = startDelays.valueAt(i) - minStartDelay + animator->getStartDelay();
            animator->setStartDelay(delay);
        }
    }
}

bool Transition::isValidTarget(View* target) const{
    if (target == nullptr){
        return false;
    }
    int targetId = target->getId();
    if (!mTargetIdExcludes.empty() && contains(mTargetIdExcludes, targetId)){
        return false;
    }
    if (!mTargetExcludes.empty() && contains(mTargetExcludes, target)){
        return false;
    }
    if (!mTargetTypeExcludes.empty()){
        const std::type_index tid = std::type_index(typeid(*target));
        for (const std::type_index& type : mTargetTypeExcludes){
            if (type == tid){ // android: type.isInstance(target) — exact match (subclass limitation)
                return false;
            }
        }
    }
    if (!mTargetNameExcludes.empty()){
        const std::string& tn = target->getTransitionName();
        if (!tn.empty() && contains(mTargetNameExcludes, tn)){
            return false;
        }
    }
    if (mTargetIds.empty() && mTargets.empty() && mTargetTypes.empty() && mTargetNames.empty()){
        return true;
    }
    if (contains(mTargetIds, targetId) || contains(mTargets, target)){
        return true;
    }
    if (!mTargetNames.empty()){
        const std::string& tn = target->getTransitionName();
        if (!tn.empty() && contains(mTargetNames, tn)){
            return true;
        }
    }
    if (!mTargetTypes.empty()){
        const std::type_index tid = std::type_index(typeid(*target));
        for (const std::type_index& type : mTargetTypes){
            if (type == tid){ // android: type.isInstance(target) — exact match (subclass limitation)
                return true;
            }
        }
    }
    return false;
}

ArrayMap<Animator*, Transition::AnimationInfo>& Transition::getRunningAnimators(){
    // android: ThreadLocal<ArrayMap>. CDROID runs transitions on the UI thread, so a
    // process-local static is equivalent.
    static ArrayMap<Animator*, AnimationInfo> runningAnimators;
    return runningAnimators;
}

void Transition::runAnimators(){
    if (DBG){ LOGD("runAnimators() on %s", toString().c_str()); }
    start();
    ArrayMap<Animator*, AnimationInfo>& runningAnimators = getRunningAnimators();
    // Now start every Animator previously created for this transition
    for (Animator* anim : mAnimators){
        if (DBG){ LOGD("  anim: %p", (void*)anim); }
        if (runningAnimators.containsKey(anim)){
            start();
            runAnimator(anim);
        }
    }
    mAnimators.clear();
    end();
}

void Transition::runAnimator(Animator* animator){
    if (animator != nullptr){
        Animator::AnimatorListener listener;
        listener.onAnimationStart = [this](Animator& animation, bool){
            mCurrentAnimators.push_back(&animation);
        };
        listener.onAnimationEnd = [this](Animator& animation, bool){
            getRunningAnimators().remove(&animation);
            auto it = std::find(mCurrentAnimators.begin(), mCurrentAnimators.end(), &animation);
            if (it != mCurrentAnimators.end()){
                mCurrentAnimators.erase(it);
            }
        };
        animator->addListener(listener);
        animate(animator);
    }
}

// ---- capture ----
void Transition::captureValues(ViewGroup* sceneRoot, bool start){
    clearValues(start);
    if ((!mTargetIds.empty() || !mTargets.empty())
            && mTargetNames.empty() && mTargetTypes.empty()){
        for (int id : mTargetIds){
            View* view = sceneRoot->findViewById(id);
            if (view != nullptr){
                TransitionValuesPtr values = std::make_shared<TransitionValues>(view);
                if (start){
                    captureStartValues(*values);
                } else {
                    captureEndValues(*values);
                }
                values->targetedTransitions.push_back(this);
                capturePropagationValues(*values);
                addViewValues(start ? mStartValues : mEndValues, view, values);
            }
        }
        for (View* view : mTargets){
            TransitionValuesPtr values = std::make_shared<TransitionValues>(view);
            if (start){
                captureStartValues(*values);
            } else {
                captureEndValues(*values);
            }
            values->targetedTransitions.push_back(this);
            capturePropagationValues(*values);
            addViewValues(start ? mStartValues : mEndValues, view, values);
        }
    } else {
        captureHierarchy(sceneRoot, start);
    }
    if (!start && !mNameOverrides.isEmpty()){
        int numOverrides = mNameOverrides.size();
        std::vector<View*> overriddenViews;
        overriddenViews.reserve(numOverrides);
        for (int i = 0; i < numOverrides; i++){
            const std::string& fromName = mNameOverrides.keyAt(i);
            int idx = mStartValues.nameValues.indexOfKey(fromName);
            View* v = (idx >= 0) ? mStartValues.nameValues.valueAt(idx) : nullptr;
            overriddenViews.push_back(v);
            if (idx >= 0){
                mStartValues.nameValues.removeAt(idx);
            }
        }
        for (int i = 0; i < numOverrides; i++){
            View* view = overriddenViews[i];
            if (view != nullptr){
                mStartValues.nameValues.put(mNameOverrides.valueAt(i), view);
            }
        }
    }
}

void Transition::addViewValues(TransitionValuesMaps& transitionValuesMaps,
        View* view, const TransitionValuesPtr& transitionValues){
    transitionValuesMaps.viewValues.put(view, transitionValues);
    int id = view->getId();
    if (id >= 0){
        if (transitionValuesMaps.idValues.indexOfKey(id) >= 0){
            // Duplicate IDs cannot match by ID.
            transitionValuesMaps.idValues.put(id, nullptr);
        } else {
            transitionValuesMaps.idValues.put(id, view);
        }
    }
    const std::string& name = view->getTransitionName();
    if (!name.empty()){
        if (transitionValuesMaps.nameValues.containsKey(name)){
            // Duplicate transitionNames: cannot match by transitionName.
            transitionValuesMaps.nameValues.put(name, nullptr);
        } else {
            transitionValuesMaps.nameValues.put(name, view);
        }
    }
    ListView* listview = dynamic_cast<ListView*>(view->getParent());
    if (listview != nullptr){
        Adapter* adapter = listview->getAdapter();
        if (adapter != nullptr && adapter->hasStableIds()){
            int position = listview->getPositionForView(view);
            int64_t itemId = listview->getItemIdAtPosition(position);
            if (transitionValuesMaps.itemIdValues.indexOfKey(itemId) >= 0){
                // Duplicate item IDs: cannot match by item ID.
                View* alreadyMatched = transitionValuesMaps.itemIdValues.get(itemId);
                if (alreadyMatched != nullptr){
                    alreadyMatched->setHasTransientState(false);
                    transitionValuesMaps.itemIdValues.put(itemId, nullptr);
                }
            } else {
                view->setHasTransientState(true);
                transitionValuesMaps.itemIdValues.put(itemId, view);
            }
        }
    }
}

void Transition::clearValues(bool start){
    if (start){
        mStartValues.viewValues.clear();
        mStartValues.idValues.clear();
        mStartValues.itemIdValues.clear();
        mStartValues.nameValues.clear();
        delete mStartValuesList;
        mStartValuesList = nullptr;
    } else {
        mEndValues.viewValues.clear();
        mEndValues.idValues.clear();
        mEndValues.itemIdValues.clear();
        mEndValues.nameValues.clear();
        delete mEndValuesList;
        mEndValuesList = nullptr;
    }
}

void Transition::captureHierarchy(View* view, bool start){
    if (view == nullptr){
        return;
    }
    int id = view->getId();
    if (!mTargetIdExcludes.empty() && contains(mTargetIdExcludes, id)){
        return;
    }
    if (!mTargetExcludes.empty() && contains(mTargetExcludes, view)){
        return;
    }
    if (!mTargetTypeExcludes.empty()){
        const std::type_index tid = std::type_index(typeid(*view));
        for (const std::type_index& type : mTargetTypeExcludes){
            if (type == tid){ // android: isInstance — exact match
                return;
            }
        }
    }
    if (dynamic_cast<ViewGroup*>(view->getParent()) != nullptr){
        TransitionValuesPtr values = std::make_shared<TransitionValues>(view);
        if (start){
            captureStartValues(*values);
        } else {
            captureEndValues(*values);
        }
        values->targetedTransitions.push_back(this);
        capturePropagationValues(*values);
        addViewValues(start ? mStartValues : mEndValues, view, values);
    }
    ViewGroup* parent = dynamic_cast<ViewGroup*>(view);
    if (parent != nullptr){
        // Don't traverse child hierarchy if there are any child-excludes on this view
        if (!mTargetIdChildExcludes.empty() && contains(mTargetIdChildExcludes, id)){
            return;
        }
        if (!mTargetChildExcludes.empty() && contains(mTargetChildExcludes, view)){
            return;
        }
        if (!mTargetTypeChildExcludes.empty()){
            const std::type_index tid = std::type_index(typeid(*view));
            for (const std::type_index& type : mTargetTypeChildExcludes){
                if (type == tid){ // android: isInstance — exact match
                    return;
                }
            }
        }
        for (int i = 0; i < parent->getChildCount(); ++i){
            captureHierarchy(parent->getChildAt(i), start);
        }
    }
}

TransitionValues* Transition::getTransitionValues(View* view, bool start){
    if (mParent != nullptr){
        return mParent->getTransitionValues(view, start);
    }
    TransitionValuesMaps& valuesMaps = start ? mStartValues : mEndValues;
    TransitionValuesPtr* p = valuesMaps.viewValues.get(view);
    return (p && *p) ? p->get() : nullptr;
}

TransitionValues* Transition::getMatchedTransitionValues(View* view, bool viewInStart){
    if (mParent != nullptr){
        return mParent->getMatchedTransitionValues(view, viewInStart);
    }
    std::vector<TransitionValuesPtr>& lookIn = viewInStart ? *mStartValuesList : *mEndValuesList;
    // android returns null when lookIn == null (not yet played). In C++ the lists are
    // only allocated by playTransition; callers must not invoke before that.
    int count = (int)lookIn.size();
    int index = -1;
    for (int i = 0; i < count; i++){
        const TransitionValuesPtr& values = lookIn[i];
        if (!values){
            // Null values are always added to the end of the list, so stop now.
            break;
        }
        if (values->view == view){
            index = i;
            break;
        }
    }
    if (index >= 0){
        std::vector<TransitionValuesPtr>& matchIn = viewInStart ? *mEndValuesList : *mStartValuesList;
        return matchIn[index].get();
    }
    return nullptr;
}

void Transition::pause(View* sceneRoot){
    if (!mEnded){
        ArrayMap<Animator*, AnimationInfo>& runningAnimators = getRunningAnimators();
        int numOldAnims = runningAnimators.size();
        if (sceneRoot != nullptr){
            void* windowId = sceneRoot->getWindowId();
            for (int i = numOldAnims - 1; i >= 0; i--){
                AnimationInfo* info = runningAnimators.valueAtPtr(i);
                if (info && info->view != nullptr && windowId != nullptr && windowId == info->windowId){
                    Animator* anim = runningAnimators.keyAt(i);
                    anim->pause();
                }
            }
        }
        if (!mListeners.empty()){
            std::vector<TransitionListener*> tmpListeners = mListeners; // snapshot
            int numListeners = (int)tmpListeners.size();
            for (int i = 0; i < numListeners; ++i){
                tmpListeners[i]->onTransitionPause(*this);
            }
        }
        mPaused = true;
    }
}

void Transition::resume(View* sceneRoot){
    if (mPaused){
        if (!mEnded){
            ArrayMap<Animator*, AnimationInfo>& runningAnimators = getRunningAnimators();
            int numOldAnims = runningAnimators.size();
            void* windowId = sceneRoot->getWindowId();
            for (int i = numOldAnims - 1; i >= 0; i--){
                AnimationInfo* info = runningAnimators.valueAtPtr(i);
                if (info && info->view != nullptr && windowId != nullptr && windowId == info->windowId){
                    Animator* anim = runningAnimators.keyAt(i);
                    anim->resume();
                }
            }
            if (!mListeners.empty()){
                std::vector<TransitionListener*> tmpListeners = mListeners; // snapshot
                int numListeners = (int)tmpListeners.size();
                for (int i = 0; i < numListeners; ++i){
                    tmpListeners[i]->onTransitionResume(*this);
                }
            }
        }
        mPaused = false;
    }
}

void Transition::playTransition(ViewGroup* sceneRoot){
    mStartValuesList = new std::vector<TransitionValuesPtr>();
    mEndValuesList = new std::vector<TransitionValuesPtr>();
    matchStartAndEnd(mStartValues, mEndValues);

    ArrayMap<Animator*, AnimationInfo>& runningAnimators = getRunningAnimators();
    int numOldAnims = runningAnimators.size();
    void* windowId = sceneRoot->getWindowId();
    for (int i = numOldAnims - 1; i >= 0; i--){
        Animator* anim = runningAnimators.keyAt(i);
        if (anim != nullptr){
            AnimationInfo* oldInfo = runningAnimators.valueAtPtr(i);
            if (oldInfo && oldInfo->view != nullptr && oldInfo->windowId == windowId){
                TransitionValuesPtr oldValues = oldInfo->values;
                View* oldView = oldInfo->view;
                TransitionValues* startValues = getTransitionValues(oldView, true);
                TransitionValues* endValues = getMatchedTransitionValues(oldView, true);
                if (startValues == nullptr && endValues == nullptr){
                    TransitionValuesPtr* ev = mEndValues.viewValues.get(oldView);
                    endValues = (ev && *ev) ? ev->get() : nullptr;
                }
                bool cancel = (startValues != nullptr || endValues != nullptr) &&
                        oldInfo->transition->isTransitionRequired(oldValues.get(), endValues);
                if (cancel){
                    if (anim->isRunning() || anim->isStarted()){
                        if (DBG){ LOGD("Canceling anim %p", (void*)anim); }
                        anim->cancel();
                    } else {
                        if (DBG){ LOGD("removing anim from info list: %p", (void*)anim); }
                        runningAnimators.remove(anim);
                    }
                }
            }
        }
    }

    createAnimators(sceneRoot, mStartValues, mEndValues, *mStartValuesList, *mEndValuesList);
    runAnimators();
}

bool Transition::isTransitionRequired(TransitionValues* startValues, TransitionValues* endValues){
    bool valuesChanged = false;
    // if startValues null, transition didn't care to stash values, won't get canceled
    if (startValues != nullptr && endValues != nullptr){
        std::vector<std::string> properties = getTransitionProperties();
        if (!properties.empty()){
            int count = (int)properties.size();
            for (int i = 0; i < count; i++){
                if (isValueChanged(*startValues, *endValues, properties[i])){
                    valuesChanged = true;
                    break;
                }
            }
        } else {
            for (const auto& kv : startValues->values){
                if (isValueChanged(*startValues, *endValues, kv.first)){
                    valuesChanged = true;
                    break;
                }
            }
        }
    }
    return valuesChanged;
}

bool Transition::isValueChanged(const TransitionValues& oldValues,
        const TransitionValues& newValues, const std::string& key){
    bool oldHas = oldValues.values.count(key) > 0;
    bool newHas = newValues.values.count(key) > 0;
    // Android: compare ONLY when BOTH old and new captured the key. If either side is
    // missing it the transition didn't track it there, so treat as no change. The former
    // `if (oldHas != newHas) return false` only covered the exactly-one-has case, so the
    // BOTH-missing case fell through to values.at(key) and threw std::out_of_range
    // (hit by ChangeBounds whose property set includes keys a given view didn't capture).
    if (!(oldHas && newHas)){
        return false;
    }
    const nonstd::any& oldValue = oldValues.values.at(key);
    const nonstd::any& newValue = newValues.values.at(key);
    bool changed;
    if (!oldValue.has_value() && !newValue.has_value()){
        changed = false;
    } else if (!oldValue.has_value() || !newValue.has_value()){
        changed = true;
    } else {
        // neither empty — compare via TransitionValues::anyEquals semantics
        changed = !anyValuesEqual(oldValue, newValue);
    }
    return changed;
}

void Transition::animate(Animator* animator){
    if (animator == nullptr){
        end();
    } else {
        if (getDuration() >= 0){
            animator->setDuration(getDuration());
        }
        if (getStartDelay() >= 0){
            animator->setStartDelay(getStartDelay() + animator->getStartDelay());
        }
        if (getInterpolator() != nullptr){
            animator->setInterpolator(getInterpolator());
        }
        Animator::AnimatorListener listener;
        listener.onAnimationEnd = [this](Animator&, bool){
            end();
            // android: animation.removeListener(this). CDROID copies the listener into
            // the Animator, so self-removal by identity is not possible; the animation
            // is ending, so leaving the one-shot listener is harmless.
        };
        animator->addListener(listener);
        animator->start();
    }
}

void Transition::start(){
    if (mNumInstances == 0){
        if (!mListeners.empty()){
            std::vector<TransitionListener*> tmpListeners = mListeners; // snapshot
            int numListeners = (int)tmpListeners.size();
            for (int i = 0; i < numListeners; ++i){
                tmpListeners[i]->onTransitionStart(*this);
            }
        }
        mEnded = false;
    }
    mNumInstances++;
}

void Transition::end(){
    --mNumInstances;
    if (mNumInstances == 0){
        if (!mListeners.empty()){
            std::vector<TransitionListener*> tmpListeners = mListeners; // snapshot
            int numListeners = (int)tmpListeners.size();
            for (int i = 0; i < numListeners; ++i){
                tmpListeners[i]->onTransitionEnd(*this);
            }
        }
        for (size_t i = 0; i < mStartValues.itemIdValues.size(); ++i){
            View* view = mStartValues.itemIdValues.valueAt(i);
            if (view != nullptr){
                view->setHasTransientState(false);
            }
        }
        for (size_t i = 0; i < mEndValues.itemIdValues.size(); ++i){
            View* view = mEndValues.itemIdValues.valueAt(i);
            if (view != nullptr){
                view->setHasTransientState(false);
            }
        }
        mEnded = true;
    }
}

void Transition::forceToEnd(ViewGroup* sceneRoot){
    ArrayMap<Animator*, AnimationInfo>& runningAnimators = getRunningAnimators();
    int numOldAnims = runningAnimators.size();
    if (sceneRoot == nullptr || numOldAnims == 0){
        return;
    }
    void* windowId = sceneRoot->getWindowId();
    ArrayMap<Animator*, AnimationInfo> oldAnimators(runningAnimators);
    runningAnimators.clear();
    for (int i = numOldAnims - 1; i >= 0; i--){
        AnimationInfo* info = oldAnimators.valueAtPtr(i);
        if (info && info->view != nullptr && windowId != nullptr && windowId == info->windowId){
            Animator* anim = oldAnimators.keyAt(i);
            anim->end();
        }
    }
}

void Transition::cancel(){
    int numAnimators = (int)mCurrentAnimators.size();
    for (int i = numAnimators - 1; i >= 0; i--){
        Animator* animator = mCurrentAnimators[i];
        animator->cancel();
    }
    if (!mListeners.empty()){
        std::vector<TransitionListener*> tmpListeners = mListeners; // snapshot
        int numListeners = (int)tmpListeners.size();
        for (int i = 0; i < numListeners; ++i){
            tmpListeners[i]->onTransitionCancel(*this);
        }
    }
}

Transition& Transition::addListener(TransitionListener* listener){
    mListeners.push_back(listener);
    return *this;
}

Transition& Transition::removeListener(TransitionListener* listener){
    auto it = std::find(mListeners.begin(), mListeners.end(), listener);
    if (it != mListeners.end()){
        mListeners.erase(it);
    }
    return *this;
}

// ---- epicenter / path motion / propagation ----
void Transition::setEpicenterCallback(EpicenterCallback* epicenterCallback){
    mEpicenterCallback = epicenterCallback;
}
Transition::EpicenterCallback* Transition::getEpicenterCallback() const{
    return mEpicenterCallback;
}
Rect Transition::getEpicenter() const{
    // android returns null when no callback. C++ Rect has no null; callers wanting the
    // null semantics should check getEpicenterCallback() == nullptr first.
    if (mEpicenterCallback == nullptr){
        return Rect();
    }
    return mEpicenterCallback->onGetEpicenter(*const_cast<Transition*>(this));
}
void Transition::setPathMotion(PathMotion* pathMotion){
    mPathMotion = (pathMotion == nullptr) ? straightPathMotion() : pathMotion;
}
PathMotion* Transition::getPathMotion() const{
    return mPathMotion;
}
void Transition::setPropagation(TransitionPropagation* transitionPropagation){
    mPropagation = transitionPropagation;
}
TransitionPropagation* Transition::getPropagation() const{
    return mPropagation;
}

void Transition::capturePropagationValues(TransitionValues& transitionValues){
    if (mPropagation != nullptr && !transitionValues.values.empty()){
        std::vector<std::string> propertyNames = mPropagation->getPropagationProperties();
        if (propertyNames.empty()){
            return;
        }
        bool containsAll = true;
        for (const std::string& name : propertyNames){
            if (transitionValues.values.count(name) == 0){
                containsAll = false;
                break;
            }
        }
        if (!containsAll){
            mPropagation->captureValues(&transitionValues);
        }
    }
}

// ---- engine hooks ----
Transition* Transition::setSceneRoot(ViewGroup* sceneRoot){
    mSceneRoot = sceneRoot;
    return this;
}
void Transition::setCanRemoveViews(bool canRemoveViews){
    mCanRemoveViews = canRemoveViews;
}
bool Transition::canRemoveViews() const{
    return mCanRemoveViews;
}
void Transition::setNameOverrides(const ArrayMap<std::string, std::string>& overrides){
    mNameOverrides = overrides;
}
ArrayMap<std::string, std::string>& Transition::getNameOverrides(){
    return mNameOverrides;
}

// ---- targets ----
Transition& Transition::addTarget(int targetId){
    if (targetId > 0){
        mTargetIds.push_back(targetId);
    }
    return *this;
}
Transition& Transition::addTarget(const std::string& targetName){
    if (!targetName.empty()){
        mTargetNames.push_back(targetName);
    }
    return *this;
}
Transition& Transition::addTarget(const std::type_index& targetType){
    mTargetTypes.push_back(targetType);
    return *this;
}
Transition& Transition::addTarget(View* target){
    mTargets.push_back(target);
    return *this;
}
Transition& Transition::removeTarget(int targetId){
    if (targetId > 0){
        auto it = std::find(mTargetIds.begin(), mTargetIds.end(), targetId);
        if (it != mTargetIds.end()) mTargetIds.erase(it);
    }
    return *this;
}
Transition& Transition::removeTarget(const std::string& targetName){
    if (!targetName.empty()){
        auto it = std::find(mTargetNames.begin(), mTargetNames.end(), targetName);
        if (it != mTargetNames.end()) mTargetNames.erase(it);
    }
    return *this;
}
Transition& Transition::removeTarget(const std::type_index& targetType){
    auto it = std::find(mTargetTypes.begin(), mTargetTypes.end(), targetType);
    if (it != mTargetTypes.end()) mTargetTypes.erase(it);
    return *this;
}
Transition& Transition::removeTarget(View* target){
    if (target != nullptr){
        auto it = std::find(mTargets.begin(), mTargets.end(), target);
        if (it != mTargets.end()) mTargets.erase(it);
    }
    return *this;
}

Transition& Transition::excludeTarget(int targetId, bool exclude){
    if (targetId >= 0){
        excludeObject(mTargetIdExcludes, targetId, exclude);
    }
    return *this;
}
Transition& Transition::excludeTarget(const std::string& targetName, bool exclude){
    excludeObject(mTargetNameExcludes, targetName, exclude);
    return *this;
}
Transition& Transition::excludeTarget(const std::type_index& targetType, bool exclude){
    excludeObject(mTargetTypeExcludes, targetType, exclude);
    return *this;
}
Transition& Transition::excludeTarget(View* target, bool exclude){
    excludeObject(mTargetExcludes, target, exclude);
    return *this;
}
Transition& Transition::excludeChildren(int targetId, bool exclude){
    if (targetId >= 0){
        excludeObject(mTargetIdChildExcludes, targetId, exclude);
    }
    return *this;
}
Transition& Transition::excludeChildren(const std::type_index& targetType, bool exclude){
    excludeObject(mTargetTypeChildExcludes, targetType, exclude);
    return *this;
}
Transition& Transition::excludeChildren(View* target, bool exclude){
    excludeObject(mTargetChildExcludes, target, exclude);
    return *this;
}

const std::vector<int>& Transition::getTargetIds() const{ return mTargetIds; }
const std::vector<View*>& Transition::getTargets() const{ return mTargets; }
const std::vector<std::string>& Transition::getTargetNames() const{ return mTargetNames; }
const std::vector<std::type_index>& Transition::getTargetTypes() const{ return mTargetTypes; }
const std::vector<std::string>& Transition::getTargetViewNames() const{ return mTargetNames; }

// ---- name / toString / clone ----
std::string Transition::getName() const{
    return mName;
}

std::string Transition::toString(){
    return toString("");
}

std::string Transition::toString(const std::string& indent){
    std::ostringstream oss;
    oss << indent << mName << "@" << std::hex << reinterpret_cast<std::intptr_t>(this) << ": ";
    if (mDuration != -1){
        oss << "dur(" << std::dec << mDuration << ") ";
    }
    if (mStartDelay != -1){
        oss << "dly(" << mStartDelay << ") ";
    }
    if (mInterpolator != nullptr){
        oss << "interp(" << (void*)mInterpolator << ") ";
    }
    if (!mTargetIds.empty() || !mTargets.empty()){
        oss << "tgts(";
        for (size_t i = 0; i < mTargetIds.size(); ++i){
            if (i > 0) oss << ", ";
            oss << mTargetIds[i];
        }
        for (size_t i = 0; i < mTargets.size(); ++i){
            if (i > 0) oss << ", ";
            oss << (void*)mTargets[i];
        }
        oss << ")";
    }
    return oss.str();
}

void Transition::copyCloneFields(Transition* clone) const{
    // android super.clone() is a shallow copy; then these fields are reset so the clone
    // does not share the original's animator/values state. (Does NOT delete the copied
    // mStartValuesList/mEndValuesList pointers — the original still owns them.)
    clone->mAnimators.clear();
    clone->mStartValues = TransitionValuesMaps();
    clone->mEndValues = TransitionValuesMaps();
    clone->mStartValuesList = nullptr;
    clone->mEndValuesList = nullptr;
}

Transition* Transition::clone() const{
    // Transition is abstract; concrete subclasses override clone() as
    //   Derived* c = new Derived(*this); copyCloneFields(c); return c;
    // This base implementation cannot instantiate the runtime type.
    LOGE("Transition::clone() called on abstract base; subclass must override");
    return nullptr;
}

} // namespace cdroid
