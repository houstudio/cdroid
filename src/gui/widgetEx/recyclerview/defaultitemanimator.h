#ifndef __DEFAULT_ITEM_ANIMATOR_H__
#define __DEFAULT_ITEM_ANIMATOR_H__
#include <widgetEx/recyclerview/simpleitemanimator.h>
namespace cdroid{
class DefaultItemAnimator:public SimpleItemAnimator {
private:
    static constexpr bool _DEBUG = true;
    static TimeInterpolator* sDefaultInterpolator;
protected:
    class MoveInfo;
    class ChangeInfo;
private:
    std::vector<RecyclerView::ViewHolder*> mPendingRemovals;
    std::vector<RecyclerView::ViewHolder*> mPendingAdditions;
    std::vector<MoveInfo*> mPendingMoves;
    std::vector<ChangeInfo*> mPendingChanges;
protected:
    std::vector<std::vector<RecyclerView::ViewHolder*>*> mAdditionsList;
    std::vector<std::vector<MoveInfo*>*> mMovesList;
    std::vector<std::vector<ChangeInfo*>*> mChangesList;

    std::vector<RecyclerView::ViewHolder*> mAddAnimations;
    std::vector<RecyclerView::ViewHolder*> mMoveAnimations;
    std::vector<RecyclerView::ViewHolder*> mRemoveAnimations;
    std::vector<RecyclerView::ViewHolder*> mChangeAnimations;

    class MoveInfo {
    public:
        RecyclerView::ViewHolder* holder;
        int fromX, fromY, toX, toY;
    public:
        MoveInfo(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY);
    };

    class ChangeInfo {
    public:
        RecyclerView::ViewHolder* oldHolder, *newHolder;
        int fromX, fromY, toX, toY;
    private:
       	ChangeInfo(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder);
    public:
        ChangeInfo(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
                int fromX, int fromY, int toX, int toY);
    };
protected:
    void onAddAnimationStart(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onAddAnimationCancel(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onAddAnimationEnd(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onRemoveAnimationStart(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onRemoveAnimationEnd(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onMoveAnimationStart(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onMoveAnimationCancel(int deltaX,int deltaY,RecyclerView::ViewHolder*,Animator& animator);
    void onMoveAnimationEnd(RecyclerView::ViewHolder*,Animator& animator,bool isReverse);
    void onChangeAnimationStart(bool,ChangeInfo*,Animator& animator,bool isReverse);
    void onChangeAnimationEnd(bool,ChangeInfo*,Animator& animator,bool isReverse);
protected:
    void animateRemoveImpl(RecyclerView::ViewHolder& holder);
    void endChangeAnimation(std::vector<ChangeInfo*>& infoList, RecyclerView::ViewHolder& item);
    void endChangeAnimationIfNecessary(ChangeInfo& changeInfo);
    bool endChangeAnimationIfNecessary(ChangeInfo& changeInfo, RecyclerView::ViewHolder& item);
    void resetAnimation(RecyclerView::ViewHolder& holder);
protected:
    void animateAddImpl(RecyclerView::ViewHolder& holder);
    void animateMoveImpl(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY);
    void animateChangeImpl(ChangeInfo& changeInfo);
    void dispatchFinishedWhenDone();
    void cancelAll(std::vector<RecyclerView::ViewHolder*>& viewHolders);
public:
    void runPendingAnimations()override;
    bool animateRemove(RecyclerView::ViewHolder& holder)override;
    bool animateAdd(RecyclerView::ViewHolder& holder)override;
    bool animateMove(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY)override;
    bool animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
            int fromX, int fromY, int toX, int toY)override;
    void endAnimation(RecyclerView::ViewHolder& item)override;
    bool isRunning()override;
    void endAnimations()override;
    bool canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder,std::vector<Object*>& payloads)override;
};
}
#endif/*__DEFAULT_ITEM_ANIMATOR_H__*/
