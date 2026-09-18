#include "bluetoothprogresscategory.h"

#include <widget/progressbar.h>
#include <R.h>

namespace preferencedemo {

BluetoothProgressCategory::BluetoothProgressCategory(Context* context)
    : PreferenceCategory(*context) {
    // AOSP ProgressCategory: R.layout.preference_progress_category.
    setLayoutResource((int)preferencedemo::R::layout::preference_progress_category);
}

void BluetoothProgressCategory::onBindViewHolder(
        cdroid::PreferenceViewHolder& holder) {
    PreferenceCategory::onBindViewHolder(holder);
    // AOSP: spinner visible while mProgress, gone otherwise.
    if (View* progress = holder.itemView->findViewById(
                (int)preferencedemo::R::id::scanning_progress)) {
        progress->setVisibility(mProgress ? View::VISIBLE : View::GONE);
    }
}

void BluetoothProgressCategory::setProgress(bool progress) {
    if (mProgress == progress) return;
    mProgress = progress;

    // AOSP ProgressCategory.onBindViewHolder: with no progress and no
    // devices, the centered empty-text row takes the list's place.
    const bool noDeviceFound = getPreferenceCount() == 0
            || (getPreferenceCount() == 1 && mNoDeviceFoundPreference != nullptr);
    if (mProgress || !noDeviceFound) {
        if (mNoDeviceFoundPreference != nullptr) {
            removePreference(mNoDeviceFoundPreference);
            delete mNoDeviceFoundPreference;
            mNoDeviceFoundPreference = nullptr;
        }
    } else if (mNoDeviceFoundPreference == nullptr && !mEmptyText.empty()) {
        mNoDeviceFoundPreference = new Preference(getContext());
        mNoDeviceFoundPreference->setLayoutResource(
                (int)preferencedemo::R::layout::preference_empty_list);
        mNoDeviceFoundPreference->setTitle(mEmptyText);
        mNoDeviceFoundPreference->setSelectable(false);
        addPreference(mNoDeviceFoundPreference);
    }
    notifyChanged();
}

} // namespace preferencedemo
