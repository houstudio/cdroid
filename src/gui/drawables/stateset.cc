#include <drawables/stateset.h>
#include <bitset.h>
namespace cdroid{

const std::vector<int>StateSet::NOTHING={0};
const std::vector<int>StateSet::WILD_CARD={};

std::vector<int>StateSet::VIEW_STATE_IDS={
    WINDOW_FOCUSED , VIEW_STATE_WINDOW_FOCUSED,
    SELECTED       , VIEW_STATE_SELECTED ,
    FOCUSED        , VIEW_STATE_FOCUSED  ,
    ENABLED        , VIEW_STATE_ENABLED  ,
    PRESSED        , VIEW_STATE_PRESSED  ,
    ACTIVATED      , VIEW_STATE_ACTIVATED,
    HOVERED        , VIEW_STATE_HOVERED  ,
    CHECKED        , VIEW_STATE_CHECKED  ,
    CHECKABLE      , VIEW_STATE_CHECKABLE,
    DRAG_ACCPETABLE, VIEW_STATE_DRAG_CAN_ACCEPT,
    DRAG_HOVERED   , VIEW_STATE_DRAG_HOVERED
};

std::vector<std::vector<int>>StateSet::VIEW_STATE_SETS;

void StateSet::trimStateSet(std::vector<int>&states,int newsize){
    states.resize(newsize);
}

void StateSet::createStates(){
    const int NUM_BITS=10;

    std::vector<int>orderedIds;
    orderedIds.resize(VIEW_STATE_IDS.size());
    for (int i = 0; i < VIEW_STATE_IDS.size()/2;i++){//R.styleable.ViewDrawableStates.length; i++) {
        int viewState = i+1;//R.styleable.ViewDrawableStates[i];STATE is codedfrom WINDOW_FOCUSED:1-->DRAG_HOVERED:10
        for (int j = 0; j < VIEW_STATE_IDS.size(); j += 2) {
            if (VIEW_STATE_IDS[j] == viewState) {
                orderedIds[i * 2] = viewState;
                orderedIds[i * 2 + 1] = VIEW_STATE_IDS[j + 1];
            }
        }
    }
    VIEW_STATE_SETS.resize(1<<NUM_BITS);
    for (int i = 0; i < VIEW_STATE_SETS.size(); i++) {
        std::vector<int>set;
        for (int j = 0; j < orderedIds.size(); j += 2) {
            if ((i & orderedIds[j + 1]) != 0) {
                set.push_back(orderedIds[j]);
            }
        }
        VIEW_STATE_SETS[i] = set;
    }
}

std::vector<int>& StateSet::get(int mask){
    if(VIEW_STATE_SETS.size()==0)
         createStates();
    return VIEW_STATE_SETS[mask]; 
}

bool StateSet::isWildCard(const std::vector<int>&stateSetOrSpec){
    return stateSetOrSpec.size() == 0 || stateSetOrSpec[0] == 0;
}

bool StateSet::stateSetMatches(const std::vector<int>&stateSpec,const std::vector<int>&stateSet){
    
    if (stateSet.size()==0)
        return stateSpec.size()==0 || isWildCard(stateSpec);

    const int stateSpecSize=stateSpec.size();
    const int stateSetSize=stateSet.size();
    for (int i = 0; i < stateSpecSize; i++) {
        int stateSpecState = stateSpec[i];
        if (stateSpecState == 0) { // We've reached the end of the cases to match against.
            return true;
        }
        const bool mustMatch=(stateSpecState > 0);
        if (stateSpecState<0) { // We use negative values to indicate must-NOT-match states.
            stateSpecState = -stateSpecState;
        }
        bool found = false;
        for (int j = 0; j < stateSetSize; j++) {
            const int state = stateSet[j];
            if (state == 0) {  // We've reached the end of states to match.
                if (mustMatch){// We didn't find this must-match state.
                    return false;
                } else { //Continue checking other must-not-match states.
                    break;
                }
            }
            if (state == stateSpecState) {
                if (mustMatch) {// Continue checking other other must-match states.
                    found = true;
                    break;
                } else { // Any match of a must-not-match state returns false.
                    return false;
                }
            }
        }
        if (mustMatch && !found) {
            // We've reached the end of states to match and we didn't
            // find a must-match state.
            return false;
        }
    }
    return true;
}

bool StateSet::stateSetMatches(const std::vector<int>&stateSpec,int state){
    const int stateSpecSize=stateSpec.size();
    for (int i = 0; i < stateSpecSize; i++) {
        int stateSpecState = stateSpec[i];
        if (stateSpecState == 0)// We've reached the end of the cases to match against.
            return true;

        if (stateSpecState > 0) {
            if(state != stateSpecState) return false;
        }else{// We use negative values to indicate must-NOT-match states.
            if(state == -stateSpecState)// We matched a must-not-match case.
                return false;
        }
    }
    return true;
}

bool StateSet::containsAttribute(const std::vector<std::vector<int>>&stateSpecs,int attr){
    for (auto spec : stateSpecs) {
        if (spec.empty())  break;
        for (int specAttr : spec) {
            if ( (specAttr == attr) || (-specAttr == attr) )
                return true;
        }
    }
    return false;    
}

void StateSet::appendState(std::vector<int>&states,const std::string&s,int value){
    if(s.empty())return;
    states.push_back(s.compare("true")?-value:value);
}

int StateSet::parseState(std::vector<int>&states,const AttributeSet&atts){
    appendState(states,atts.getString("state_enabled") , ENABLED );
    appendState(states,atts.getString("state_focused") , FOCUSED );
    appendState(states,atts.getString("state_selected"), SELECTED);
    appendState(states,atts.getString("state_checked") , CHECKED );
    appendState(states,atts.getString("state_checkable"),CHECKABLE);
    appendState(states,atts.getString("state_pressed") , PRESSED );
    appendState(states,atts.getString("state_hovered") , HOVERED );
    return states.size();
}

}
