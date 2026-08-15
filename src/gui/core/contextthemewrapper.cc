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
#include <core/contextthemewrapper.h>

namespace cdroid{

ContextThemeWrapper::ContextThemeWrapper() {
}

ContextThemeWrapper::ContextThemeWrapper(Context* base,int themeResId) {
    attachBaseContext(base);
    mThemeResource = themeResId;
}

ContextThemeWrapper::ContextThemeWrapper(Context* base,const Resources::Theme& theme) {
    attachBaseContext(base);
    mTheme = std::make_unique<Resources::Theme>(theme);
}

void ContextThemeWrapper::attachBaseContext(Context* newBase) {
    ContextWrapper::attachBaseContext(newBase);
}

void ContextThemeWrapper::setTheme(int resid) {
    if (mThemeResource != resid) {
        mThemeResource = resid;
        initializeTheme();
    }
}

void ContextThemeWrapper::setTheme(const Resources::Theme& theme) {
    mTheme = std::make_unique<Resources::Theme>(theme);
}

int ContextThemeWrapper::getThemeResId()const {
    return mThemeResource;
}

Resources::Theme ContextThemeWrapper::getTheme() {
    if (mTheme != nullptr) {
        return *mTheme;
    }

    // AOSP: mThemeResource = Resources.selectDefaultTheme(mThemeResource,
    // getApplicationInfo().targetSdkVersion) — CDROID applies the framework
    // default theme at App bootstrap, so the resource id is used as-is.
    initializeTheme();

    return *mTheme;
}

void ContextThemeWrapper::onApplyThemeResource(Resources::Theme& theme,int resId,bool /*first*/) {
    theme.applyStyle(resId, true);
}

void ContextThemeWrapper::initializeTheme() {
    const bool first = (mTheme == nullptr);
    if (first) {
        mTheme = std::make_unique<Resources::Theme>(getResources().newTheme());
        const Resources::Theme theme = getBaseContext()->getTheme();
        // AOSP checks theme != null; CDROID getTheme() always yields a usable
        // engine (Assets lazily builds one), so setTo unconditionally.
        mTheme->setTo(theme);
    }
    onApplyThemeResource(*mTheme, mThemeResource, first);
}

}//endof namespace
