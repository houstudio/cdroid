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
#ifndef __CDROID_SEEKBAR_PREFERENCE_H__
#define __CDROID_SEEKBAR_PREFERENCE_H__

#include <preference/preference.h>
#include <widget/seekbar.h>
#include <widget/textview.h>
#include <view/view.h>

namespace cdroid {

class SeekBar;
class TextView;

/**
 * Port of androidx.preference.SeekBarPreference — title plus a SeekBar and an
 * optional SeekBar value TextView. adjustable controls whether the bar
 * responds to DPAD left/right; updatesContinuously saves while dragging.
 */
class SeekBarPreference : public Preference {
public:
    SeekBarPreference(Context& context);
    SeekBarPreference(Context& context, const AttributeSet& attrs);
    SeekBarPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    SeekBarPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    void onBindViewHolder(PreferenceViewHolder& holder) override;

    void onSetInitialValue(const any& defaultValue) override;
    any onGetDefaultValue(const TypedArray& a, int index) override;

    int getMin() const;
    void setMin(int min);

    int getSeekBarIncrement() const;
    void setSeekBarIncrement(int seekBarIncrement);

    int getMax() const;
    void setMax(int max);

    bool isAdjustable() const;
    void setAdjustable(bool adjustable);

    bool getUpdatesContinuously() const;
    void setUpdatesContinuously(bool updatesContinuously);

    bool getShowSeekBarValue() const;
    void setShowSeekBarValue(bool showSeekBarValue);

    int getValue() const;
    void setValue(int seekBarValue);

    std::string getPreferenceClassName() const override { return "SeekBarPreference"; }

    class SavedState : public Preference::BaseSavedState {
    public:
        int mSeekBarValue;
        int mMin;
        int mMax;
        SavedState(Parcel& source);
        SavedState(Parcelable* superState);
        void writeToParcel(Parcel& dest, int flags) override;
    };

protected:
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable* state) override;

private:
    void setValueInternal(int seekBarValue, bool notifyChanged);
    void syncValueInternal(SeekBar& seekBar);
    void updateLabelValue(int value);

    int mSeekBarValue = 0;
    int mMin = 0;
    int mMax = 100;
    int mSeekBarIncrement = 0;
    bool mTrackingTouch = false;
    SeekBar* mSeekBar = nullptr;
    TextView* mSeekBarValueTextView = nullptr;
    // Whether the SeekBar should respond to the left/right keys
    bool mAdjustable = true;
    // Whether to show the SeekBar value TextView next to the bar
    bool mShowSeekBarValue = false;
    // Whether to continuously save the value while the bar is being dragged
    bool mUpdatesContinuously = false;

    SeekBar::OnSeekBarChangeListener mSeekBarChangeListener;
    View::OnKeyListener mSeekBarKeyListener;
};

} // namespace cdroid

#endif // __CDROID_SEEKBAR_PREFERENCE_H__
