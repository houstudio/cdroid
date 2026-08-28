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
#ifndef __CDROID_DROPDOWN_PREFERENCE_H__
#define __CDROID_DROPDOWN_PREFERENCE_H__

#include <preference/listpreference.h>
#include <widget/adapterview.h>
#include <widget/adapter.h>

namespace cdroid {

class Spinner;
class AdapterView;

/**
 * Port of androidx.preference.DropDownPreference — a ListPreference whose
 * choices surface through a Spinner in the row's widget frame instead of a
 * dialog.
 */
class DropDownPreference : public ListPreference {
public:
    DropDownPreference(Context& context);
    DropDownPreference(Context& context, const AttributeSet& attrs);
    DropDownPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    DropDownPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);
    // CDROID ownership: we new the entries adapter in the ctor (AOSP: GC) —
    // Spinner/DialogPopup take their own wrap, this one stays ours.
    ~DropDownPreference() override;

    void onClick() override;

    void setEntries(const std::vector<std::string>& entries) override;

    void setValueIndex(int index) override;

    void onBindViewHolder(PreferenceViewHolder& holder) override;

    std::string getPreferenceClassName() const override { return "DropDownPreference"; }

protected:
    /** AOSP's createAdapter seam; the default is a plain string ArrayAdapter. */
    virtual Adapter* createAdapter();

    void notifyChanged() override;

private:
    void updateEntries();
    int findSpinnerIndexOfValue(const std::string& value) const;

    Context& mContext;
    Adapter* mAdapter = nullptr;
    Spinner* mSpinner = nullptr;
    AdapterView::OnItemSelectedListener mItemSelectedListener;
};

} // namespace cdroid

#endif // __CDROID_DROPDOWN_PREFERENCE_H__
