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
    std::vector<RecyclerView::ViewHolder*> mPendingRemovals;// = new ArrayList<>();
    std::vector<RecyclerView::ViewHolder*> mPendingAdditions;// = new ArrayList<>();
    std::vector<MoveInfo*> mPendingMoves;// = new ArrayList<>();
    std::vector<ChangeInfo*> mPendingChanges;// = new ArrayList<>();
protected:
    std::vector<std::vector<RecyclerView::ViewHolder*>> mAdditionsList;// = new ArrayList<>();
    std::vector<std::vector<MoveInfo*>> mMovesList;// = new ArrayList<>();
    std::vector<std::vector<ChangeInfo*>> mChangesList;// = new ArrayList<>();

    std::vector<RecyclerView::ViewHolder*> mAddAnimations;// = new ArrayList<>();
    std::vector<RecyclerView::ViewHolder*> mMoveAnimations;// = new ArrayList<>();
    std::vector<RecyclerView::ViewHolder*> mRemoveAnimations;// = new ArrayList<>();
    std::vector<RecyclerView::ViewHolder*> mChangeAnimations;// = new ArrayList<>();

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
