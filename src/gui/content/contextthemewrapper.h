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
#ifndef __CONTEXT_THEME_WRAPPER_H__
#define __CONTEXT_THEME_WRAPPER_H__
#include <content/contextwrapper.h>
namespace cdroid{

/** A context wrapper that allows you to modify or replace the theme of the
 *  wrapped context. (AOSP android.view.ContextThemeWrapper, ported verbatim;
 *  the Configuration-override and LayoutInflater getSystemService branches are
 *  deferred — those subsystems are not ported.) */
class ContextThemeWrapper:public ContextWrapper{
private:
    int mThemeResource = 0;
    std::unique_ptr<Resources::Theme> mTheme;
protected:
    void attachBaseContext(Context* newBase)override;
    /** AOSP onApplyThemeResource: apply a theme resource to the current Theme.
     *  May be overridden to change the default (simple) behavior. */
    virtual void onApplyThemeResource(Resources::Theme& theme,int resId,bool first);
    void initializeTheme();
public:
    /** Creates a new context wrapper with no theme and no base context; a base
     *  context must be attached via attachBaseContext() before any other call. */
    ContextThemeWrapper();
    /** The theme will be applied on top of the base context's theme; attributes
     *  not defined there keep their original values. */
    ContextThemeWrapper(Context* base,int themeResId);
    /** The theme completely replaces the base context's theme. */
    ContextThemeWrapper(Context* base,const Resources::Theme& theme);

    void setTheme(int resid)override;
    /** Set the current Theme directly (null resets to the default). */
    void setTheme(const Resources::Theme& theme);
    int getThemeResId()const;
    Resources::Theme getTheme()override;
};

}//endof namespace
#endif
