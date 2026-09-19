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
#include <content/typedarray.h>
#include <preference/preferencefragment.h>
#include <preference/preferencegroupadapter.h>
#include <preference/preferenceviewholder.h>
#include <fragment/fragmentmanager.h>
#include <preference/preferencescreen.h>
#include <preference/edittextpreference.h>
#include <preference/listpreference.h>
#include <preference/multiselectlistpreference.h>
#include <preference/edittextpreferencedialogfragment.h>
#include <preference/listpreferencedialogfragment.h>
#include <preference/multiselectlistpreferencedialogfragment.h>
#include <preference/androidresources.h>
#include <core/handler.h>
#include <core/looper.h>
#include <core/bundle.h>
#include <view/layoutinflater.h>
#include <view/viewgroup.h>
#include <widget/linearlayout.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <drawable/drawable.h>
#include <core/rect.h>
#include <porting/cdlog.h>

namespace cdroid {

/**
 * Port of the PreferenceFragment.DividerDecoration — draws the divider
 * between preference rows.
 */
class PreferenceFragment::DividerDecoration : public RecyclerView::ItemDecoration {
public:
    // CDROID ownership: the RecyclerView owns (and deletes) its decorations,
    // so the divider handed in by onCreateView must die with the decoration —
    // without this dtor it leaked one divider clone per fragment view
    // creation (valgrind: GradientState::newDrawable + its nested shape).
    ~DividerDecoration() override { delete mDivider; }
    Drawable* mDivider = nullptr;
    int mDividerHeight = 0;
    bool mAllowDividerAfterLastItem = true;
    RecyclerView* mList = nullptr;

    void onDrawOver(Canvas& c, RecyclerView& parent, RecyclerView::State& state) override {
        if (mDivider == nullptr) {
            return;
        }
        const int childCount = parent.getChildCount();
        const int width = parent.getWidth();
        for (int childViewIndex = 0; childViewIndex < childCount; childViewIndex++) {
            View* view = parent.getChildAt(childViewIndex);
            if (shouldDrawDividerBelow(view, parent)) {
                int top = (int)view->getY() + view->getHeight();
                // CDROID Rect is (x, y, w, h) — AOSP passes (l, t, r, b) here;
                // porting kept "top + mDividerHeight" as the 4th arg, which made
                // the divider slab grow `top` px tall instead of 1px (gray bands).
                mDivider->setBounds(0, top, width, mDividerHeight);
                mDivider->draw(c);
            }
        }
    }

    void getItemOffsets(Rect& outRect, View& view, RecyclerView& parent,
            RecyclerView::State& state) override {
        if (shouldDrawDividerBelow(&view, parent)) {
            outRect.set(0, 0, 0, mDividerHeight);
        }
    }

    bool shouldDrawDividerBelow(View* view, RecyclerView& parent) {
        RecyclerView::ViewHolder* holder = parent.getChildViewHolder(view);
        auto* prefHolder = dynamic_cast<PreferenceViewHolder*>(holder);
        const bool dividerAllowedBelow =
                prefHolder != nullptr && prefHolder->isDividerAllowedBelow();
        if (!dividerAllowedBelow) {
            return false;
        }
        bool nextAllowed = mAllowDividerAfterLastItem;
        int index = parent.indexOfChild(view);
        if (index < parent.getChildCount() - 1) {
            View* nextView = parent.getChildAt(index + 1);
            RecyclerView::ViewHolder* nextHolder = parent.getChildViewHolder(nextView);
            auto* nextPrefHolder = dynamic_cast<PreferenceViewHolder*>(nextHolder);
            nextAllowed = nextPrefHolder != nullptr && nextPrefHolder->isDividerAllowedAbove();
        }
        return nextAllowed;
    }

    void setDivider(Drawable* divider) {
        if (divider != nullptr) {
            mDividerHeight = divider->getIntrinsicHeight();
        } else {
            mDividerHeight = 0;
        }
        mDivider = divider;
        mList->invalidateItemDecorations();
    }

    void setDividerHeight(int dividerHeight) {
        mDividerHeight = dividerHeight;
        mList->invalidateItemDecorations();
    }

    void setAllowDividerAfterLastItem(bool allowDividerAfterLastItem) {
        mAllowDividerAfterLastItem = allowDividerAfterLastItem;
    }
};

/**
 * Port of PreferenceFragment.ScrollToPreferenceObserver — retries the scroll
 * when the adapter updates.
 */
class PreferenceFragment::ScrollToPreferenceObserver : public RecyclerView::AdapterDataObserver {
public:
    RecyclerView::Adapter* mAdapter;
    RecyclerView* mList;
    Preference* mPreference;
    std::string mKey;

    ScrollToPreferenceObserver(RecyclerView::Adapter* adapter, RecyclerView* list,
            Preference* preference, const std::string& key)
        : mAdapter(adapter), mList(list), mPreference(preference), mKey(key) {
    }

    void scrollToPreference() {
        mAdapter->unregisterAdapterDataObserver(this);
        int position;
        if (mPreference != nullptr) {
            auto* callback = dynamic_cast<PreferenceGroup::PreferencePositionCallback*>(mAdapter);
            position = callback ? callback->getPreferenceAdapterPosition(mPreference)
                                : RecyclerView::NO_POSITION;
        } else {
            auto* callback = dynamic_cast<PreferenceGroup::PreferencePositionCallback*>(mAdapter);
            position = callback ? callback->getPreferenceAdapterPosition(mKey)
                                : RecyclerView::NO_POSITION;
        }
        if (position != RecyclerView::NO_POSITION) {
            mList->scrollToPosition(position);
        }
    }

    void onChanged() override { scrollToPreference(); }
    void onItemRangeChanged(int positionStart, int itemCount) override { scrollToPreference(); }
    void onItemRangeChanged(int positionStart, int itemCount, Object* payload) override {
        scrollToPreference();
    }
    void onItemRangeInserted(int positionStart, int itemCount) override { scrollToPreference(); }
    void onItemRangeRemoved(int positionStart, int itemCount) override { scrollToPreference(); }
    void onItemRangeMoved(int fromPosition, int toPosition, int itemCount) override {
        scrollToPreference();
    }
};

PreferenceFragment::~PreferenceFragment() {
    // mDividerDecoration is owned by the list RecyclerView (RV owns its
    // decorations by design); it dies with the view tree.
    delete mBoundAdapter;   // belt: onDestroyView normally unbinds first
    delete mPreferenceManager;
}

void PreferenceFragment::onCreate(Bundle* savedInstanceState) {
    Fragment::onCreate(savedInstanceState);

    mHandler = std::make_unique<Handler>(Looper::getMainLooper());
    mRequestFocus = [this]() {
        mList->focusableViewAvailable(mList);
    };

    // AOSP resolves ?attr/preferenceTheme and applies the overlay style; the
    // CDROID framework theme already carries the preference styles, so the
    // overlay step is skipped.

    mPreferenceManager = new PreferenceManager(*requireContext());
    mPreferenceManager->setOnNavigateToScreenListener(
        [this](PreferenceScreen& preferenceScreen) {
            onNavigateToScreen(preferenceScreen);
        });
    std::string rootKey;
    const Bundle* args = getArguments();
    if (args != nullptr) {
        rootKey = args->getString(ARG_PREFERENCE_ROOT);
    }
    onCreatePreferences(savedInstanceState, rootKey);
}

View* PreferenceFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
        Bundle* savedInstanceState) {
    namespace ns = internal::R::styleable;

    Context& context = *requireContext();
    int layoutResId = (int)internal::R::layout::preference_list_fragment;
    Drawable* divider = nullptr;
    int dividerHeight = -1;
    bool allowDividerAfterLastItem = true;

    auto a = context.obtainStyledAttributes(nullptr, ns::PreferenceFragment,
            (int)internal::R::attr::preferenceFragmentStyle, 0);
    if (a) {
        layoutResId = (int)a->getResourceId(ns::PreferenceFragment_layout, (uint32_t)layoutResId);
        divider = a->getDrawable(ns::PreferenceFragment_divider);
        dividerHeight = a->getDimensionPixelSize(ns::PreferenceFragment_dividerHeight, -1);
        allowDividerAfterLastItem = a->getBoolean(ns::PreferenceFragment_allowDividerAfterLastItem, true);
    }

    View* view = inflater->inflate(layoutResId, container, false);

    View* rawListContainer = view->findViewById(AndroidResources::ANDROID_R_LIST_CONTAINER);
    ViewGroup* listContainer = dynamic_cast<ViewGroup*>(rawListContainer);
    if (listContainer == nullptr) {
        throw std::logic_error("Content has view with id attribute "
                "'android.R.id.list_container' that is not a ViewGroup class");
    }

    RecyclerView* listView = onCreateRecyclerView(inflater, listContainer, savedInstanceState);
    if (listView == nullptr) {
        throw std::runtime_error("Could not create RecyclerView");
    }

    mList = listView;

    mDividerDecoration = new DividerDecoration();
    mDividerDecoration->mList = mList;
    mList->addItemDecoration(mDividerDecoration);
    mDividerDecoration->setDivider(divider);
    if (dividerHeight != -1) {
        mDividerDecoration->setDividerHeight(dividerHeight);
    }
    mDividerDecoration->setAllowDividerAfterLastItem(allowDividerAfterLastItem);

    // If mList isn't present in the view hierarchy, add it.
    if (mList->getParent() == nullptr) {
        listContainer->addView(mList);
    }
    mHandler->post(mRequestFocus);

    return view;
}

void PreferenceFragment::setDivider(Drawable* divider) {
    // The decoration is owned by the list RecyclerView; it only exists
    // between onCreateView and onDestroyView (mList delimits that window).
    if (mList == nullptr || mDividerDecoration == nullptr) return;
    mDividerDecoration->setDivider(divider);
}

void PreferenceFragment::setDividerHeight(int height) {
    if (mList == nullptr || mDividerDecoration == nullptr) return;
    mDividerDecoration->setDividerHeight(height);
}

void PreferenceFragment::onViewCreated(View* view, Bundle* savedInstanceState) {
    Fragment::onViewCreated(view, savedInstanceState);

    if (savedInstanceState != nullptr) {
        Bundle* container = savedInstanceState->getBundle(PREFERENCES_TAG);
        if (container != nullptr) {
            PreferenceScreen* preferenceScreen = getPreferenceScreen();
            if (preferenceScreen != nullptr) {
                preferenceScreen->restoreHierarchyState(*container);
            }
        }
    }

    if (mHavePrefs) {
        bindPreferences();
        if (mSelectPreferenceRunnable) {
            mSelectPreferenceRunnable();
            mSelectPreferenceRunnable = nullptr;
        }
    }

    mInitDone = true;
}

void PreferenceFragment::onStart() {
    Fragment::onStart();
    mPreferenceManager->setOnPreferenceTreeClickListener(
        [this](Preference& preference) { return onPreferenceTreeClick(preference); });
    mPreferenceManager->setOnDisplayPreferenceDialogListener(
        [this](Preference& preference) { onDisplayPreferenceDialog(preference); });
}

void PreferenceFragment::onStop() {
    Fragment::onStop();
    mPreferenceManager->setOnPreferenceTreeClickListener(nullptr);
    mPreferenceManager->setOnDisplayPreferenceDialogListener(nullptr);
}

void PreferenceFragment::onDestroyView() {
    mHandler->removeCallbacks(mRequestFocus);
    mHandler->removeMessages(MSG_BIND_PREFERENCES);
    if (mHavePrefs) {
        unbindPreferences();
    }
    mList = nullptr;
    Fragment::onDestroyView();
}

void PreferenceFragment::onSaveInstanceState(Bundle* outState) {
    Fragment::onSaveInstanceState(outState);

    PreferenceScreen* preferenceScreen = getPreferenceScreen();
    if (preferenceScreen != nullptr) {
        Bundle container;
        preferenceScreen->saveHierarchyState(container);
        outState->putBundle(PREFERENCES_TAG, &container);
    }
}

PreferenceManager* PreferenceFragment::getPreferenceManager() const {
    return mPreferenceManager;
}

PreferenceScreen* PreferenceFragment::getPreferenceScreen() const {
    if (mPreferenceManager == nullptr) {
        return nullptr;
    }
    return mPreferenceManager->getPreferenceScreen();
}

void PreferenceFragment::setPreferenceScreen(PreferenceScreen* preferenceScreen) {
    if (preferenceScreen != nullptr && mPreferenceManager->setPreferences(preferenceScreen)) {
        onUnbindPreferences();
        mHavePrefs = true;
        if (mInitDone) {
            postBindPreferences();
        }
    }
}

void PreferenceFragment::addPreferencesFromResource(int preferencesResId) {
    requirePreferenceManager();

    setPreferenceScreen(mPreferenceManager->inflateFromResource(*requireContext(),
            preferencesResId, getPreferenceScreen()));
}

void PreferenceFragment::setPreferencesFromResource(int preferencesResId,
        const std::string& key) {
    requirePreferenceManager();

    PreferenceScreen* xmlRoot = mPreferenceManager->inflateFromResource(*requireContext(),
            preferencesResId, nullptr);

    Preference* root;
    if (!key.empty()) {
        root = xmlRoot->findPreference(key);
        if (dynamic_cast<PreferenceScreen*>(root) == nullptr) {
            throw std::invalid_argument("Preference object with key " + key
                    + " is not a PreferenceScreen");
        }
    } else {
        root = xmlRoot;
    }

    setPreferenceScreen(static_cast<PreferenceScreen*>(root));
}

bool PreferenceFragment::onPreferenceTreeClick(Preference& preference) {
    // AOSP walks the callback-fragment / parent-fragment chain and the
    // activity for OnPreferenceStartFragmentCallback, then falls back to
    // FragmentFactory instantiation. CDROID has no reflective fragment
    // factory, so a fragment-typed preference without a handler up the chain
    // just logs.
    if (!preference.getFragment().empty()) {
        bool handled = false;
        Fragment* callbackFragment = this;
        while (!handled && callbackFragment != nullptr) {
            auto* cb = dynamic_cast<OnPreferenceStartFragmentCallback*>(callbackFragment);
            if (cb != nullptr) {
                handled = cb->onPreferenceStartFragment(*this, preference);
            }
            callbackFragment = callbackFragment->getParentFragment();
        }
        if (!handled) {
            LOGW("onPreferenceStartFragment is not implemented in the parent - cannot"
                 " instantiate fragment \"%s\" reflectively in CDROID",
                 preference.getFragment().c_str());
        }
        return true;
    }
    return false;
}

void PreferenceFragment::onNavigateToScreen(PreferenceScreen& preferenceScreen) {
    bool handled = false;
    Fragment* callbackFragment = this;
    while (!handled && callbackFragment != nullptr) {
        auto* cb = dynamic_cast<OnPreferenceStartScreenCallback*>(callbackFragment);
        if (cb != nullptr) {
            handled = cb->onPreferenceStartScreen(*this, preferenceScreen);
        }
        callbackFragment = callbackFragment->getParentFragment();
    }
}

Preference* PreferenceFragment::findPreference(const std::string& key) const {
    if (mPreferenceManager == nullptr) {
        return nullptr;
    }
    return mPreferenceManager->findPreference(key);
}

void PreferenceFragment::requirePreferenceManager() {
    if (mPreferenceManager == nullptr) {
        throw std::runtime_error("This should be called after super.onCreate.");
    }
}

void PreferenceFragment::postBindPreferences() {
    if (mHandler->hasMessages(MSG_BIND_PREFERENCES)) return;
    mHandler->sendEmptyMessage(MSG_BIND_PREFERENCES);
}

void PreferenceFragment::bindPreferences() {
    PreferenceScreen* preferenceScreen = getPreferenceScreen();
    if (preferenceScreen != nullptr) {
        // RecyclerView does not own adapters (AOSP relies on GC); the fragment
        // owns whatever it installs — free the previous binding first
        // (bindPreferences re-runs on preferences reloads).
        delete mBoundAdapter;
        mBoundAdapter = onCreateAdapter(preferenceScreen);
        getListView()->setAdapter(mBoundAdapter);
        preferenceScreen->onAttached();
    }
    onBindPreferences();
}

void PreferenceFragment::unbindPreferences() {
    getListView()->setAdapter(nullptr);
    delete mBoundAdapter;
    mBoundAdapter = nullptr;
    PreferenceScreen* preferenceScreen = getPreferenceScreen();
    if (preferenceScreen != nullptr) {
        preferenceScreen->onDetached();
    }
    onUnbindPreferences();
}

void PreferenceFragment::onBindPreferences() {
}

void PreferenceFragment::onUnbindPreferences() {
}

RecyclerView* PreferenceFragment::getListView() const {
    return mList;
}

RecyclerView* PreferenceFragment::onCreateRecyclerView(LayoutInflater* inflater,
        ViewGroup* parent, Bundle* /*savedInstanceState*/) {
    RecyclerView* recyclerView = static_cast<RecyclerView*>(
            inflater->inflate((int)internal::R::layout::preference_recyclerview, parent, false));

    recyclerView->setLayoutManager(std::unique_ptr<RecyclerView::LayoutManager>(
            onCreateLayoutManager()));

    return recyclerView;
}

RecyclerView::LayoutManager* PreferenceFragment::onCreateLayoutManager() {
    return new LinearLayoutManager(requireContext());
}

RecyclerView::Adapter* PreferenceFragment::onCreateAdapter(PreferenceScreen* preferenceScreen) {
    return new PreferenceGroupAdapter(*preferenceScreen);
}

void PreferenceFragment::onDisplayPreferenceDialog(Preference& preference) {
    bool handled = false;
    Fragment* callbackFragment = this;
    while (!handled && callbackFragment != nullptr) {
        auto* cb = dynamic_cast<OnPreferenceDisplayDialogCallback*>(callbackFragment);
        if (cb != nullptr) {
            handled = cb->onPreferenceDisplayDialog(*this, preference);
        }
        callbackFragment = callbackFragment->getParentFragment();
    }

    if (handled) {
        return;
    }

    // check if dialog is already showing
    if (getParentFragmentManager()->findFragmentByTag(DIALOG_FRAGMENT_TAG) != nullptr) {
        return;
    }

    DialogFragment* f = nullptr;
    if (dynamic_cast<EditTextPreference*>(&preference) != nullptr) {
        f = EditTextPreferenceDialogFragment::newInstance(preference.getKey());
    } else if (dynamic_cast<ListPreference*>(&preference) != nullptr) {
        f = ListPreferenceDialogFragment::newInstance(preference.getKey());
    } else if (dynamic_cast<MultiSelectListPreference*>(&preference) != nullptr) {
        f = MultiSelectListPreferenceDialogFragment::newInstance(preference.getKey());
    } else {
        throw std::invalid_argument(
                std::string("Cannot display dialog for an unknown Preference type: ")
                + preference.getPreferenceClassName()
                + ". Make sure to implement onPreferenceDisplayDialog() to handle "
                "displaying a custom dialog for this Preference.");
    }
    f->setTargetFragment(this, 0);
    f->show(getParentFragmentManager(), DIALOG_FRAGMENT_TAG);
}

Fragment* PreferenceFragment::getCallbackFragment() {
    return nullptr;
}

void PreferenceFragment::scrollToPreference(const std::string& key) {
    scrollToPreferenceInternal(nullptr, key);
}

void PreferenceFragment::scrollToPreference(Preference* preference) {
    scrollToPreferenceInternal(preference, std::string());
}

void PreferenceFragment::scrollToPreferenceInternal(Preference* preference,
        const std::string& key) {
    Runnable r = [this, preference, key]() {
        RecyclerView::Adapter* adapter = mList->getAdapter();
        auto* positionCallback = dynamic_cast<PreferenceGroup::PreferencePositionCallback*>(adapter);
        if (positionCallback == nullptr) {
            if (adapter != nullptr) {
                throw std::logic_error("Adapter must implement PreferencePositionCallback");
            } else {
                // Adapter was set to null, so don't scroll
                return;
            }
        }
        int position;
        if (preference != nullptr) {
            position = positionCallback->getPreferenceAdapterPosition(preference);
        } else {
            position = positionCallback->getPreferenceAdapterPosition(key);
        }
        if (position != RecyclerView::NO_POSITION) {
            mList->scrollToPosition(position);
        } else {
            // Item not found, wait for an update and try again
            adapter->registerAdapterDataObserver(
                    new ScrollToPreferenceObserver(adapter, mList, preference, key));
        }
    };
    if (mList == nullptr) {
        mSelectPreferenceRunnable = r;
    } else {
        r();
    }
}

} // namespace cdroid
