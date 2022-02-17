#ifndef __STATELIST_DRAWABLE_H__
#define __STATELIST_DRAWABLE_H__
#include <drawables/drawablecontainer.h>
namespace cdroid{

class StateListDrawable:public DrawableContainer{
protected:
    class StateListState:public DrawableContainerState{
    public:
        std::vector<std::vector<int>>mStateSets;
        StateListState(const StateListState*orig,StateListDrawable*own);
        void mutate();
        Drawable*newDrawable()override;
        int addStateSet(const std::vector<int>&stateSet, Drawable*drawable);
        int indexOfStateSet(const std::vector<int>&stateSet);
        bool hasFocusStateSpecified()const;
    };
private:
    std::shared_ptr<StateListState>mStateListState;
protected:
    StateListDrawable(std::shared_ptr<StateListState>state);
    int indexOfStateSet(const std::vector<int>&states)const;
    bool onStateChange(const std::vector<int>&stateSet)override;
    std::shared_ptr<DrawableContainerState>cloneConstantState()override;
    void setConstantState(std::shared_ptr<DrawableContainerState>state)override;
public:
    StateListDrawable();
    void addState(const std::vector<int>&stateSet,Drawable*drawable);
    bool isStateful()const override{return true;}
    bool hasFocusStateSpecified()const override;
    Drawable*mutate()override;
    void clearMutated()override;
    int getStateCount()const;
    const std::vector<int>&getStateSet(int idx)const;
    Drawable*getStateDrawable(int index);
    int getStateDrawableIndex(const std::vector<int>&state)const;
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}
#endif
