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
#ifndef __OP_REORDERER_H__
#define __OP_REORDERER_H__
#include <widgetEx/recyclerview/adapterhelper.h>

namespace cdroid{
class OpReorderer{
public:
    struct Callback {
        std::function<AdapterHelper::UpdateOp*(int,int,int,Object*)> obtainUpdateOp;//(int cmd, int startPosition, int itemCount, Object payload);
        std::function<void(AdapterHelper::UpdateOp*)> recycleUpdateOp;//(AdapterHelper::UpdateOp* op);
    };
private:
    void swapMoveOp(std::vector<AdapterHelper::UpdateOp*>& list, int badMove, int next);
    void swapMoveAdd(std::vector<AdapterHelper::UpdateOp*>& list, int move, AdapterHelper::UpdateOp* moveOp, int add,
            AdapterHelper::UpdateOp* addOp);
    int getLastMoveOutOfOrder(std::vector<AdapterHelper::UpdateOp*>& list);
protected:
    Callback mCallback;
    friend AdapterHelper;
protected:
    void reorderOps(std::vector<AdapterHelper::UpdateOp*>& ops);
    void swapMoveRemove(std::vector<AdapterHelper::UpdateOp*>& list, int movePos, AdapterHelper::UpdateOp* moveOp,
            int removePos, AdapterHelper::UpdateOp* removeOp);
    void swapMoveUpdate(std::vector<AdapterHelper::UpdateOp*>& list, int move, AdapterHelper::UpdateOp* moveOp,
            int update,AdapterHelper::UpdateOp* updateOp);
public:
    OpReorderer(Callback callback);
};
}/*endof namespace*/
#endif/*__OP_REORERER_H__*/
