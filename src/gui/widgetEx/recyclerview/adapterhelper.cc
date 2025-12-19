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
#include <widgetEx/recyclerview/adapterhelper.h>
#include <widgetEx/recyclerview/opreorderer.h>
namespace cdroid{
static constexpr int _Debug = 0;
AdapterHelper::AdapterHelper(Callback callback)
   :AdapterHelper(callback, false){
}

AdapterHelper::AdapterHelper(Callback callback, bool disableRecycler)
    :mUpdateOpPool(UpdateOp::POOL_SIZE){
    mCallback = callback;
    mDisableRecycler = disableRecycler;
    OpReorderer::Callback cbk;
    cbk.obtainUpdateOp = [this](int cmd, int positionStart, int itemCount, Object* payload){
        return obtainUpdateOp(cmd,positionStart,itemCount,payload);
    };
    cbk.recycleUpdateOp= [this](UpdateOp* op){recycleUpdateOp(op);};
    mOpReorderer = new OpReorderer(cbk);
}

AdapterHelper::~AdapterHelper(){
    delete mOpReorderer;
}

AdapterHelper& AdapterHelper::addUpdateOp(const std::vector<UpdateOp*>&ops) {
    mPendingUpdates.insert(mPendingUpdates.end(),ops.begin(),ops.end());
    return *this;
}

void AdapterHelper::reset() {
    recycleUpdateOpsAndClearList(mPendingUpdates);
    recycleUpdateOpsAndClearList(mPostponedList);
    mExistingUpdateTypes = 0;
}

void AdapterHelper::preProcess() {
    mOpReorderer->reorderOps(mPendingUpdates);
    const size_t count = mPendingUpdates.size();
    for (size_t i = 0; i < count; i++) {
        UpdateOp* op = mPendingUpdates.at(i);
        switch (op->cmd) {
        case UpdateOp::ADD:    applyAdd(op);   break;
        case UpdateOp::REMOVE: applyRemove(op);break;
        case UpdateOp::UPDATE: applyUpdate(op);break;
        case UpdateOp::MOVE:   applyMove(op);  break;
        }
        if (mOnItemProcessedCallback != nullptr) {
            mOnItemProcessedCallback();//.run();
        }
    }
    mPendingUpdates.clear();
}

void AdapterHelper::consumePostponedUpdates() {
    const size_t count = mPostponedList.size();
    for (size_t i = 0; i < count; i++) {
        mCallback.onDispatchSecondPass(mPostponedList.at(i));
    }
    recycleUpdateOpsAndClearList(mPostponedList);
    mExistingUpdateTypes = 0;
}

void AdapterHelper::applyMove(UpdateOp* op) {
    // MOVE ops are pre-processed so at this point, we know that item is still in the adapter.
    // otherwise, it would be converted into a REMOVE operation
    postponeAndUpdateViewHolders(op);
}

void AdapterHelper::applyRemove(UpdateOp* op) {
    int tmpStart = op->positionStart;
    int tmpCount = 0;
    int tmpEnd = op->positionStart + op->itemCount;
    int type = -1;
    for (int position = op->positionStart; position < tmpEnd; position++) {
        bool typeChanged = false;
        RecyclerView::ViewHolder* vh = mCallback.findViewHolder(position);
        if (vh != nullptr || canFindInPreLayout(position)) {
            // If a ViewHolder exists or this is a newly added item, we can defer this update
            // to post layout stage.
            // * For existing ViewHolders, we'll fake its existence in the pre-layout phase.
            // * For items that are added and removed in the same process cycle, they won't
            // have any effect in pre-layout since their add ops are already deferred to
            // post-layout pass.
            if (type == POSITION_TYPE_INVISIBLE) {
                // Looks like we have other updates that we cannot merge with this one.
                // Create an UpdateOp and dispatch it to LayoutManager.
                UpdateOp* newOp = obtainUpdateOp(UpdateOp::REMOVE, tmpStart, tmpCount, nullptr);
                dispatchAndUpdateViewHolders(newOp);
                typeChanged = true;
            }
            type = POSITION_TYPE_NEW_OR_LAID_OUT;
        } else {
            // This update cannot be recovered because we don't have a ViewHolder representing
            // this position. Instead, post it to LayoutManager immediately
            if (type == POSITION_TYPE_NEW_OR_LAID_OUT) {
                // Looks like we have other updates that we cannot merge with this one.
                // Create UpdateOp op and dispatch it to LayoutManager.
                UpdateOp* newOp = obtainUpdateOp(UpdateOp::REMOVE, tmpStart, tmpCount, nullptr);
                postponeAndUpdateViewHolders(newOp);
                typeChanged = true;
            }
            type = POSITION_TYPE_INVISIBLE;
        }
        if (typeChanged) {
            position -= tmpCount; // also equal to tmpStart
            tmpEnd -= tmpCount;
            tmpCount = 1;
        } else {
            tmpCount++;
        }
    }
    if (tmpCount != op->itemCount) { // all 1 effect
        recycleUpdateOp(op);
        op = obtainUpdateOp(UpdateOp::REMOVE, tmpStart, tmpCount, nullptr);
    }
    if (type == POSITION_TYPE_INVISIBLE) {
        dispatchAndUpdateViewHolders(op);
    } else {
        postponeAndUpdateViewHolders(op);
    }
}

void AdapterHelper::applyUpdate(UpdateOp* op) {
    int tmpStart = op->positionStart;
    int tmpCount = 0;
    int tmpEnd = op->positionStart + op->itemCount;
    int type = -1;
    for (int position = op->positionStart; position < tmpEnd; position++) {
        RecyclerView::ViewHolder* vh = mCallback.findViewHolder(position);
        if ((vh !=nullptr) || canFindInPreLayout(position)) { // deferred
            if (type == POSITION_TYPE_INVISIBLE) {
                UpdateOp* newOp = obtainUpdateOp(UpdateOp::UPDATE, tmpStart, tmpCount,
                        op->payload);
                dispatchAndUpdateViewHolders(newOp);
                tmpCount = 0;
                tmpStart = position;
            }
            type = POSITION_TYPE_NEW_OR_LAID_OUT;
        } else { // applied
            if (type == POSITION_TYPE_NEW_OR_LAID_OUT) {
                UpdateOp* newOp = obtainUpdateOp(UpdateOp::UPDATE, tmpStart, tmpCount,
                        op->payload);
                postponeAndUpdateViewHolders(newOp);
                tmpCount = 0;
                tmpStart = position;
            }
            type = POSITION_TYPE_INVISIBLE;
        }
        tmpCount++;
    }
    if (tmpCount != op->itemCount) { // all 1 effect
        Object* payload = op->payload;
        recycleUpdateOp(op);
        op = obtainUpdateOp(UpdateOp::UPDATE, tmpStart, tmpCount, payload);
    }
    if (type == POSITION_TYPE_INVISIBLE) {
        dispatchAndUpdateViewHolders(op);
    } else {
        postponeAndUpdateViewHolders(op);
    }
}

void AdapterHelper::dispatchAndUpdateViewHolders(UpdateOp* op) {
    // tricky part.
    // traverse all postpones and revert their changes on this op if necessary, apply updated
    // dispatch to them since now they are after this op.
    if (op->cmd == UpdateOp::ADD || op->cmd == UpdateOp::MOVE) {
        throw "should not dispatch add or move for pre layout";
    }
    if (_Debug) {
        LOGD("dispatch (pre)%p" ,op);
        LOGD("postponed state before:");
        for (UpdateOp* updateOp : mPostponedList) {
            LOGD("%s",updateOp->toString().c_str());
        }
    }

    // handle each pos 1 by 1 to ensure continuity. If it breaks, dispatch partial
    // TODO Since move ops are pushed to end, we should not need this anymore
    int tmpStart = updatePositionWithPostponed(op->positionStart, op->cmd);
    LOGD_IF(_Debug,"pos:%d,updatedPos:%d mPostponedList.size=%d",op->positionStart,tmpStart,mPostponedList.size());
    int tmpCnt = 1;
    int offsetPositionForPartial = op->positionStart;
    int positionMultiplier;
    switch (op->cmd) {
    case UpdateOp::UPDATE:  positionMultiplier = 1;  break;
    case UpdateOp::REMOVE:  positionMultiplier = 0;  break;
    default:   FATAL("op should be remove or update.%d",op->cmd);
    }
    for (int p = 1; p < op->itemCount; p++) {
        const int pos = op->positionStart + (positionMultiplier * p);
        int updatedPos = updatePositionWithPostponed(pos, op->cmd);
        LOGD_IF(_Debug,"pos:%d,updatedPos:%d",pos,updatedPos);
        bool continuous = false;
        switch (op->cmd) {
	case UpdateOp::UPDATE: continuous = updatedPos == tmpStart + 1;  break;
	case UpdateOp::REMOVE: continuous = updatedPos == tmpStart;      break;
        }
        if (continuous) {
            tmpCnt++;
        } else {
            // need to dispatch this separately
            UpdateOp* tmp = obtainUpdateOp(op->cmd, tmpStart, tmpCnt, op->payload);
            LOGD_IF(_Debug,"need to dispatch separately %d",tmp->cmd);
            dispatchFirstPassAndUpdateViewHolders(tmp, offsetPositionForPartial);
            recycleUpdateOp(tmp);
            if (op->cmd == UpdateOp::UPDATE) {
                offsetPositionForPartial += tmpCnt;
            }
            tmpStart = updatedPos; // need to remove previously dispatched
            tmpCnt = 1;
        }
    }
    Object* payload = op->payload;
    recycleUpdateOp(op);
    if (tmpCnt > 0) {
        UpdateOp* tmp = obtainUpdateOp(op->cmd, tmpStart, tmpCnt, payload);
        LOGD_IF(_Debug,"dispatching:%d",tmp->cmd);
        dispatchFirstPassAndUpdateViewHolders(tmp, offsetPositionForPartial);
        recycleUpdateOp(tmp);
    }
    if (_Debug) {
        LOGD("post dispatch");
        LOGD("postponed state after:");
        for (UpdateOp* updateOp : mPostponedList) {
            LOGD("%s",updateOp->toString().c_str());
        }
    }
}
void AdapterHelper::dispatchFirstPassAndUpdateViewHolders(UpdateOp* op, int offsetStart) {
    mCallback.onDispatchFirstPass(op);
    switch (op->cmd) {
    case UpdateOp::REMOVE:
        mCallback.offsetPositionsForRemovingInvisible(offsetStart, op->itemCount);
        break;
    case UpdateOp::UPDATE:
        mCallback.markViewHoldersUpdated(offsetStart, op->itemCount, op->payload);
        break;
    default:
         FATAL("only remove and update ops can be dispatched in first pass");
    }
}

int AdapterHelper::updatePositionWithPostponed(int pos, int cmd) {
    const int count = (int)mPostponedList.size();
    for (int i = count - 1; i >= 0; i--) {
        UpdateOp* postponed = mPostponedList.at(i);
        if (postponed->cmd == UpdateOp::MOVE) {
            int start, end;
            if (postponed->positionStart < postponed->itemCount) {
                start = postponed->positionStart;
                end = postponed->itemCount;
            } else {
                start = postponed->itemCount;
                end = postponed->positionStart;
            }
            if (pos >= start && pos <= end) {
                //i'm affected
                if (start == postponed->positionStart) {
                    if (cmd == UpdateOp::ADD) {
                        postponed->itemCount++;
                    } else if (cmd == UpdateOp::REMOVE) {
                        postponed->itemCount--;
                    }
                    // op moved to left, move it right to revert
                    pos++;
                } else {
                    if (cmd == UpdateOp::ADD) {
                        postponed->positionStart++;
                    } else if (cmd == UpdateOp::REMOVE) {
                        postponed->positionStart--;
                    }
                    // op was moved right, move left to revert
                    pos--;
                }
            } else if (pos < postponed->positionStart) {
                // postponed MV is outside the dispatched OP. if it is before, offset
                if (cmd == UpdateOp::ADD) {
                    postponed->positionStart++;
                    postponed->itemCount++;
                } else if (cmd == UpdateOp::REMOVE) {
                    postponed->positionStart--;
                    postponed->itemCount--;
                }
            }
        } else {
            if (postponed->positionStart <= pos) {
                if (postponed->cmd == UpdateOp::ADD) {
                    pos -= postponed->itemCount;
                } else if (postponed->cmd == UpdateOp::REMOVE) {
                    pos += postponed->itemCount;
                }
            } else {
                if (cmd == UpdateOp::ADD) {
                    postponed->positionStart++;
                } else if (cmd == UpdateOp::REMOVE) {
                    postponed->positionStart--;
                }
            }
        }
        if (_Debug) {
            LOGD("dispath (step %d)",i);
            LOGD("postponed state:%d, pos:%d",i,pos);
            for (UpdateOp* updateOp : mPostponedList) {
                LOGD(updateOp->toString().c_str());
            }
        }
    }
    for (int i = int(mPostponedList.size() - 1); i >= 0; i--) {
        UpdateOp* op = mPostponedList.at(i);
        if (op->cmd == UpdateOp::MOVE) {
            if (op->itemCount == op->positionStart || op->itemCount < 0) {
                mPostponedList.erase(mPostponedList.begin()+i);//remove(i);
                recycleUpdateOp(op);
            }
        } else if (op->itemCount <= 0) {
            mPostponedList.erase(mPostponedList.begin()+i);//remove(i);
            recycleUpdateOp(op);
        }
    }
    LOGD("mPostponedList.size %d->%d",count,mPostponedList.size());
    return pos;
}

bool AdapterHelper::canFindInPreLayout(int position) {
    const int count = (int)mPostponedList.size();
    for (int i = 0; i < count; i++) {
        UpdateOp* op = mPostponedList.at(i);
        if (op->cmd == UpdateOp::MOVE) {
            if (findPositionOffset(op->itemCount, i + 1) == position) {
                return true;
            }
        } else if (op->cmd == UpdateOp::ADD) {
            // TODO optimize.
            const int end = op->positionStart + op->itemCount;
            for (int pos = op->positionStart; pos < end; pos++) {
                if (findPositionOffset(pos, i + 1) == position) {
                    return true;
                }
            }
        }
    }
    return false;
}
void AdapterHelper::applyAdd(UpdateOp* op) {
    postponeAndUpdateViewHolders(op);
}

void AdapterHelper::postponeAndUpdateViewHolders(UpdateOp* op) {
    mPostponedList.push_back(op);//add(op);
    LOGD_IF(_Debug,"postponing op->%p:%d mPostponedList.size=%d",op,op->cmd,mPostponedList.size());
    switch (op->cmd) {
   case UpdateOp::ADD:
        mCallback.offsetPositionsForAdd(op->positionStart, op->itemCount);
        break;
   case UpdateOp::MOVE:
        mCallback.offsetPositionsForMove(op->positionStart, op->itemCount);
        break;
   case UpdateOp::REMOVE:
        mCallback.offsetPositionsForRemovingLaidOutOrNewView(op->positionStart,
                op->itemCount);
        break;
   case UpdateOp::UPDATE:
        mCallback.markViewHoldersUpdated(op->positionStart, op->itemCount, op->payload);
        break;
    default:FATAL("Unknown update op type for %d",op->cmd);
    }
}

bool AdapterHelper::hasPendingUpdates() {
    return mPendingUpdates.size() > 0;
}

bool AdapterHelper::hasAnyUpdateTypes(int updateTypes) {
    return (mExistingUpdateTypes & updateTypes) != 0;
}

int AdapterHelper::findPositionOffset(int position) {
    return findPositionOffset(position, 0);
}

int AdapterHelper::findPositionOffset(int position, int firstPostponedItem) {
    const int count = (int)mPostponedList.size();
    for (int i = firstPostponedItem; i < count; ++i) {
        UpdateOp* op = mPostponedList.at(i);
        if (op->cmd == UpdateOp::MOVE) {
            if (op->positionStart == position) {
                position = op->itemCount;
            } else {
                if (op->positionStart < position) {
                    position--; // like a remove
                }
                if (op->itemCount <= position) {
                    position++; // like an add
                }
            }
        } else if (op->positionStart <= position) {
            if (op->cmd == UpdateOp::REMOVE) {
                if (position < op->positionStart + op->itemCount) {
                    return -1;
                }
                position -= op->itemCount;
            } else if (op->cmd == UpdateOp::ADD) {
                position += op->itemCount;
            }
        }
    }
    return position;
}

bool AdapterHelper::onItemRangeChanged(int positionStart, int itemCount, Object* payload) {
    if (itemCount < 1) {
        return false;
    }
    mPendingUpdates.push_back(obtainUpdateOp(UpdateOp::UPDATE, positionStart, itemCount, payload));
    mExistingUpdateTypes |= UpdateOp::UPDATE;
    return mPendingUpdates.size() == 1;
}

bool AdapterHelper::onItemRangeInserted(int positionStart, int itemCount) {
    if (itemCount < 1) {
        return false;
    }
    mPendingUpdates.push_back(obtainUpdateOp(UpdateOp::ADD, positionStart, itemCount, nullptr));
    mExistingUpdateTypes |= UpdateOp::ADD;
    return mPendingUpdates.size() == 1;
}

/**
 * @return True if updates should be processed.
 */
bool AdapterHelper::onItemRangeRemoved(int positionStart, int itemCount) {
    if (itemCount < 1) {
        return false;
    }
    mPendingUpdates.push_back(obtainUpdateOp(UpdateOp::REMOVE, positionStart, itemCount, nullptr));
    mExistingUpdateTypes |= UpdateOp::REMOVE;
    return mPendingUpdates.size() == 1;
}

/**
 * @return True if updates should be processed.
 */
bool AdapterHelper::onItemRangeMoved(int from, int to, int itemCount) {
    if (from == to) {
        return false; // no-op
    }
    if (itemCount != 1) {
        FATAL("Moving more than 1 item is not supported yet");
    }
    mPendingUpdates.push_back(obtainUpdateOp(UpdateOp::MOVE, from, to, nullptr));
    mExistingUpdateTypes |= UpdateOp::MOVE;
    return mPendingUpdates.size() == 1;
}

/**
 * Skips pre-processing and applies all updates in one pass.
 */
void AdapterHelper::consumeUpdatesInOnePass() {
    // we still consume postponed updates (if there is) in case there was a pre-process call
    // w/o a matching consumePostponedUpdates.
    consumePostponedUpdates();
    const size_t count = mPendingUpdates.size();
    for (size_t i = 0; i < count; i++) {
        UpdateOp* op = mPendingUpdates.at(i);
        switch (op->cmd) {
        case UpdateOp::ADD:
            mCallback.onDispatchSecondPass(op);
            mCallback.offsetPositionsForAdd(op->positionStart, op->itemCount);
            break;
        case UpdateOp::REMOVE:
            mCallback.onDispatchSecondPass(op);
            mCallback.offsetPositionsForRemovingInvisible(op->positionStart, op->itemCount);
            break;
        case UpdateOp::UPDATE:
            mCallback.onDispatchSecondPass(op);
            mCallback.markViewHoldersUpdated(op->positionStart, op->itemCount, op->payload);
            break;
        case UpdateOp::MOVE:
            mCallback.onDispatchSecondPass(op);
            mCallback.offsetPositionsForMove(op->positionStart, op->itemCount);
            break;
        }
        if (mOnItemProcessedCallback != nullptr) {
            mOnItemProcessedCallback();//.run();
        }
    }
    recycleUpdateOpsAndClearList(mPendingUpdates);
    mExistingUpdateTypes = 0;
}

int AdapterHelper::applyPendingUpdatesToPosition(int position)const {
    const size_t size = mPendingUpdates.size();
    for (size_t i = 0; i < size; i++) {
        UpdateOp* op = mPendingUpdates.at(i);
        switch (op->cmd) {
        case UpdateOp::ADD:
            if (op->positionStart <= position) {
                position += op->itemCount;
            }
            break;
        case UpdateOp::REMOVE:
            if (op->positionStart <= position) {
                const int end = op->positionStart + op->itemCount;
                if (end > position) {
                    return RecyclerView::NO_POSITION;
                }
                position -= op->itemCount;
            }
            break;
        case UpdateOp::MOVE:
            if (op->positionStart == position) {
                position = op->itemCount; //position end
            } else {
                if (op->positionStart < position) {
                    position -= 1;
                }
                if (op->itemCount <= position) {
                    position += 1;
                }
            }
            break;
        }
    }
    return position;
}

bool AdapterHelper::hasUpdates() {
    return !mPostponedList.empty() && !mPendingUpdates.empty();
}

AdapterHelper::UpdateOp* AdapterHelper::obtainUpdateOp(int cmd, int positionStart, int itemCount, Object* payload) {
    UpdateOp* op = mUpdateOpPool.acquire();
    if (op == nullptr) {
        op = new UpdateOp(cmd, positionStart, itemCount, payload);
    } else {
        op->cmd = cmd;
        op->positionStart = positionStart;
        op->itemCount = itemCount;
        op->payload = payload;
    }
    return op;
}

void AdapterHelper::recycleUpdateOp(UpdateOp* op) {
    if (!mDisableRecycler) {
        op->payload = nullptr;
        if(!mUpdateOpPool.release(op)){
            /*out of pool*/
            delete op;
        }
    }
}

void AdapterHelper::recycleUpdateOpsAndClearList(std::vector<UpdateOp*>& ops) {
    const size_t count = ops.size();
    for (size_t i = 0; i < count; i++) {
        recycleUpdateOp(ops.at(i));
    }
    ops.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AdapterHelper::UpdateOp::UpdateOp(){
    cmd = 0;
    itemCount = 0;
    payload = nullptr;
}

AdapterHelper::UpdateOp::UpdateOp(int cmd, int positionStart, int itemCount, Object* payload) {
    this->cmd = cmd;
    this->positionStart = positionStart;
    this->itemCount = itemCount;
    this->payload = payload;
}

const std::string AdapterHelper::UpdateOp::toString()const{
    std::ostringstream oss;
    const char*cmds[]={"add","rm","up","mv"};
    const int idx = __builtin_clz(cmd);
    oss<<cmds[idx]<<",s:"<<positionStart<<" c:"<<itemCount;
    return oss.str();
}

}/*endof namespace*/
