#include <fragment/fragmenttransaction.h>
#include <fragment/fragment.h>
namespace cdroid{
namespace fragment{

FragmentTransaction& FragmentTransaction::addOp(const Op& op){
    Op o = op;
    if(mEnterAnim || mExitAnim || mPopEnterAnim || mPopExitAnim){
        if(!o.mEnterAnim) o.mEnterAnim = mEnterAnim;
        if(!o.mExitAnim) o.mExitAnim = mExitAnim;
        if(!o.mPopEnterAnim) o.mPopEnterAnim = mPopEnterAnim;
        if(!o.mPopExitAnim) o.mPopExitAnim = mPopExitAnim;
    }
    mOps.push_back(o);
    return *this;
}

FragmentTransaction& FragmentTransaction::add(Fragment* fragment, const std::string& tag){
    Op op; op.mCmd = OP_ADD; op.mFragment = fragment;
    if(!tag.empty()) fragment->mTag = tag;
    return addOp(op);
}

FragmentTransaction& FragmentTransaction::add(int containerViewId, Fragment* fragment){
    return add(containerViewId, fragment, std::string());
}

FragmentTransaction& FragmentTransaction::add(int containerViewId, Fragment* fragment, const std::string& tag){
    fragment->mContainerId = containerViewId;
    if(fragment->mFragmentId == 0) fragment->mFragmentId = containerViewId;
    if(!tag.empty()) fragment->mTag = tag;
    Op op; op.mCmd = OP_ADD; op.mFragment = fragment;
    return addOp(op);
}

FragmentTransaction& FragmentTransaction::replace(int containerViewId, Fragment* fragment){
    return replace(containerViewId, fragment, std::string());
}

FragmentTransaction& FragmentTransaction::replace(int containerViewId, Fragment* fragment, const std::string& tag){
    fragment->mContainerId = containerViewId;
    if(fragment->mFragmentId == 0) fragment->mFragmentId = containerViewId;
    if(!tag.empty()) fragment->mTag = tag;
    Op op; op.mCmd = OP_REPLACE; op.mFragment = fragment;
    return addOp(op);
}

FragmentTransaction& FragmentTransaction::remove(Fragment* fragment){ Op op; op.mCmd = OP_REMOVE; op.mFragment = fragment; return addOp(op); }
FragmentTransaction& FragmentTransaction::hide(Fragment* fragment){ Op op; op.mCmd = OP_HIDE; op.mFragment = fragment; return addOp(op); }
FragmentTransaction& FragmentTransaction::show(Fragment* fragment){ Op op; op.mCmd = OP_SHOW; op.mFragment = fragment; return addOp(op); }
FragmentTransaction& FragmentTransaction::detach(Fragment* fragment){ Op op; op.mCmd = OP_DETACH; op.mFragment = fragment; return addOp(op); }
FragmentTransaction& FragmentTransaction::attach(Fragment* fragment){ Op op; op.mCmd = OP_ATTACH; op.mFragment = fragment; return addOp(op); }

FragmentTransaction& FragmentTransaction::setPrimaryNavigationFragment(Fragment* fragment){
    Op op; op.mCmd = OP_SET_PRIMARY_NAV; op.mFragment = fragment; return addOp(op);
}
FragmentTransaction& FragmentTransaction::setMaxLifecycle(Fragment* fragment, lifecycle::Lifecycle::State /*state*/){
    Op op; op.mCmd = OP_SET_MAX_LIFECYCLE; op.mFragment = fragment; return addOp(op);
}

FragmentTransaction& FragmentTransaction::addToBackStack(const std::string& name){
    mAddToBackStack = true;
    mName = name;
    return *this;
}
FragmentTransaction& FragmentTransaction::disallowAddToBackStack(){ mAddToBackStack = false; return *this; }
FragmentTransaction& FragmentTransaction::setReorderingAllowed(bool reorderingAllowed){ mReorderingAllowed = reorderingAllowed; return *this; }
FragmentTransaction& FragmentTransaction::setCustomAnimations(int enterAnim, int exitAnim, int popEnterAnim, int popExitAnim){
    mEnterAnim = enterAnim; mExitAnim = exitAnim; mPopEnterAnim = popEnterAnim; mPopExitAnim = popExitAnim;
    return *this;
}

}//namespace fragment
}//namespace cdroid
