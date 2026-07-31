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
#ifndef __FRAGMENTMANAGERVIEWMODEL_H__
#define __FRAGMENTMANAGERVIEWMODEL_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentManagerViewModel. The always up-to-date
 * view of a FragmentManager's non-config state: retained Fragments, child
 * FragmentManagerViewModels and per-Fragment ViewModelStores. The deprecated
 * getSnapshot/restoreFromSnapshot path is omitted (only mStateAutomaticallySaved
 * = true mode is supported).
 *********************************************************************************/
#include <string>
#include <vector>
#include <unordered_map>
#include <lifecycle/viewmodel.h>
namespace cdroid{
namespace lifecycle{ class ViewModelStore; }
namespace fragment{

class Fragment;

class FragmentManagerViewModel : public lifecycle::ViewModel{
public:
    explicit FragmentManagerViewModel(bool stateAutomaticallySaved);
    ~FragmentManagerViewModel() override;

    static FragmentManagerViewModel* getInstance(lifecycle::ViewModelStore* viewModelStore);

    void setIsStateSaved(bool isStateSaved){ mIsStateSaved = isStateSaved; }
    bool isCleared() const { return mHasBeenCleared; }

    void addRetainedFragment(Fragment* fragment);
    Fragment* findRetainedFragmentByWho(const std::string& who) const;
    std::vector<Fragment*> getRetainedFragments() const;
    bool shouldDestroy(Fragment* fragment) const;
    void removeRetainedFragment(Fragment* fragment);

    FragmentManagerViewModel* getChildNonConfig(Fragment* f);
    lifecycle::ViewModelStore* getViewModelStore(Fragment* f);
    void clearNonConfigState(Fragment* f, bool destroyChildNonConfig);
    void clearNonConfigState(const std::string& who, bool destroyChildNonConfig);

protected:
    void onCleared() override;

private:
    void clearNonConfigStateInternal(const std::string& who, bool destroyChildNonConfig);

    bool mStateAutomaticallySaved;
    bool mHasBeenCleared = false;
    bool mHasSavedSnapshot = false;
    bool mIsStateSaved = false;
    std::unordered_map<std::string, Fragment*> mRetainedFragments;            // borrowed
    std::unordered_map<std::string, FragmentManagerViewModel*> mChildNonConfigs; // owned
    std::unordered_map<std::string, lifecycle::ViewModelStore*> mViewModelStores; // owned
};

}//namespace fragment
}//namespace cdroid
#endif
