/*********************************************************************************
 * Port of AOSP 12 Settings:
 *   src/com/android/settings/ProgressCategory.java (the spinner row layout,
 *   R.layout.preference_progress_category) +
 *   src/com/android/settings/bluetooth/BluetoothProgressCategory.java
 *   (empty text = "未在附近找到蓝牙设备。").
 *********************************************************************************/
#ifndef PREFERENCEDRMO_BLUETOOTH_PROGRESS_CATEGORY_H
#define PREFERENCEDRMO_BLUETOOTH_PROGRESS_CATEGORY_H

#include <preference/preferencecategory.h>
#include <preference/preference.h>
#include <preference/preferenceviewholder.h>

namespace preferencedemo {

using cdroid::Preference;
using cdroid::PreferenceCategory;
using cdroid::Context;

class BluetoothProgressCategory : public PreferenceCategory {
public:
    explicit BluetoothProgressCategory(Context* context);

    void setProgress(bool progress);
    bool getProgress() const { return mProgress; }

    void setEmptyTextRes(const std::string& emptyText) { mEmptyText = emptyText; }

    void onBindViewHolder(cdroid::PreferenceViewHolder& holder) override;

private:
    bool mProgress = false;
    std::string mEmptyText;
    /* AOSP mNoDeviceFoundPreference: centered empty-list row, shown when a
     * scan ends with no devices. */
    Preference* mNoDeviceFoundPreference = nullptr;
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_BLUETOOTH_PROGRESS_CATEGORY_H
