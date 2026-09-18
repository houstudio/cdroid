#ifndef __DESKCLOCK_ITEMANIMATOR_H__
#define __DESKCLOCK_ITEMANIMATOR_H__
/*********************************************************************************
 * Port of com.android.deskclock.ItemAnimator — alpha add/remove, translation
 * move, and change animations delegated to the view holders' OnAnimateChange
 * listeners. Removes play before changes/moves, which play before additions.
 *********************************************************************************/
#include <unordered_map>
#include <vector>

#include <animation/animator.h>
#include <widgetEx/recyclerview/simpleitemanimator.h>

#include <animatorutils.h>

namespace cdroid {
namespace deskclock {

/** Implemented by view holders that know how to animate their own changes. */
class OnAnimateChangeListener {
public:
    virtual ~OnAnimateChangeListener() = default;

    virtual Animator* onAnimateChange(RecyclerView::ViewHolder& oldHolder,
                                      RecyclerView::ViewHolder& newHolder,
                                      int64_t duration) = 0;

    virtual Animator* onAnimateChange(std::vector<Object*>* payloads,
                                      int fromLeft, int fromTop, int fromRight, int fromBottom,
                                      int64_t duration) = 0;
};

class ItemAnimator : public SimpleItemAnimator {
private:
    /** ItemHolderInfo that carries the change payloads (in-place animations). */
    class PayloadItemHolderInfo : public ItemHolderInfo {
    public:
        std::vector<Object*> payloads;
    };

    std::vector<Animator*> mAddAnimatorsList;
    std::vector<Animator*> mRemoveAnimatorsList;
    std::vector<Animator*> mChangeAnimatorsList;
    std::vector<Animator*> mMoveAnimatorsList;

    std::unordered_map<RecyclerView::ViewHolder*, Animator*> mAnimators;

    /** Root pending sets built by runPendingAnimations; pruned when finished.
     *  (GC-owned upstream; child AnimatorSets/Animators are adopted by the root.) */
    std::vector<AnimatorSet*> mPendingSets;

public:
    ~ItemAnimator() override;

    bool animateRemove(RecyclerView::ViewHolder& holder) override;
    bool animateAdd(RecyclerView::ViewHolder& holder) override;
    bool animateMove(RecyclerView::ViewHolder& holder, int fromX, int fromY,
                     int toX, int toY) override;
    bool animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder* newHolder,
                       ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo) override;
    bool animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder* newHolder,
                       int fromLeft, int fromTop, int toLeft, int toTop) override;

    void runPendingAnimations() override;
    void endAnimation(RecyclerView::ViewHolder& holder) override;
    void endAnimations() override;
    bool isRunning() override;

    ItemHolderInfo* recordPreLayoutInformation(RecyclerView::State& state,
            RecyclerView::ViewHolder& viewHolder, int changeFlags,
            std::vector<Object*>& payloads) override;
    ItemHolderInfo* obtainHolderInfo() override;
    bool canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder,
                                   std::vector<Object*>& payloads) override;

private:
    void dispatchFinishedWhenDone();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ITEMANIMATOR_H__
