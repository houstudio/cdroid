#ifndef __BACKSTACKRECORD_H__
#define __BACKSTACKRECORD_H__
/*********************************************************************************
 * Port of androidx.fragment.app.BackStackRecord. A concrete FragmentTransaction
 * that records ops and can commit them (and reverse them for popBackStack).
 * MVP: commit() executes ops synchronously (no Handler-deferred execPendingActions).
 *********************************************************************************/
#include <fragment/fragmenttransaction.h>
namespace cdroid{
namespace fragment{

class FragmentManager;

class BackStackRecord : public FragmentTransaction{
public:
    explicit BackStackRecord(FragmentManager* manager);

    int commit() override;
    int commitAllowingStateLoss() override;
    void commitNow() override;
    void commitNowAllowingStateLoss() override;

    // Apply all recorded ops (add/remove/hide/...).
    void executeOps();
    // Reverse all recorded ops (used by popBackStack).
    void executePopOps();

    int getIndex() const { return mIndex; }
    void setIndex(int index){ mIndex = index; }
    const std::string& getName() const { return mName; }

private:
    int commitInternal();
    FragmentManager* mManager;
    int mIndex = -1;
};

}//namespace fragment
}//namespace cdroid
#endif
