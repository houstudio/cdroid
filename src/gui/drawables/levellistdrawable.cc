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
void LevelListDrawable::inflate(XmlPullParser& parser,const AttributeSet& atts){
    DrawableContainer::inflate(parser,atts);
    inflateChildElements(parser,atts);
}

void LevelListDrawable::inflateChildElements(XmlPullParser& parser,const AttributeSet& atts){
    int type,depth,low = 0;
    const int innerDepth = parser.getDepth();
    XmlPullParser::XmlEvent event;
    while (((type = parser.next(event,depth)) != XmlPullParser::END_DOCUMENT)
            && (depth >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if ((depth > innerDepth) || event.name.compare("item")) {
            continue;
        }
        AttributeSet& a = event.attributes;
        low = a.getInt("minLevel", 0);
        int high = a.getInt("maxLevel", 0);
        Drawable*dr = a.getDrawable("drawable");

        if (high < 0) {
            throw std::logic_error(//parser.getPositionDescription()
                ": <item> tag requires a 'maxLevel' attribute");
        }

        if (dr==nullptr) {
            while ((type = parser.next(event,depth)) == XmlPullParser::TEXT) {}
            if (type != XmlPullParser::START_TAG) {
                throw std::logic_error(
                                ": <item> tag requires a 'drawable' attribute or "
                                "child tag defining a drawable");
                //parser.getPositionDescription()
            }
            dr = Drawable::createFromXmlInner(parser,a);
        }
        mLevelListState->addLevel(low, high, dr);
    }

    onLevelChange(getLevel());
}

}
