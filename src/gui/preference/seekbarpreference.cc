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
#include <preference/seekbarpreference.h>
#include <preference/preferenceviewholder.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <preference/preferenceinflater.h>
#include <widget/seekbar.h>
#include <widget/textview.h>
#include <view/view.h>
#include <view/keyevent.h>
#include <content/typedarray.h>
#include <core/parcel.h>
#include <algorithm>
#include <porting/cdlog.h>

namespace cdroid {

SeekBarPreference::SeekBarPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : Preference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    auto a = context.obtainStyledAttributes(attrs, ns::SeekBarPreference, defStyleAttr, defStyleRes);

    // The ordering of these two statements are important. If we want to set max first, we need
    // to perform the same steps by changing min/max to max/min as following:
    // mMax = a.getInt(...) and setMin(...).
    mMin = a->getInt(ns::SeekBarPreference_min, 0);
    setMax(a->getInt(ns::SeekBarPreference_max, 100));
    setSeekBarIncrement(a->getInt(ns::SeekBarPreference_seekBarIncrement, 0));
    mAdjustable = a->getBoolean(ns::SeekBarPreference_adjustable, true);
    mShowSeekBarValue = a->getBoolean(ns::SeekBarPreference_showSeekBarValue, false);
    mUpdatesContinuously = a->getBoolean(ns::SeekBarPreference_updatesContinuously, false);

    // Listener reacting to the SeekBar changing value by the user
    mSeekBarChangeListener.onProgressChanged =
        [this](SeekBar& seekBar, int progress, bool fromUser) {
            if (fromUser && (mUpdatesContinuously || !mTrackingTouch)) {
                syncValueInternal(seekBar);
            } else {
                // We always want to update the text while the seekbar is being dragged
                updateLabelValue(progress + mMin);
            }
        };
    mSeekBarChangeListener.onStartTrackingTouch = [this](SeekBar&) {
        mTrackingTouch = true;
    };
    mSeekBarChangeListener.onStopTrackingTouch = [this](SeekBar& seekBar) {
        mTrackingTouch = false;
        if (seekBar.getProgress() + mMin != mSeekBarValue) {
            syncValueInternal(seekBar);
        }
    };

    // Listener reacting to the user pressing DPAD left/right keys if adjustable
    // is set; transfers the key presses to the SeekBar to be handled accordingly.
    mSeekBarKeyListener = [this](View&, int keyCode, KeyEvent& event) {
        if (event.getAction() != KeyEvent::ACTION_DOWN) {
            return false;
        }

        if (!mAdjustable && (keyCode == KeyEvent::KEYCODE_DPAD_LEFT
                || keyCode == KeyEvent::KEYCODE_DPAD_RIGHT)) {
            // Right or left keys are pressed when in non-adjustable mode; Skip the keys.
            return false;
        }

        // We don't want to propagate the click keys down to the SeekBar view since it will
        // create the ripple effect for the thumb.
        if (keyCode == KeyEvent::KEYCODE_DPAD_CENTER || keyCode == KeyEvent::KEYCODE_ENTER) {
            return false;
        }

        if (mSeekBar == nullptr) {
            LOGE("SeekBar view is null and hence cannot be adjusted.");
            return false;
        }
        // onKeyDown is protected (AOSP calls it package-private); dispatch the
        // event through the public face instead.
        return mSeekBar->dispatchKeyEvent(event);
    };
}

SeekBarPreference::SeekBarPreference(Context& context, const AttributeSet& attrs, int defStyleAttr)
    : SeekBarPreference(context, attrs, defStyleAttr, 0) {
}

SeekBarPreference::SeekBarPreference(Context& context, const AttributeSet& attrs)
    : SeekBarPreference(context, attrs, (int)internal::R::attr::seekBarPreferenceStyle) {
}

SeekBarPreference::SeekBarPreference(Context& context)
    : SeekBarPreference(context, AttributeSet()) {
}

void SeekBarPreference::onBindViewHolder(PreferenceViewHolder& holder) {
    Preference::onBindViewHolder(holder);
    holder.itemView->setOnKeyListener(mSeekBarKeyListener);
    mSeekBar = dynamic_cast<SeekBar*>(holder.findViewById((int)internal::R::id::seekbar));
    // The framework preference_widget_seekbar layout has no seekbar_value
    // TextView (the androidx one does); the lookup stays so a layout that
    // provides one (aapt2 assigns the id from @+android:id/seekbar_value)
    // keeps working — here it resolves to null and updateLabelValue no-ops.
    static constexpr int SEEKBAR_VALUE_ID = 0;
    mSeekBarValueTextView = dynamic_cast<TextView*>(holder.findViewById(SEEKBAR_VALUE_ID));
    if (mShowSeekBarValue) {
        if (mSeekBarValueTextView != nullptr) mSeekBarValueTextView->setVisibility(View::VISIBLE);
    } else {
        if (mSeekBarValueTextView != nullptr) mSeekBarValueTextView->setVisibility(View::GONE);
        mSeekBarValueTextView = nullptr;
    }

    if (mSeekBar == nullptr) {
        LOGE("SeekBar view is null in onBindViewHolder.");
        return;
    }
    mSeekBar->setOnSeekBarChangeListener(mSeekBarChangeListener);
    mSeekBar->setMax(mMax - mMin);
    if (mSeekBarIncrement != 0) {
        mSeekBar->setKeyProgressIncrement(mSeekBarIncrement);
    } else {
        mSeekBarIncrement = mSeekBar->getKeyProgressIncrement();
    }

    mSeekBar->setProgress(mSeekBarValue - mMin);
    updateLabelValue(mSeekBarValue);
    mSeekBar->setEnabled(isEnabled());
}

void SeekBarPreference::onSetInitialValue(const any& defaultValue) {
    int def = 0;
    if (defaultValue.has_value()) {
        def = any_cast<int>(defaultValue);
    }
    setValue(getPersistedInt(def));
}

any SeekBarPreference::onGetDefaultValue(const TypedArray& a, int index) {
    return any(a.getInt(index, 0));
}

int SeekBarPreference::getMin() const {
    return mMin;
}

void SeekBarPreference::setMin(int min) {
    if (min > mMax) {
        min = mMax;
    }
    if (min != mMin) {
        mMin = min;
        notifyChanged();
    }
}

int SeekBarPreference::getSeekBarIncrement() const {
    return mSeekBarIncrement;
}

void SeekBarPreference::setSeekBarIncrement(int seekBarIncrement) {
    if (seekBarIncrement != mSeekBarIncrement) {
        mSeekBarIncrement = std::min(mMax - mMin, std::abs(seekBarIncrement));
        notifyChanged();
    }
}

int SeekBarPreference::getMax() const {
    return mMax;
}

void SeekBarPreference::setMax(int max) {
    if (max < mMin) {
        max = mMin;
    }
    if (max != mMax) {
        mMax = max;
        notifyChanged();
    }
}

bool SeekBarPreference::isAdjustable() const {
    return mAdjustable;
}

void SeekBarPreference::setAdjustable(bool adjustable) {
    mAdjustable = adjustable;
}

bool SeekBarPreference::getUpdatesContinuously() const {
    return mUpdatesContinuously;
}

void SeekBarPreference::setUpdatesContinuously(bool updatesContinuously) {
    mUpdatesContinuously = updatesContinuously;
}

bool SeekBarPreference::getShowSeekBarValue() const {
    return mShowSeekBarValue;
}

void SeekBarPreference::setShowSeekBarValue(bool showSeekBarValue) {
    mShowSeekBarValue = showSeekBarValue;
    notifyChanged();
}

void SeekBarPreference::setValueInternal(int seekBarValue, bool notifyChanged) {
    if (seekBarValue < mMin) {
        seekBarValue = mMin;
    }
    if (seekBarValue > mMax) {
        seekBarValue = mMax;
    }

    if (seekBarValue != mSeekBarValue) {
        mSeekBarValue = seekBarValue;
        updateLabelValue(mSeekBarValue);
        persistInt(seekBarValue);
        if (notifyChanged) {
            this->notifyChanged();
        }
    }
}

int SeekBarPreference::getValue() const {
    return mSeekBarValue;
}

void SeekBarPreference::setValue(int seekBarValue) {
    setValueInternal(seekBarValue, true);
}

void SeekBarPreference::syncValueInternal(SeekBar& seekBar) {
    const int seekBarValue = mMin + seekBar.getProgress();
    if (seekBarValue != mSeekBarValue) {
        if (callChangeListener(seekBarValue)) {
            setValueInternal(seekBarValue, false);
        } else {
            seekBar.setProgress(mSeekBarValue - mMin);
            updateLabelValue(mSeekBarValue);
        }
    }
}

void SeekBarPreference::updateLabelValue(int value) {
    if (mSeekBarValueTextView != nullptr) {
        mSeekBarValueTextView->setText(std::to_string(value));
    }
}

Parcelable* SeekBarPreference::onSaveInstanceState() {
    Parcelable* superState = Preference::onSaveInstanceState();
    if (isPersistent()) {
        // No need to save instance state since it's persistent
        return superState;
    }

    SavedState* myState = new SavedState(superState);
    myState->mSeekBarValue = mSeekBarValue;
    myState->mMin = mMin;
    myState->mMax = mMax;
    return myState;
}

void SeekBarPreference::onRestoreInstanceState(Parcelable* state) {
    if (state == nullptr || dynamic_cast<SavedState*>(state) == nullptr) {
        // Didn't save state for us in saveInstanceState
        Preference::onRestoreInstanceState(state);
        return;
    }

    SavedState* myState = static_cast<SavedState*>(state);
    Preference::onRestoreInstanceState(myState->getSuperState());
    mSeekBarValue = myState->mSeekBarValue;
    mMin = myState->mMin;
    mMax = myState->mMax;
    notifyChanged();
}

SeekBarPreference::SavedState::SavedState(Parcel& source)
    : Preference::BaseSavedState(source) {
    mSeekBarValue = source.readInt();
    mMin = source.readInt();
    mMax = source.readInt();
}

SeekBarPreference::SavedState::SavedState(Parcelable* superState)
    : Preference::BaseSavedState(superState), mSeekBarValue(0), mMin(0), mMax(0) {
}

void SeekBarPreference::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt(mSeekBarValue);
    dest.writeInt(mMin);
    dest.writeInt(mMax);
}

DECLARE_PREFERENCE(SeekBarPreference)

} // namespace cdroid
