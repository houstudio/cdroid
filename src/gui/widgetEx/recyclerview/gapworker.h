#ifndef __GAP_WORKER_H__
#define __GAP_WORKER_H__
namespace cdroid{
class RecyclerView;
class GapWorker{
protected:
    class Task {
    public:
        bool neededNextFrame;
        int viewVelocity;
        int distanceToItem;
        RecyclerView* view;
        int position;
        void clear() {
            neededNextFrame = false;
            viewVelocity = 0;
            distanceToItem = 0;
            view = nullptr;
            position = 0;
        }
    };
private:
    /**
     * Temporary storage for prefetch Tasks that execute in {@link #prefetch(long)}. Task objects
     * are pooled in the ArrayList, and never removed to avoid allocations, but always cleared
     * in between calls.
     */
    std::vector<Task*> mTasks;
    Runnable mRunnable;
    friend RecyclerView;
    friend RecyclerView::LayoutManager;
private:
    void buildTaskList();
    static int TaskComparator(Task* lhs, Task* rhs);
    void* prefetchPositionWithDeadline(RecyclerView* view, int position, int64_t deadlineNs);
    void prefetchInnerRecyclerViewWithDeadline(RecyclerView* innerView,int64_t deadlineNs);
    void flushTaskWithDeadline(Task* task, int64_t deadlineNs);
    void flushTasksWithDeadline(int64_t deadlineNs); 
protected:
    static GapWorker* sGapWorker;// = new ThreadLocal<>();
    std::vector<RecyclerView*> mRecyclerViews;
    int64_t mPostTimeNs;
    int64_t mFrameIntervalNs;

    static bool isPrefetchPositionAttached(RecyclerView* view, int position);
    /**
     * Schedule a prefetch immediately after the current traversal.
     */
    void postFromTraversal(RecyclerView* recyclerView, int prefetchDx, int prefetchDy);
    /**
     * Prefetch information associated with a specific RecyclerView.
     */
    class LayoutPrefetchRegistryImpl:public RecyclerView::LayoutManager::LayoutPrefetchRegistry {
        int mPrefetchDx=5;
        int mPrefetchDy=5;
        int mCount;
        std::vector<int> mPrefetchArray;
    protected:
        friend GapWorker;
        friend RecyclerView::Recycler;
        friend RecyclerView::ViewFlinger;
        friend RecyclerView::LayoutManager;
        void setPrefetchVector(int dx, int dy);
        void collectPrefetchPositionsFromView(RecyclerView* view, bool nested);
        bool lastPrefetchIncludedPosition(int position);
        void clearPrefetchPositions();
        void addPosition(int layoutPosition, int pixelDistance)override;
    };
public:
    GapWorker();
    void add(RecyclerView* recyclerView);
    void remove(RecyclerView* recyclerView);
    void prefetch(int64_t deadlineNs);
    void run();
};
}/*endof namespace*/
#endif/*__GAP_WORKER_H__*/
