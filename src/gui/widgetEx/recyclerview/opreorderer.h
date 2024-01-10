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
