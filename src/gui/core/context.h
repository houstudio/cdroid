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
#ifndef __CONTEXT_H__
#define __CONTEXT_H__
#include <string>
#include <iostream>
#include <functional>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <core/callbackbase.h>
#include <core/attributeset.h>
#include <core/displaymetrics.h>

#define USE(FEATURE) (defined(USE_##FEATURE) && USE_##FEATURE)
#define ENABLE(FEATURE) (defined(ENABLE_##FEATURE) && ENABLE_##FEATURE)

// AOSP-aligned resource types (defined in the androidfw sub-library, now compiled
// into cdroid.so). Forward-declared at GLOBAL scope so cdroid::Context can expose
// the ID-based AOSP Context resource face; their full headers are included only
// where needed (core/context.cc, assets.cc). Must NOT be nested in cdroid (would
// create cdroid::android and shadow the real ::android used elsewhere, e.g.
// android::localeDataComputeScript).
namespace android { class Resources; class AssetManager; class Asset; }

namespace cdroid{
class Drawable;
class ColorStateList;
class Typeface;
class Intent;
class Context{
public:
    virtual ~Context() = default;
    virtual const std::string getPackageName() const = 0;
    // CDROID seam: androidx ActivityNavigator ends in context.startActivity(intent). CDROID has no
    // framework "start Activity by Intent" (Activity == Window, instantiated by `new`, not by name),
    // so the default is a no-op. Wiring (className -> Window factory + show) is deferred; override
    // (e.g. on App) to actually launch. Kept as Context* so Navigator's mContext->startActivity compiles.
    virtual void startActivity(const Intent& /*intent*/) = 0;
    virtual const std::string getTheme() const = 0;
    virtual void setTheme(const std::string&theme) = 0;
    virtual const DisplayMetrics&getDisplayMetrics() const = 0;
    virtual int getId(const std::string&) const = 0;
    virtual int getNextAutofillId() = 0;
    virtual const std::string getString(const std::string&id,const std::string&lan="") = 0;
    virtual std::unique_ptr<std::istream>getInputStream(const std::string&,std::string*outpkg=nullptr) = 0;

    virtual Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname,int width,int height) = 0;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname) {
        return loadImage(resname,-1,-1);
    }
    virtual Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream&,int width,int height) = 0;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream&stream) {
        return loadImage(stream,-1,-1);
    }

    virtual Drawable* getDrawable(const std::string&resid) = 0;
    virtual int getColor(const std::string&resid) = 0;
    virtual bool getBoolean(const std::string&resid) const = 0;
    virtual int getDimension(const std::string&resid) const = 0;
    virtual int getDimensionPixelSize(const std::string&key,int def=0) const = 0;
    virtual float getFloat(const std::string&resid,float def=0) const = 0;
    virtual size_t getArray(const std::string&resname,std::vector<std::string>&) = 0;
    virtual size_t getArray(const std::string&resname,std::vector<int>&) = 0;
    virtual RefPtr<ColorStateList> getColorStateList(const std::string&resid) = 0;
    virtual AttributeSet obtainStyledAttributes(const std::string&resid) = 0;

    // --- AOSP-aligned ID-based resource face (android.content.Context) ---
    // Coexists with the string-based legacy methods above (overloads differ by
    // int vs std::string). Default implementations live in core/context.cc and
    // delegate to getResources(); pure-virtual ones (getResources/getAssets/
    // getDrawable(int)/getColorStateList(int)) are implemented by Assets.
    virtual android::Resources&      getResources() = 0;
    virtual android::AssetManager&   getAssets() = 0;
    virtual std::string    getString(int id);
    virtual std::u16string getText(int id);
    virtual std::string    getQuantityString(int id, int quantity);
    virtual int            getColor(int id);
    virtual bool           getBoolean(int id);
    virtual int            getInteger(int id);
    virtual float          getDimension(int id);
    virtual int            getDimensionPixelSize(int id);
    virtual android::Asset* openRawResource(int id);
    virtual Drawable*       getDrawable(int id) = 0;
    virtual ColorStateList* getColorStateList(int id) = 0;
    virtual Typeface*       getFont(int id);   // default nullptr (deferred)
};

}
#endif
