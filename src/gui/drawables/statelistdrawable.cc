#include <drawables/statelistdrawable.h>
#include <cdtypes.h>
#include <cdlog.h>
namespace cdroid{

StateListDrawable::StateListState::StateListState(const StateListState*orig,StateListDrawable*own)
    :DrawableContainerState(orig,own){
    if(orig){
        mStateSets = orig->mStateSets;
    }
}

Drawable*StateListDrawable::StateListState::newDrawable(){
    return new StateListDrawable(std::dynamic_pointer_cast<StateListState>(shared_from_this()));
}

void StateListDrawable::StateListState::mutate(){
}

int StateListDrawable::StateListState::addStateSet(const std::vector<int>&stateSet, Drawable*drawable){
    const int pos = addChild(drawable);
    mStateSets.push_back(stateSet);
    return pos;
}

int StateListDrawable::StateListState::indexOfStateSet(const std::vector<int>&stateSet){
    const int N = getChildCount();
    for (int i = 0; i < N; i++) {
        if (StateSet::stateSetMatches(mStateSets[i], stateSet)) {
            return i;
        }
    }
    return -1;
}

bool StateListDrawable::StateListState::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateSets,StateSet::FOCUSED);
}

StateListDrawable::StateListDrawable(){
    auto state=std::make_shared<StateListState>(nullptr,this);
    setConstantState(state);
}

StateListDrawable::StateListDrawable(std::shared_ptr<StateListState>state){
    std::shared_ptr<StateListState>newState =std::make_shared<StateListState>(state.get(), this);
    setConstantState(newState);
    onStateChange(getState());
}

std::shared_ptr<DrawableContainer::DrawableContainerState>StateListDrawable::cloneConstantState(){
    return std::make_shared<StateListState>(mStateListState.get(),this);
}

Drawable*StateListDrawable::mutate(){
    if (!mMutated && DrawableContainer::mutate() == this) {
        mStateListState->mutate();
        mMutated = true;
    }
    return this;
}

void StateListDrawable::clearMutated(){
    StateListDrawable::clearMutated();
    mMutated = false;
}

void StateListDrawable::setConstantState(std::shared_ptr<DrawableContainerState>state){
    DrawableContainer::setConstantState(state);
    mStateListState =std::dynamic_pointer_cast<StateListState>(state);
}

int StateListDrawable::indexOfStateSet(const std::vector<int>&stateSet)const{
    for (int i = 0; i < mStateListState->mStateSets.size(); i++) {
        if (StateSet::stateSetMatches(mStateListState->mStateSets[i], stateSet)) {
            return i;
        }
    }
    return -1;
}

void StateListDrawable::addState(const std::vector<int>&stateSet, Drawable* drawable){
    if(drawable){
        mStateListState->addStateSet(stateSet,drawable);
        onStateChange(getState());
    }
}

bool StateListDrawable::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateListState->mStateSets,StateSet::FOCUSED);
}

int StateListDrawable::getStateCount()const{
    return getChildCount();
}

const std::vector<int>& StateListDrawable::getStateSet(int idx)const{
    return mStateListState->mStateSets[idx];
}

Drawable*StateListDrawable::getStateDrawable(int index){
    return getChild(index);
}

int StateListDrawable::getStateDrawableIndex(const std::vector<int>&stateSet)const{
    return indexOfStateSet(stateSet); 
}

bool StateListDrawable::onStateChange(const std::vector<int>&stateSet){
    const bool changed =DrawableContainer::onStateChange(stateSet);
    const int  idx=getStateDrawableIndex(stateSet);
    LOGV("%p set stateIndex=%d/%d ",this,idx,getChildCount(),idx>=0?getChild(idx):nullptr);
    return selectDrawable(idx) || changed;
}

Drawable*StateListDrawable::inflate(Context*ctx,const AttributeSet&atts){
    StateListDrawable*sd=new StateListDrawable();
    //setConstantSize(atts.getBoolean("constantSize"));=["true" | "false"]
    //android:dither=["true" | "false"]
    //android:variablePadding=["true" | "false"] >
    return sd;
}

}
