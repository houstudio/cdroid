/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __ADAPTER_HELPER_H__
#define __ADAPTER_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <core/pools.h>
namespace cdroid{
class AdapterHelper{
public:
    static constexpr int POSITION_TYPE_INVISIBLE = 0;
    static constexpr int POSITION_TYPE_NEW_OR_LAID_OUT = 1;
    class UpdateOp {
    public:
        static constexpr int ADD = 1;
        static constexpr int REMOVE = 1 << 1;
        static constexpr int UPDATE = 1 << 2;
        static constexpr int MOVE = 1 << 3;
        static constexpr int POOL_SIZE = 30;
        int cmd;
        int positionStart;
        // holds the target position if this is a MOVE
        int itemCount;
        Object* payload;
    public:
        UpdateOp();
        UpdateOp(int cmd, int positionStart, int itemCount, Object* payload);
        const std::string toString()const;
    };	
    struct Callback {
        std::function<RecyclerView::ViewHolder*(int)> findViewHolder;//(int position)
        std::function<void(int,int)> offsetPositionsForRemovingInvisible;//(int positionStart, int itemCount);
        std::function<void(int,int)> offsetPositionsForRemovingLaidOutOrNewView;//(int positionStart, int itemCount);
        std::function<void(int,int,Object*)> markViewHoldersUpdated;//(int positionStart, int itemCount, Object payloads);
        std::function<void(UpdateOp*)> onDispatchFirstPass;//(UpdateOp updateOp);
        std::function<void(UpdateOp*)> onDispatchSecondPass;//(UpdateOp updateOp);
        std::function<void(int,int)> offsetPositionsForAdd;//(int positionStart, int itemCount);
        std::function<void(int,int)> offsetPositionsForMove;//(int from, int to);
    };
private:
    friend GapWorker;
    friend RecyclerView;
    int mExistingUpdateTypes;
    Pools::SimplePool<UpdateOp> mUpdateOpPool;
private:
    void applyMove(UpdateOp* op);
    void applyRemove(UpdateOp* op);
    void applyUpdate(UpdateOp* op);
    void dispatchAndUpdateViewHolders(UpdateOp* op);
    int updatePositionWithPostponed(int pos, int cmd);
    bool canFindInPreLayout(int position);
    void applyAdd(UpdateOp* op);
    void postponeAndUpdateViewHolders(UpdateOp* op);
protected:
    Callback mCallback;
    std::vector<UpdateOp*> mPendingUpdates;
    std::vector<UpdateOp*> mPostponedList;
    Runnable mOnItemProcessedCallback;
    class OpReorderer* mOpReorderer;
    bool mDisableRecycler;
protected:
    void recycleUpdateOpsAndClearList(std::vector<UpdateOp*>& ops);
    void reset();
    void preProcess();
    void consumePostponedUpdates();
    void dispatchFirstPassAndUpdateViewHolders(UpdateOp* op, int offsetStart);
    bool hasPendingUpdates();
    bool hasAnyUpdateTypes(int updateTypes);
    int findPositionOffset(int position);
    int findPositionOffset(int position, int firstPostponedItem);
    bool onItemRangeChanged(int positionStart, int itemCount, Object* payload);
    bool onItemRangeInserted(int positionStart, int itemCount);
    bool onItemRangeRemoved(int positionStart, int itemCount);
    bool onItemRangeMoved(int from, int to, int itemCount);
    void consumeUpdatesInOnePass();
    bool hasUpdates();
public:
    AdapterHelper(Callback callback);
    AdapterHelper(Callback callback, bool disableRecycler);
    ~AdapterHelper();
    AdapterHelper& addUpdateOp(const std::vector<UpdateOp*>&);
    int applyPendingUpdatesToPosition(int position)const;
    UpdateOp* obtainUpdateOp(int cmd, int positionStart, int itemCount, Object* payload);
    void recycleUpdateOp(UpdateOp* op);
};
}/*endof namespace*/
#endif
