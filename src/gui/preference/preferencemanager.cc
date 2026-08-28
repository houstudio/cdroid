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
#include <preference/preferencemanager.h>
#include <preference/preference.h>
#include <preference/preferencedatastore.h>
#include <preference/preferencescreen.h>
#include <preference/preferenceinflater.h>
#include <preference/twostatepreference.h>
#include <preference/dropdownpreference.h>
#include <widget/R.h>

namespace cdroid {

PreferenceManager::PreferenceManager(Context& context)
    : mContext(context) {
    setSharedPreferencesName(getDefaultSharedPreferencesName(context));
}

std::shared_ptr<SharedPreferences> PreferenceManager::getDefaultSharedPreferences(Context& context) {
    return context.getSharedPreferences(getDefaultSharedPreferencesName(context),
            getDefaultSharedPreferencesMode());
}

std::string PreferenceManager::getDefaultSharedPreferencesName(Context& context) {
    return context.getPackageName() + "_preferences";
}

int PreferenceManager::getDefaultSharedPreferencesMode() {
    return Context::MODE_PRIVATE;
}

void PreferenceManager::setDefaultValues(Context& context, int resId, bool readAgain) {
    // Use the default shared preferences name and mode
    setDefaultValues(context, getDefaultSharedPreferencesName(context),
            getDefaultSharedPreferencesMode(), resId, readAgain);
}

void PreferenceManager::setDefaultValues(Context& context,
        const std::string& sharedPreferencesName, int sharedPreferencesMode,
        int resId, bool readAgain) {
    auto defaultValueSp = context.getSharedPreferences(KEY_HAS_SET_DEFAULT_VALUES,
            Context::MODE_PRIVATE);

    if (readAgain || !defaultValueSp->getBoolean(KEY_HAS_SET_DEFAULT_VALUES, false)) {
        PreferenceManager pm(context);
        pm.setSharedPreferencesName(sharedPreferencesName);
        pm.setSharedPreferencesMode(sharedPreferencesMode);
        pm.inflateFromResource(context, resId, nullptr);

        defaultValueSp->edit().putBoolean(KEY_HAS_SET_DEFAULT_VALUES, true).apply();
    }
}

PreferenceScreen* PreferenceManager::inflateFromResource(Context& context, int resId,
        PreferenceScreen* rootPreferences) {
    // Block commits
    setNoCommit(true);

    PreferenceInflater inflater(context, *this);
    rootPreferences = static_cast<PreferenceScreen*>(inflater.inflate(resId, rootPreferences));
    rootPreferences->onAttachedToHierarchy(*this);

    // Unblock commits
    setNoCommit(false);

    return rootPreferences;
}

PreferenceScreen* PreferenceManager::createPreferenceScreen(Context& context) {
    PreferenceScreen* preferenceScreen = new PreferenceScreen(context, AttributeSet());
    preferenceScreen->onAttachedToHierarchy(*this);
    return preferenceScreen;
}

int64_t PreferenceManager::getNextId() {
    return mNextId++;
}

const std::string& PreferenceManager::getSharedPreferencesName() const {
    return mSharedPreferencesName;
}

void PreferenceManager::setSharedPreferencesName(const std::string& sharedPreferencesName) {
    mSharedPreferencesName = sharedPreferencesName;
    mSharedPreferences.reset();
}

int PreferenceManager::getSharedPreferencesMode() const {
    return mSharedPreferencesMode;
}

void PreferenceManager::setSharedPreferencesMode(int sharedPreferencesMode) {
    mSharedPreferencesMode = sharedPreferencesMode;
    mSharedPreferences.reset();
}

void PreferenceManager::setStorageDefault() {
    mStorage = STORAGE_DEFAULT;
    mSharedPreferences.reset();
}

void PreferenceManager::setStorageDeviceProtected() {
    // CDROID has no device-protected storage; the call is kept for API parity
    // and behaves like the pre-N AOSP branch (no-op below API 24).
    mStorage = STORAGE_DEVICE_PROTECTED;
    mSharedPreferences.reset();
}

bool PreferenceManager::isStorageDefault() const {
    return mStorage == STORAGE_DEFAULT;
}

bool PreferenceManager::isStorageDeviceProtected() const {
    return mStorage == STORAGE_DEVICE_PROTECTED;
}

void PreferenceManager::setPreferenceDataStore(PreferenceDataStore* dataStore) {
    mPreferenceDataStore = dataStore;
}

PreferenceDataStore* PreferenceManager::getPreferenceDataStore() const {
    return mPreferenceDataStore;
}

SharedPreferences* PreferenceManager::getSharedPreferences() const {
    if (getPreferenceDataStore() != nullptr) {
        return nullptr;
    }

    if (!mSharedPreferences) {
        // CDROID has no separate device-protected storage context, so both
        // storage modes resolve to the host context (AOSP pre-N behavior).
        mSharedPreferences = mContext.getSharedPreferences(mSharedPreferencesName,
                mSharedPreferencesMode);
    }

    return mSharedPreferences.get();
}

PreferenceScreen* PreferenceManager::getPreferenceScreen() const {
    return mPreferenceScreen;
}

PreferenceManager::~PreferenceManager() {
    delete mPreferenceScreen;
}

bool PreferenceManager::setPreferences(PreferenceScreen* preferenceScreen) {
    if (preferenceScreen != mPreferenceScreen) {
        if (mPreferenceScreen != nullptr) {
            mPreferenceScreen->onDetached();
            delete mPreferenceScreen;   // CDROID owns the tree (AOSP: GC)
        }
        mPreferenceScreen = preferenceScreen;
        return true;
    }

    return false;
}

Preference* PreferenceManager::findPreference(const std::string& key) const {
    if (mPreferenceScreen == nullptr) {
        return nullptr;
    }

    return mPreferenceScreen->findPreference(key);
}

SharedPreferences::Editor* PreferenceManager::getEditor() const {
    if (mPreferenceDataStore != nullptr) {
        return nullptr;
    }

    if (mNoCommit) {
        if (mEditor == nullptr) {
            mEditor = &getSharedPreferences()->edit();
        }
        return mEditor;
    } else {
        return &getSharedPreferences()->edit();
    }
}

bool PreferenceManager::shouldCommit() const {
    return !mNoCommit;
}

void PreferenceManager::setNoCommit(bool noCommit) {
    if (!noCommit && mEditor) {
        mEditor->apply();
        mEditor = nullptr;
    }
    mNoCommit = noCommit;
}

Context& PreferenceManager::getContext() const {
    return mContext;
}

PreferenceManager::PreferenceComparisonCallback
PreferenceManager::getPreferenceComparisonCallback() const {
    return mPreferenceComparisonCallback;
}

void PreferenceManager::setPreferenceComparisonCallback(
        const PreferenceComparisonCallback& preferenceComparisonCallback) {
    mPreferenceComparisonCallback = preferenceComparisonCallback;
}

PreferenceManager::OnDisplayPreferenceDialogListener
PreferenceManager::getOnDisplayPreferenceDialogListener() const {
    return mOnDisplayPreferenceDialogListener;
}

void PreferenceManager::setOnDisplayPreferenceDialogListener(
        const OnDisplayPreferenceDialogListener& onDisplayPreferenceDialogListener) {
    mOnDisplayPreferenceDialogListener = onDisplayPreferenceDialogListener;
}

void PreferenceManager::showDialog(Preference& preference) {
    if (mOnDisplayPreferenceDialogListener) {
        mOnDisplayPreferenceDialogListener(preference);
    }
}

void PreferenceManager::setOnPreferenceTreeClickListener(
        const OnPreferenceTreeClickListener& listener) {
    mOnPreferenceTreeClickListener = listener;
}

PreferenceManager::OnPreferenceTreeClickListener
PreferenceManager::getOnPreferenceTreeClickListener() const {
    return mOnPreferenceTreeClickListener;
}

void PreferenceManager::setOnNavigateToScreenListener(
        const OnNavigateToScreenListener& listener) {
    mOnNavigateToScreenListener = listener;
}

PreferenceManager::OnNavigateToScreenListener
PreferenceManager::getOnNavigateToScreenListener() const {
    return mOnNavigateToScreenListener;
}

PreferenceManager::PreferenceComparisonCallback
PreferenceManager::SimplePreferenceComparisonCallback() {
    PreferenceComparisonCallback callback;
    callback.arePreferenceItemsTheSame = [](const Preference& p1, const Preference& p2) {
        return p1.getId() == p2.getId();
    };
    callback.arePreferenceContentsTheSame = [](const Preference& p1, const Preference& p2) {
        if (p1.getPreferenceClassName() != p2.getPreferenceClassName()) {
            return false;
        }
        if (&p1 == &p2 && p1.wasDetached()) {
            // Defensively handle the case where a preference was removed, updated and re-added.
            // Hopefully this is rare.
            return false;
        }
        if (p1.getTitle() != p2.getTitle()) {
            return false;
        }
        if (p1.getSummary() != p2.getSummary()) {
            return false;
        }
        const Drawable* p1Icon = p1.getIcon();
        const Drawable* p2Icon = p2.getIcon();
        if (p1Icon != p2Icon && (p1Icon == nullptr || p1Icon != p2Icon)) {
            return false;
        }
        if (p1.isEnabled() != p2.isEnabled()) {
            return false;
        }
        if (p1.isSelectable() != p2.isSelectable()) {
            return false;
        }
        const TwoStatePreference* t1 = dynamic_cast<const TwoStatePreference*>(&p1);
        const TwoStatePreference* t2 = dynamic_cast<const TwoStatePreference*>(&p2);
        if (t1 != nullptr && t1->isChecked() != t2->isChecked()) {
            return false;
        }
        if (dynamic_cast<const DropDownPreference*>(&p1) != nullptr && &p1 != &p2) {
            // Different object, must re-bind spinner adapter
            return false;
        }

        return true;
    };
    return callback;
}

} // namespace cdroid
