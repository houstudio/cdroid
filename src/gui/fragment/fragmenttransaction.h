#ifndef __FRAGMENTTRANSACTION_H__
#define __FRAGMENTTRANSACTION_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentTransaction (abstract base). Records a
 * list of Ops (add/replace/remove/hide/show/attach/detach). Concrete commit/
 * executeOps is implemented by BackStackRecord.
 *********************************************************************************/
#include <vector>
#include <string>
#include <lifecycle/lifecycle.h>
namespace cdroid{
namespace fragment{

class Fragment;
class FragmentFactory;

class FragmentTransaction{
public:
    // Op commands (verbatim from androidx).
    static const int OP_NULL = 0;
    static const int OP_ADD = 1;
    static const int OP_REPLACE = 2;
    static const int OP_REMOVE = 3;
    static const int OP_HIDE = 4;
    static const int OP_SHOW = 5;
    static const int OP_DETACH = 6;
    static const int OP_ATTACH = 7;
    static const int OP_SET_PRIMARY_NAV = 8;
    static const int OP_UNSET_PRIMARY_NAV = 9;
    static const int OP_SET_MAX_LIFECYCLE = 10;

    struct Op{
        int mCmd = OP_NULL;
        Fragment* mFragment = nullptr;
        Fragment* mOldFragment = nullptr; // OP_REPLACE: the fragment removed, restored on pop
        int mEnterAnim = 0;
        int mExitAnim = 0;
        int mPopEnterAnim = 0;
        int mPopExitAnim = 0;
    };

    virtual ~FragmentTransaction() = default;

    FragmentTransaction& add(Fragment* fragment, const std::string& tag = "");
    FragmentTransaction& add(int containerViewId, Fragment* fragment);
    FragmentTransaction& add(int containerViewId, Fragment* fragment, const std::string& tag);
    FragmentTransaction& replace(int containerViewId, Fragment* fragment);
    FragmentTransaction& replace(int containerViewId, Fragment* fragment, const std::string& tag);
    FragmentTransaction& remove(Fragment* fragment);
    FragmentTransaction& hide(Fragment* fragment);
    FragmentTransaction& show(Fragment* fragment);
    FragmentTransaction& detach(Fragment* fragment);
    FragmentTransaction& attach(Fragment* fragment);
    FragmentTransaction& setPrimaryNavigationFragment(Fragment* fragment);
    FragmentTransaction& setMaxLifecycle(Fragment* fragment, lifecycle::Lifecycle::State state);
    FragmentTransaction& addToBackStack(const std::string& name);
    FragmentTransaction& disallowAddToBackStack();
    FragmentTransaction& setReorderingAllowed(bool reorderingAllowed);
    FragmentTransaction& setCustomAnimations(int enterAnim, int exitAnim,
                                             int popEnterAnim = 0, int popExitAnim = 0);

    bool isEmpty() const { return mOps.empty(); }

    // Concrete subclasses (BackStackRecord) implement commit semantics.
    virtual int commit() = 0;
    virtual int commitAllowingStateLoss() = 0;
    virtual void commitNow() = 0;
    virtual void commitNowAllowingStateLoss() = 0;

protected:
    FragmentTransaction& addOp(const Op& op);
    std::vector<Op> mOps;
    bool mAddToBackStack = false;
    std::string mName;
    bool mReorderingAllowed = false;
    int mEnterAnim = 0, mExitAnim = 0, mPopEnterAnim = 0, mPopExitAnim = 0;
};

}//namespace fragment
}//namespace cdroid
#endif
