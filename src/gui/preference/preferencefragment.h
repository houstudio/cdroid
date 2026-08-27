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
#ifndef __CDROID_PREFERENCE_FRAGMENT_H__
#define __CDROID_PREFERENCE_FRAGMENT_H__

#include <string>
#include <functional>
#include <memory>
#include <fragment/fragment.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <preference/preference.h>
#include <preference/preferencemanager.h>

namespace cdroid {

class Bundle;
class Drawable;
class PreferenceScreen;
class DialogPreference;
class PreferenceFragment;

/**
 * Interface that the fragment's containing activity should implement to be
 * able to process preference items that wish to switch to a specified fragment.
 */
class OnPreferenceStartFragmentCallback {
public:
    virtual ~OnPreferenceStartFragmentCallback() = default;
    virtual bool onPreferenceStartFragment(PreferenceFragment& caller, Preference& pref) = 0;
};

/**
 * Interface that the fragment's containing activity should implement to be
 * able to process preference items that wish to switch to a new screen of
 * preferences.
 */
class OnPreferenceStartScreenCallback {
public:
    virtual ~OnPreferenceStartScreenCallback() = default;
    virtual bool onPreferenceStartScreen(PreferenceFragment& caller, PreferenceScreen& pref) = 0;
};

/**
 * Interface that the fragment's containing activity should implement to be
 * able to process preference items that wish to display a dialog.
 */
class OnPreferenceDisplayDialogCallback {
public:
    virtual ~OnPreferenceDisplayDialogCallback() = default;
    virtual bool onPreferenceDisplayDialog(PreferenceFragment& caller, Preference& pref) = 0;
};

/**
 * Port of androidx.preference.PreferenceFragmentCompat (the Compat suffix is
 * dropped — CDROID has no legacy framework fragment to be compatible with).
 * A PreferenceFragment is the entry point to using the Preference library.
 * This Fragment displays a hierarchy of Preference objects to the user. It
 * also handles persisting values to the device.
 */
class PreferenceFragment : public fragment::Fragment {
public:
    /**
     * Fragment argument used to specify the tag of the desired root
     * PreferenceScreen object.
     */
    static constexpr const char* ARG_PREFERENCE_ROOT =
            "androidx.preference.PreferenceFragment.PREFERENCE_ROOT";

    void onCreate(Bundle* savedInstanceState) override;
    View* onCreateView(LayoutInflater* inflater, ViewGroup* container,
            Bundle* savedInstanceState) override;

    void setDivider(Drawable* divider);
    void setDividerHeight(int height);

    void onViewCreated(View* view, Bundle* savedInstanceState) override;
    void onStart() override;
    void onStop() override;
    void onDestroyView() override;
    void onSaveInstanceState(Bundle* outState) override;

    PreferenceManager* getPreferenceManager() const;

    /** Gets the root of the preference hierarchy that this fragment is showing. */
    PreferenceScreen* getPreferenceScreen() const;

    /** Sets the root of the preference hierarchy that this fragment is showing. */
    void setPreferenceScreen(PreferenceScreen* preferenceScreen);

    /** Inflates the given XML resource and adds the preference hierarchy. */
    void addPreferencesFromResource(int preferencesResId);

    /**
     * Inflates the given XML resource and replaces the current preference
     * hierarchy with the hierarchy rooted at key.
     */
    void setPreferencesFromResource(int preferencesResId, const std::string& key);

    bool onPreferenceTreeClick(Preference& preference);
    void onNavigateToScreen(PreferenceScreen& preferenceScreen);
    void onDisplayPreferenceDialog(Preference& preference);

    Preference* findPreference(const std::string& key) const;

    RecyclerView* getListView() const;

    /**
     * Creates the RecyclerView used to display the preferences.
     */
    virtual RecyclerView* onCreateRecyclerView(LayoutInflater* inflater, ViewGroup* parent,
            Bundle* savedInstanceState);

    /**
     * Called from onCreateRecyclerView to create the LayoutManager.
     */
    virtual RecyclerView::LayoutManager* onCreateLayoutManager();

    /**
     * Creates the root adapter.
     */
    virtual RecyclerView::Adapter* onCreateAdapter(PreferenceScreen* preferenceScreen);

    void scrollToPreference(const std::string& key);
    void scrollToPreference(Preference* preference);

protected:
    /**
     * Called during onCreate to supply the preferences for this fragment.
     */
    virtual void onCreatePreferences(Bundle* savedInstanceState, const std::string& rootKey) = 0;

    virtual void onBindPreferences();
    virtual void onUnbindPreferences();

    /**
     * A wrapper for getParentFragment which is v17+. Used by the leanback
     * preference lib.
     */
    virtual fragment::Fragment* getCallbackFragment();

private:
    void requirePreferenceManager();
    void postBindPreferences();
    void bindPreferences();
    void unbindPreferences();
    void scrollToPreferenceInternal(Preference* preference, const std::string& key);

    static constexpr int MSG_BIND_PREFERENCES = 1;
    static constexpr const char* PREFERENCES_TAG = "android:preferences";
    static constexpr const char* DIALOG_FRAGMENT_TAG = "androidx.preference.PreferenceFragment.DIALOG";

    class DividerDecoration;
    class ScrollToPreferenceObserver;

    DividerDecoration* mDividerDecoration = nullptr;
    PreferenceManager* mPreferenceManager = nullptr;
    RecyclerView* mList = nullptr;
    bool mHavePrefs = false;
    bool mInitDone = false;
    int mLayoutResId = 0 /* internal::R::layout::preference_list_fragment */;
    Runnable mSelectPreferenceRunnable;
    std::unique_ptr<Handler> mHandler;
    Runnable mRequestFocus;

public:
    ~PreferenceFragment() override;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_FRAGMENT_H__
