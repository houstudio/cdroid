#include <widgetEx/recyclerview/opreorderer.h>
namespace cdroid{
OpReorderer::OpReorderer(Callback callback) {
    mCallback = callback;
}

void OpReorderer::reorderOps(std::vector<AdapterHelper::UpdateOp*>& ops) {
    // since move operations breaks continuity, their effects on ADD/RM are hard to handle.
    // we push them to the end of the list so that they can be handled easily.
    int badMove;
    while ((badMove = getLastMoveOutOfOrder(ops)) != -1) {
        swapMoveOp(ops, badMove, badMove + 1);
    }
}

void OpReorderer::swapMoveOp(std::vector<AdapterHelper::UpdateOp*>& list, int badMove, int next) {
    AdapterHelper::UpdateOp* moveOp = list.at(badMove);
    AdapterHelper::UpdateOp* nextOp = list.at(next);
    switch (nextOp->cmd) {
    case AdapterHelper::UpdateOp::REMOVE:
        swapMoveRemove(list, badMove, moveOp, next, nextOp);
        break;
    case AdapterHelper::UpdateOp::ADD:
        swapMoveAdd(list, badMove, moveOp, next, nextOp);
        break;
    case AdapterHelper::UpdateOp::UPDATE:
        swapMoveUpdate(list, badMove, moveOp, next, nextOp);
        break;
    }
}

void OpReorderer::swapMoveRemove(std::vector<AdapterHelper::UpdateOp*>& list, int movePos, AdapterHelper::UpdateOp* moveOp,
        int removePos, AdapterHelper::UpdateOp* removeOp) {
    AdapterHelper::UpdateOp* extraRm = nullptr;
    // check if move is nulled out by remove
    bool revertedMove = false;
    bool moveIsBackwards;

    if (moveOp->positionStart < moveOp->itemCount) {
        moveIsBackwards = false;
        if (removeOp->positionStart == moveOp->positionStart
                && removeOp->itemCount == moveOp->itemCount - moveOp->positionStart) {
            revertedMove = true;
        }
    } else {
        moveIsBackwards = true;
        if (removeOp->positionStart == moveOp->itemCount + 1
                && removeOp->itemCount == moveOp->positionStart - moveOp->itemCount) {
            revertedMove = true;
        }
    }

    // going in reverse, first revert the effect of add
    if (moveOp->itemCount < removeOp->positionStart) {
        removeOp->positionStart--;
    } else if (moveOp->itemCount < removeOp->positionStart + removeOp->itemCount) {
        // move is removed.
        removeOp->itemCount--;
        moveOp->cmd = AdapterHelper::UpdateOp::REMOVE;
        moveOp->itemCount = 1;
        if (removeOp->itemCount == 0) {
            list.erase(list.begin()+removePos);//remove(removePos);
            mCallback.recycleUpdateOp(removeOp);
        }
        // no need to swap, it is already a remove
        return;
    }

    // now affect of add is consumed. now apply effect of first remove
    if (moveOp->positionStart <= removeOp->positionStart) {
        removeOp->positionStart++;
    } else if (moveOp->positionStart < removeOp->positionStart + removeOp->itemCount) {
        int remaining = removeOp->positionStart + removeOp->itemCount
                - moveOp->positionStart;
        extraRm = mCallback.obtainUpdateOp(AdapterHelper::UpdateOp::REMOVE, moveOp->positionStart + 1, remaining, nullptr);
        removeOp->itemCount = moveOp->positionStart - removeOp->positionStart;
    }

    // if effects of move is reverted by remove, we are done.
    if (revertedMove) {
        list[movePos]=removeOp;//list.set(movePos, removeOp);
        list.erase(list.begin()+removePos);//list.remove(removePos);
        mCallback.recycleUpdateOp(moveOp);
        return;
    }

    // now find out the new locations for move actions
    if (moveIsBackwards) {
        if (extraRm != nullptr) {
            if (moveOp->positionStart > extraRm->positionStart) {
                moveOp->positionStart -= extraRm->itemCount;
            }
            if (moveOp->itemCount > extraRm->positionStart) {
                moveOp->itemCount -= extraRm->itemCount;
            }
        }
        if (moveOp->positionStart > removeOp->positionStart) {
            moveOp->positionStart -= removeOp->itemCount;
        }
        if (moveOp->itemCount > removeOp->positionStart) {
            moveOp->itemCount -= removeOp->itemCount;
        }
    } else {
        if (extraRm != nullptr) {
            if (moveOp->positionStart >= extraRm->positionStart) {
                moveOp->positionStart -= extraRm->itemCount;
            }
            if (moveOp->itemCount >= extraRm->positionStart) {
                moveOp->itemCount -= extraRm->itemCount;
            }
        }
        if (moveOp->positionStart >= removeOp->positionStart) {
            moveOp->positionStart -= removeOp->itemCount;
        }
        if (moveOp->itemCount >= removeOp->positionStart) {
            moveOp->itemCount -= removeOp->itemCount;
        }
    }

    list[movePos]=removeOp;//list.set(movePos, removeOp);
    if (moveOp->positionStart != moveOp->itemCount) {
        list[removePos]=moveOp;//list.set(removePos, moveOp);
    } else {
        list.erase(list.begin()+removePos);//list.remove(removePos);
    }
    if (extraRm != nullptr) {
        list.insert(list.begin()+movePos,extraRm);//list.add(movePos, extraRm);
    }
}

void OpReorderer::swapMoveAdd(std::vector<AdapterHelper::UpdateOp*>& list, int move, AdapterHelper::UpdateOp* moveOp, int add,
        AdapterHelper::UpdateOp* addOp) {
    int offset = 0;
    // going in reverse, first revert the effect of add
    if (moveOp->itemCount < addOp->positionStart) {
        offset--;
    }
    if (moveOp->positionStart < addOp->positionStart) {
        offset++;
    }
    if (addOp->positionStart <= moveOp->positionStart) {
        moveOp->positionStart += addOp->itemCount;
    }
    if (addOp->positionStart <= moveOp->itemCount) {
        moveOp->itemCount += addOp->itemCount;
    }
    addOp->positionStart += offset;
    list[move]=addOp;//list.set(move, addOp);
    list[add]=moveOp;//list.set(add, moveOp);
}

void OpReorderer::swapMoveUpdate(std::vector<AdapterHelper::UpdateOp*>& list, int move, AdapterHelper::UpdateOp* moveOp,
	int update, AdapterHelper::UpdateOp* updateOp) {
    AdapterHelper::UpdateOp* extraUp1 = nullptr;
    AdapterHelper::UpdateOp* extraUp2 = nullptr;
    // going in reverse, first revert the effect of add
    if (moveOp->itemCount < updateOp->positionStart) {
        updateOp->positionStart--;
    } else if (moveOp->itemCount < updateOp->positionStart + updateOp->itemCount) {
        // moved item is updated. add an update for it
        updateOp->itemCount--;
        extraUp1 = mCallback.obtainUpdateOp(AdapterHelper::UpdateOp::UPDATE, moveOp->positionStart, 1, updateOp->payload);
    }
    // now affect of add is consumed. now apply effect of first remove
    if (moveOp->positionStart <= updateOp->positionStart) {
        updateOp->positionStart++;
    } else if (moveOp->positionStart < updateOp->positionStart + updateOp->itemCount) {
        const int remaining = updateOp->positionStart + updateOp->itemCount
                - moveOp->positionStart;
        extraUp2 = mCallback.obtainUpdateOp(
                AdapterHelper::UpdateOp::UPDATE, moveOp->positionStart + 1, remaining,
                updateOp->payload);
        updateOp->itemCount -= remaining;
    }
    list[update]=moveOp;//.set(update, moveOp);
    if (updateOp->itemCount > 0) {
        list[move]=updateOp;//.set(move, updateOp);
    } else {
        list.erase(list.begin()+move);//.remove(move);
        mCallback.recycleUpdateOp(updateOp);
    }
    if (extraUp1 != nullptr) {
        list.insert(list.begin()+move,extraUp1);//.add(move, extraUp1);
    }
    if (extraUp2 != nullptr) {
        list.insert(list.begin()+move,extraUp2);//.add(move, extraUp2);
    }
}

int OpReorderer::getLastMoveOutOfOrder(std::vector<AdapterHelper::UpdateOp*>& list) {
    bool foundNonMove = false;
    for (int i = list.size() - 1; i >= 0; i--) {
        AdapterHelper::UpdateOp* op1 = list.at(i);
        if (op1->cmd == AdapterHelper::UpdateOp::MOVE) {
            if (foundNonMove) {
                return i;
            }
        } else {
            foundNonMove = true;
        }
    }
    return -1;
}
}/*endof namespace*/
