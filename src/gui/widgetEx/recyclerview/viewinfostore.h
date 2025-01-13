#ifndef __VIEW_INFO_STORE_H__
#define __VIEW_INFO_STORE_H__
#include <core/pools.h>
#include <map>

namespace cdroid{
class ViewInfoStore {
public:
    struct ProcessCallback {
        std::function<void(RecyclerView::ViewHolder*,RecyclerView::ItemAnimator::ItemHolderInfo*,
			RecyclerView::ItemAnimator::ItemHolderInfo*)> processDisappeared;
	    //(RecyclerView::ViewHolder* viewHolder,RecyclerView::ItemAnimator::ItemHolderInfo* preInfo, RecyclerView::ItemAnimator::ItemHolderInfo* postInfo);
	    std::function<void(RecyclerView::ViewHolder*,RecyclerView::ItemAnimator::ItemHolderInfo*,
			RecyclerView::ItemAnimator::ItemHolderInfo*)> processAppeared;
	    //(RecyclerView::ViewHolder* viewHolder, RecyclerView::ItemAnimator::ItemHolderInfo* preInfo,RecyclerView::ItemAnimator::ItemHolderInfo* postInfo);
	    std::function<void(RecyclerView::ViewHolder*,RecyclerView::ItemAnimator::ItemHolderInfo*,RecyclerView::ItemAnimator::ItemHolderInfo*)> processPersistent;
	    //(RecyclerView::ViewHolder* viewHolder,RecyclerView::ItemAnimator::ItemHolderInfo* preInfo, RecyclerView::ItemAnimator::ItemHolderInfo* postInfo);
	    std::function<void(RecyclerView::ViewHolder*)>unused;//(RecyclerView::ViewHolder* holder);
    };

    class InfoRecord {
        static constexpr int FLAG_DISAPPEARED = 1;
        static constexpr int FLAG_APPEAR = 1 << 1;
        static constexpr int FLAG_PRE = 1 << 2;
        static constexpr int FLAG_POST = 1 << 3;
        static constexpr int FLAG_APPEAR_AND_DISAPPEAR = FLAG_APPEAR | FLAG_DISAPPEARED;
        static constexpr int FLAG_PRE_AND_POST = FLAG_PRE | FLAG_POST;
        static constexpr int FLAG_APPEAR_PRE_AND_POST = FLAG_APPEAR | FLAG_PRE | FLAG_POST;
        int flags;
        RecyclerView::ItemAnimator::ItemHolderInfo* preInfo;
        RecyclerView::ItemAnimator::ItemHolderInfo* postInfo;
        static Pools::SimplePool<InfoRecord> sPool;
    private:
        friend ViewInfoStore;
    public:
        InfoRecord();
        static InfoRecord* obtain();
        static void recycle(InfoRecord* record);
        static void drainCache();
    };
protected:
    std::unordered_map<RecyclerView::ViewHolder*, InfoRecord*> mLayoutHolderMap;
    LongSparseArray<RecyclerView::ViewHolder*> mOldChangedHolders;
private:
    friend RecyclerView;
    RecyclerView::ItemAnimator::ItemHolderInfo* popFromLayoutStep(RecyclerView::ViewHolder* vh, int flag);
protected:
    void clear();
    void addToPreLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info);
    bool isDisappearing(RecyclerView::ViewHolder* holder);
    RecyclerView::ItemAnimator::ItemHolderInfo* popFromPreLayout(RecyclerView::ViewHolder* vh);
    RecyclerView::ItemAnimator::ItemHolderInfo* popFromPostLayout(RecyclerView::ViewHolder* vh);
    void addToOldChangeHolders(long key, RecyclerView::ViewHolder* holder);
    void addToAppearedInPreLayoutHolders(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info);
    bool isInPreLayout(RecyclerView::ViewHolder* viewHolder);
    RecyclerView::ViewHolder* getFromOldChangeHolders(long key);
    void addToPostLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info);
    void addToDisappearedInLayout(RecyclerView::ViewHolder* holder);
    void removeFromDisappearedInLayout(RecyclerView::ViewHolder* holder);
    void process(ProcessCallback callback);
    void removeViewHolder(RecyclerView::ViewHolder* holder);
    void onDetach();
public:
    void onViewDetached(RecyclerView::ViewHolder* viewHolder);
};
}/*endof namespace*/
#endif

