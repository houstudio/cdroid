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
#include <preference/dialogpreference.h>
#include <preference/preferencemanager.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <drawable/drawable.h>

namespace cdroid {

DialogPreference::DialogPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : Preference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    auto a = context.obtainStyledAttributes(attrs, ns::DialogPreference, defStyleAttr, defStyleRes);

    mDialogTitle = a->getString(ns::DialogPreference_dialogTitle);
    if (mDialogTitle.empty()) {
        // Fall back on the regular title of the preference
        // (the one that is seen in the list)
        mDialogTitle = getTitle();
    }

    mDialogMessage = a->getString(ns::DialogPreference_dialogMessage);
    mDialogIcon = a->getDrawable(ns::DialogPreference_dialogIcon);
    mPositiveButtonText = a->getString(ns::DialogPreference_positiveButtonText);
    mNegativeButtonText = a->getString(ns::DialogPreference_negativeButtonText);
    mDialogLayoutResId = (int)a->getResourceId(ns::DialogPreference_dialogLayout, 0);
}

DialogPreference::DialogPreference(Context& context, const AttributeSet& attrs, int defStyleAttr)
    : DialogPreference(context, attrs, defStyleAttr, 0) {
}

DialogPreference::DialogPreference(Context& context, const AttributeSet& attrs)
    : DialogPreference(context, attrs, (int)internal::R::attr::dialogPreferenceStyle) {
}

DialogPreference::DialogPreference(Context& context)
    : DialogPreference(context, AttributeSet()) {
}

void DialogPreference::setDialogTitle(const std::string& dialogTitle) {
    mDialogTitle = dialogTitle;
}

void DialogPreference::setDialogTitle(int dialogTitleResId) {
    setDialogTitle(getContext().getString(dialogTitleResId));
}

std::string DialogPreference::getDialogTitle() const {
    return mDialogTitle;
}

void DialogPreference::setDialogMessage(const std::string& dialogMessage) {
    mDialogMessage = dialogMessage;
}

void DialogPreference::setDialogMessage(int dialogMessageResId) {
    setDialogMessage(getContext().getString(dialogMessageResId));
}

std::string DialogPreference::getDialogMessage() const {
    return mDialogMessage;
}

void DialogPreference::setDialogIcon(Drawable* dialogIcon) {
    mDialogIcon = dialogIcon;
}

void DialogPreference::setDialogIcon(int dialogIconRes) {
    mDialogIcon = getContext().getDrawable(dialogIconRes);
}

Drawable* DialogPreference::getDialogIcon() const {
    return mDialogIcon;
}

void DialogPreference::setPositiveButtonText(const std::string& positiveButtonText) {
    mPositiveButtonText = positiveButtonText;
}

void DialogPreference::setPositiveButtonText(int positiveButtonTextResId) {
    setPositiveButtonText(getContext().getString(positiveButtonTextResId));
}

std::string DialogPreference::getPositiveButtonText() const {
    return mPositiveButtonText;
}

void DialogPreference::setNegativeButtonText(const std::string& negativeButtonText) {
    mNegativeButtonText = negativeButtonText;
}

void DialogPreference::setNegativeButtonText(int negativeButtonTextResId) {
    setNegativeButtonText(getContext().getString(negativeButtonTextResId));
}

std::string DialogPreference::getNegativeButtonText() const {
    return mNegativeButtonText;
}

void DialogPreference::setDialogLayoutResource(int dialogLayoutResId) {
    mDialogLayoutResId = dialogLayoutResId;
}

int DialogPreference::getDialogLayoutResource() const {
    return mDialogLayoutResId;
}

void DialogPreference::onClick() {
    getPreferenceManager()->showDialog(*this);
}

} // namespace cdroid
