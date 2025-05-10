#ifndef __LEVELLIST_DRAWABLE_H__
#define __LEVELLIST_DRAWABLE_H__
#include <drawables/drawablecontainer.h>
namespace cdroid{
class LevelListDrawable:public DrawableContainer{
private:
    class LevelListState:public DrawableContainerState{
    public:
        std::vector<int>mLows;
        std::vector<int>mHighs;
        LevelListState(const LevelListState*orig,LevelListDrawable*own);
        LevelListState(const LevelListState&state);
        void mutate()override;
        void addLevel(int low,int high,Drawable*drawable);
        int indexOfLevel(int level)const;
        LevelListDrawable*newDrawable()override;
    };
    bool mMutated;
    std::shared_ptr<LevelListState>mLevelListState;
    LevelListDrawable(std::shared_ptr<LevelListState>state);
private:
    void inflateChildElements(XmlPullParser& parser,const AttributeSet& atts);
protected:
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableContainerState> cloneConstantState()override;
    void setConstantState(std::shared_ptr<DrawableContainerState> state)override;
public:
    LevelListDrawable();
    void addLevel(int low, int high, Drawable* drawable);
    LevelListDrawable* mutate()override;
    void clearMutated()override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};
}//namespace
#endif

