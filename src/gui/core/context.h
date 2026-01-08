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

namespace cdroid{

class Drawable;
class ColorStateList;
class Context{
public:
    virtual ~Context() = default;
    virtual const std::string getPackageName() const = 0;
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
    virtual float getFloat(const std::string&resid) const = 0;
    virtual size_t getArray(const std::string&resname,std::vector<std::string>&) = 0;
    virtual size_t getArray(const std::string&resname,std::vector<int>&) = 0;
    virtual ColorStateList* getColorStateList(const std::string&resid) = 0;
    virtual AttributeSet obtainStyledAttributes(const std::string&resid) = 0;
};

}
#endif
