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
#include <fragment/dialogfragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <navigation/navhostfragment.h>
#include <navigation/navcontroller.h>
#include <core/bundle.h>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

// Saved-state keys (androidx DialogFragment.java:341-350).
static const char* SAVED_DIALOG_STATE_TAG = "android:savedDialogState";
static const char* SAVED_STYLE            = "android:style";
static const char* SAVED_THEME            = "android:theme";
static const char* SAVED_CANCELABLE       = "android:cancelable";
static const char* SAVED_SHOWS_DIALOG     = "android:showsDialog";
static const char* SAVED_BACK_STACK_ID    = "android:backStackId";

DialogFragment::DialogFragment(){
}

DialogFragment::~DialogFragment(){
    // Dialog has a protected dtor — cleanup via dismiss(), not delete.
    if(mDialog) mDialog->dismiss();
    delete mDialogState;
}

// --- Style / theme / shows-dialog / cancelable (androidx :471-686) ---

void DialogFragment::setStyle(int style, int theme){
    mStyle = style;
    if(style == STYLE_NO_FRAME || style == STYLE_NO_INPUT){
        mTheme = 0; // androidx forces android.R.style.Theme_Panel; CDROID has no Theme_Panel → 0.
    }
    if(theme != 0){
        mTheme = theme;
    }
}

void DialogFragment::setShowsDialog(bool showsDialog){
    mShowsDialog = showsDialog;
}

void DialogFragment::setCancelable(bool cancelable){
    mCancelable = cancelable;
    if(mDialog) mDialog->setCancelable(cancelable);
}

// --- show / dismiss (androidx :496-595) ---

void DialogFragment::show(FragmentManager* manager, const std::string& tag){
    // androidx show(FragmentManager, String) :496 — add the fragment; the dialog is shown in onStart.
    mDismissed = false;
    mShownByMe = true;
    FragmentTransaction* t = manager->beginTransaction();
    t->add(this, tag);
    t->commit();
}

int DialogFragment::show(FragmentTransaction* transaction, const std::string& tag){
    // androidx show(FragmentTransaction, String) :514 — add on the caller's transaction.
    mDismissed = false;
    mShownByMe = true;
    mViewDestroyed = false;
    transaction->add(this, tag);
    mBackStackId = transaction->commit();
    return mBackStackId;
}

void DialogFragment::showNow(FragmentManager* manager, const std::string& tag){
    // androidx showNow :534 — commitNow for synchronous add.
    mDismissed = false;
    mShownByMe = true;
    FragmentTransaction* t = manager->beginTransaction();
    t->add(this, tag);
    t->commitNow();
}

Dialog* DialogFragment::requireDialog(){
    Dialog* d = getDialog();
    if(d == nullptr){
        LOGE("DialogFragment %s does not have a Dialog.", mWho.c_str());
    }
    return d;
}

void DialogFragment::dismiss(){
    dismissInternal(false, false);
}

void DialogFragment::dismissAllowingStateLoss(){
    dismissInternal(true, false);
}

void DialogFragment::dismissInternal(bool /*allowStateLoss*/, bool fromOnDismiss){
    // androidx dismissInternal :572. Idempotent (no-op if already dismissed).
    if(mDismissed) return;
    mDismissed = true;
    mShownByMe = false;
    if(mDialog != nullptr){
        mDialog->setOnCancelListener(nullptr);
        mDialog->setOnDismissListener(nullptr);
        mDialog->dismiss();
        if(!fromOnDismiss){
            onDismiss(mDialog);
        }
    }
    mViewDestroyed = true;
    // androidx DialogFragmentNavigator uses a lifecycle observer to detect dialog dismissal →
    // pop the Navigator state. CDROID: pop the NavController directly here (the DialogFragment is
    // a child of the NavHostFragment, so the parent chain gives us the NavController).
    NavHostFragment* host = dynamic_cast<NavHostFragment*>(getParentFragment());
    if(host && host->getNavController()){
        host->getNavController()->popBackStack();
    }
    // Remove from FragmentManager. If shown via show(transaction, tag) with a back stack entry,
    // pop it; otherwise remove directly.
    if(mFragmentManager){
        if(mBackStackId >= 0){
            mFragmentManager->popBackStackImmediate(mTag, FragmentManager::POP_BACK_STACK_INCLUSIVE);
            mBackStackId = -1;
        } else {
            FragmentTransaction* t = mFragmentManager->beginTransaction();
            t->remove(this);
            t->commitAllowingStateLoss();
        }
    }
}

// --- Lifecycle (androidx :748-1041) ---

void DialogFragment::onCreate(Bundle* savedInstanceState){
    Fragment::onCreate(savedInstanceState);
    mShowsDialog = (mContainerId == 0); // androidx :759 — embedded fragment (containerId != 0) → no dialog.
    if(savedInstanceState){
        mStyle       = savedInstanceState->getValue<int>(SAVED_STYLE, (int)STYLE_NORMAL);
        mTheme       = savedInstanceState->getValue<int>(SAVED_THEME, 0);
        mCancelable  = savedInstanceState->getValue<bool>(SAVED_CANCELABLE, true);
        mShowsDialog = savedInstanceState->getValue<bool>(SAVED_SHOWS_DIALOG, mShowsDialog);
        mBackStackId = savedInstanceState->getValue<int>(SAVED_BACK_STACK_ID, -1);
        Bundle* ds = savedInstanceState->getBundle(SAVED_DIALOG_STATE_TAG);
        if(ds) mDialogState = new Bundle(*ds);
    }
}

void DialogFragment::onStart(){
    Fragment::onStart();
    // androidx: the dialog is lazily created in prepareDialog (triggered by onGetLayoutInflater in
    // AndroidX). CDROID's simplified model creates it here, right before show.
    if(mShowsDialog) prepareDialog(nullptr);
    if(mDialog != nullptr){
        mViewDestroyed = false;
        mDialog->show();
    }
}

void DialogFragment::onStop(){
    Fragment::onStop();
    if(mDialog != nullptr){
        mDialog->hide();
    }
}

void DialogFragment::onDestroyView(){
    Fragment::onDestroyView();
    if(mDialog != nullptr){
        mViewDestroyed = true; // prevent onDismiss → re-enter dismissInternal
        mDialog->setOnDismissListener(nullptr);
        mDialog->dismiss();
        if(!mDismissed){
            onDismiss(mDialog);
        }
        mDialog = nullptr; // Dialog cleanup is via dismiss() above; dtor is protected.
        mDialogCreated = false;
    }
}

void DialogFragment::onSaveInstanceState(Bundle* outState){
    Fragment::onSaveInstanceState(outState);
    if(outState && mDialog){
        // androidx :1003 — save the dialog's own state (best-effort; CDROID Dialog lacks onSaveInstanceState).
        if(mStyle != STYLE_NORMAL) outState->putInt(SAVED_STYLE, mStyle);
        if(mTheme != 0)            outState->putInt(SAVED_THEME, mTheme);
        if(!mCancelable)           outState->putBoolean(SAVED_CANCELABLE, mCancelable);
        if(!mShowsDialog)          outState->putBoolean(SAVED_SHOWS_DIALOG, mShowsDialog);
        if(mBackStackId != -1)     outState->putInt(SAVED_BACK_STACK_ID, mBackStackId);
    }
}

// --- prepareDialog / setupDialog (androidx :845-937) ---

void DialogFragment::prepareDialog(Bundle* savedInstanceState){
    // androidx prepareDialog :909 — lazily create + configure the Dialog. Called from onStart.
    if(!mShowsDialog) return;
    if(!mDialogCreated){
        mCreatingDialog = true;
        mDialog = onCreateDialog(savedInstanceState);
        mCreatingDialog = false;
        if(mShowsDialog){
            setupDialog(mDialog, mStyle);
            mDialog->setCancelable(mCancelable);
            // Wire the dialog's cancel/dismiss listeners back to this fragment (androidx :927-928
            // uses indirection lambdas to guard against mDialog being null during the callback).
            mDialog->setOnCancelListener([this](Dialog&){ if(mDialog) onCancel(mDialog); });
            mDialog->setOnDismissListener([this](Dialog&){ if(mDialog && !mViewDestroyed) onDismiss(mDialog); });
            mDialogCreated = true;
        } else {
            mDialog = nullptr;
        }
    }
    if(mDialogState){
        // Restore the dialog's saved state (best-effort; CDROID Dialog lacks onRestoreInstanceState).
    }
}

void DialogFragment::setupDialog(Dialog* dialog, int style){
    // androidx setupDialog :845 — apply window features per style. CDROID's Dialog/Window API
    // differs from Android's (no requestWindowFeature), so this is a best-effort no-op for now.
    // STYLE_NO_INPUT: add FLAG_NOT_FOCUSABLE | FLAG_NOT_TOUCHABLE (not available on CDROID Window).
    // STYLE_NO_FRAME/NO_TITLE: requestWindowFeature(FEATURE_NO_TITLE) (not available on CDROID).
    (void)dialog;
    (void)style;
}

}//namespace fragment
}//namespace cdroid
