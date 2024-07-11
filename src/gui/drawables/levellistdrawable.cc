#include <drawables/levellistdrawable.h>
#include <cdlog.h>


namespace cdroid{

LevelListDrawable::LevelListState::LevelListState(const LevelListState*orig,LevelListDrawable*own)
    :DrawableContainerState(orig,own){
    if(orig!=nullptr){
        mLows = orig->mLows;
        mHighs= orig->mHighs;
    }
}

void LevelListDrawable::LevelListState::mutate(){
    //mLows = mLows.clone();
    //mHighs = mHighs.clone();
}

void LevelListDrawable::LevelListState::addLevel(int low,int high,Drawable*drawable){
    addChild(drawable);
    mLows.push_back(low);
    mHighs.push_back(high);
}

int LevelListDrawable::LevelListState::indexOfLevel(int level)const{
    const int N = getChildCount();
    for (int i = 0; i < N; i++) {
       if (level >= mLows[i] && level <= mHighs[i]) {
           return i;
       }
    }
    return -1;
}

LevelListDrawable*LevelListDrawable::LevelListState::newDrawable(){
    return new LevelListDrawable(std::dynamic_pointer_cast<LevelListState>(shared_from_this()));
}

LevelListDrawable::LevelListDrawable():DrawableContainer(){
    mMutated = false;
    auto state = std::make_shared<LevelListState>(nullptr,this);
    setConstantState(state);
    onLevelChange(getLevel());
}

LevelListDrawable::LevelListDrawable(std::shared_ptr<LevelListState>state){
    auto newState = std::make_shared<LevelListState>(state.get(),this);
    mMutated = false;
    setConstantState(newState);
    onLevelChange(getLevel());
}

bool LevelListDrawable::onLevelChange(int level){
    const int idx = mLevelListState->indexOfLevel(level);
    LOGV("%p level %d.index=%d",this,level,idx);
    if (selectDrawable(idx)) {
        return true;
    }
    return DrawableContainer::onLevelChange(level);
}

std::shared_ptr<DrawableContainer::DrawableContainerState> LevelListDrawable::cloneConstantState(){
    return std::make_shared<LevelListState>(mLevelListState.get(),this);
}

void LevelListDrawable::setConstantState(std::shared_ptr<DrawableContainerState> state){
    DrawableContainer::setConstantState(state);
    mLevelListState = std::dynamic_pointer_cast<LevelListState>(state);
}

LevelListDrawable*LevelListDrawable::mutate(){
    if (!mMutated && (DrawableContainer::mutate() == this)) {
        mLevelListState->mutate();
        mMutated = true;
    }
    return this; 
}

void LevelListDrawable::clearMutated(){
    DrawableContainer::clearMutated();
    mMutated = false;
}

void LevelListDrawable::addLevel(int low,int high,Drawable* drawable) {
    if(drawable){
        mLevelListState->addLevel(low,high,drawable);
        onLevelChange(getLevel());
    }
}

Drawable*LevelListDrawable::inflate(Context*ctx,const AttributeSet&atts){
    return new LevelListDrawable();
}

}
