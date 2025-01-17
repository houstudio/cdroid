#ifndef __CHILD_HELPER_H__
#define __CHILD_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class ChildHelper{
private:
    /** Not in call to removeView/removeViewAt/removeViewIfHidden. */
    static constexpr int REMOVE_STATUS_NONE = 0;
    /** Within a call to removeView/removeViewAt. */
    static constexpr int REMOVE_STATUS_IN_REMOVE = 1;
    /** Within a call to removeViewIfHidden. */
    static constexpr int REMOVE_STATUS_IN_REMOVE_IF_HIDDEN = 2;
public:
    struct Callback{
        std::function<int()>getChildCount;
        std::function<void(View*,int)> addView;//(View* child, int index);
        std::function<int(View*)> indexOfChild;//(View* view);
        std::function<void(int)> removeViewAt;//(int index);
        std::function<View*(int)> getChildAt;//(int offset);
        std::function<void()> removeAllViews;
        std::function<RecyclerView::ViewHolder*(View*)> getChildViewHolder;//(View* view);
        std::function<void(View*,int,ViewGroup::LayoutParams*)> attachViewToParent;//(View* child, int index, ViewGroup::LayoutParams* layoutParams);
        std::function<void(int)> detachViewFromParent;//(int offset);
        std::function<void(View*)> onEnteredHiddenState;//(View* child);
        std::function<void(View*)> onLeftHiddenState;//(View* child);
    };
    class Bucket;
    Callback mCallback;
    Bucket* mBucket;
    std::vector<View*>mHiddenViews;
private:
    static constexpr bool _Debug = false;
    int mRemoveStatus = REMOVE_STATUS_NONE;
    /** The view to remove in REMOVE_STATUS_IN_REMOVE. */
    View* mViewInRemoveView;
private:
    void hideViewInternal(View* child);
    bool unhideViewInternal(View* child);
    int getOffset(int index);
public:
    ChildHelper(Callback& callback);
    virtual ~ChildHelper();
    void addView(View* child, bool hidden);
    void addView(View* child, int index, bool hidden);
    void removeView(View* view);
    void removeViewAt(int index);
    View* getChildAt(int index);
    void removeAllViewsUnfiltered();
    View* findHiddenNonRemovedView(int position);
    void attachViewToParent(View* child, int index, ViewGroup::LayoutParams* layoutParams,
            bool hidden);
    int getChildCount()const;
    int getUnfilteredChildCount();
    View* getUnfilteredChildAt(int index);
    void detachViewFromParent(int index);
    int indexOfChild(View* child);
    bool isHidden(View* view);
    void hide(View* view);
    void unhide(View* view);
    bool removeViewIfHidden(View* view);
};

class ChildHelper::Bucket{
public:
    static constexpr int BITS_PER_WORD = sizeof(long)*8;
    static constexpr long LAST_BIT= 1L<<(BITS_PER_WORD-1);
    long mData;
    ChildHelper::Bucket*mNext;
private:
    void ensureNext();
public:
    Bucket();
    void set(int);
    void clear(int);
    bool get(int);
    void reset();
    void insert(int,bool);
    bool remove(int);
    int countOnesBefore(int);
};
}
#endif
