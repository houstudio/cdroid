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
#ifndef __BACKSTACKRECORD_H__
#define __BACKSTACKRECORD_H__
/*********************************************************************************
 * Port of androidx.fragment.app.BackStackRecord. A concrete FragmentTransaction
 * that records ops and can commit them (and reverse them for popBackStack).
 * MVP: commit() executes ops synchronously (no Handler-deferred execPendingActions).
 *********************************************************************************/
#include <fragment/fragmenttransaction.h>
#include <fragment/fragmentstate.h>
#include <unordered_map>
#include <fragment/fragmentmanager.h>
namespace cdroid{
namespace fragment{

class BackStackRecord : public FragmentTransaction, public FragmentManager::OpGenerator{
public:
    explicit BackStackRecord(FragmentManager* manager);

    // commit()/commitAllowingStateLoss(): DEFERRED — enqueue for the next main-loop iteration
    // (androidx commitInternal -> enqueueAction). commitNow()/commitNowAllowingStateLoss():
    // SYNCHRONOUS — drain pending then execute this record inline (androidx execSingleAction).
    int commit() override;
    int commitAllowingStateLoss() override;
    void commitNow() override;
    void commitNowAllowingStateLoss() override;

    // OpGenerator: append this record as a forward (non-pop) batch entry.
    bool generateOps(std::vector<BackStackRecord*>& records,
                     std::vector<bool>& isRecordPop) override;

    // Apply all recorded ops (add/remove/hide/...).
    void executeOps();
    // Reverse all recorded ops (used by popBackStack).
    void executePopOps();

    int getIndex() const { return mIndex; }
    void setIndex(int index){ mIndex = index; }
    const std::string& getName() const { return mName; }
    // Capture this record's ops into a BackStackRecordState (androidx BackStackRecordState(BackStackRecord)).
    BackStackRecordState captureState() const;
    // Rebuild this record's ops from a BackStackRecordState, resolving each op's fragment by mWho
    // from `fragments` (androidx BackStackRecordState.fillInBackStackRecord).
    void restoreFromState(const BackStackRecordState& state,
                          const std::unordered_map<std::string, Fragment*>& fragments);
    // Set while saveBackStack pops this record: FragmentStateManager saves its fragments' state
    // into FragmentManager.mSavedState instead of discarding it (androidx BackStackRecord.mBeingSaved).
    bool mBeingSaved = false;

private:
    FragmentManager* mManager;
    int mIndex = -1;
};

}//namespace fragment
}//namespace cdroid
#endif
