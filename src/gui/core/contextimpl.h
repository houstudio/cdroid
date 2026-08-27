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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CONTEXTIMPL_H__
#define __CONTEXTIMPL_H__
#include <core/context.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace cdroid{

class ZIPArchive;   // private/ziparchive.h
class ResTable;     // androidfw/restable.h

// CDROID's android.app.ContextImpl: the Context implementation layer that owns
// the resource stack — the pak registry, the loaded arsc table and its theme
// engine, plus the string-key stream/image access built on them. App (the
// Application role) derives from this instead of Context directly; AOSP's
// Application is a stateless ContextWrapper around a ContextImpl.
class ContextImpl:public Context{
protected:
    std::unordered_map<std::string, ZIPArchive*> mResources;
    ResTable* mResTable = nullptr;   // resources.arsc from the paks (null if none)
    // arsc theme engine (ResTable::Theme*), kept opaque so this header needs
    // no androidfw type; the .cc casts.
    void* mArscTheme = nullptr;
    // "@[package:][+]id/filename" → package + relative name.
    const std::string parseResource(const std::string&fullResId,std::string*res,std::string*ns)const;
    // Resolve a string resource ref to its owning pak (guessExtension appends
    // .png/.xml/... when the name carries no extension).
    ZIPArchive*getResource(const std::string&fullResId,std::string*relativeResID,std::string*outPackage)const;
    // Open an arsc-recorded file path (e.g. "res/drawable-hdpi-v4/x.png")
    // against the pak layout via pakPathCandidates() (androidfw).
    ZIPArchive*findPakForPath(const std::string&package,const std::string&arscPath,std::string*outResname)const;
    // arsc identifier lookup: requested package, then "android", then any.
    uint32_t arscGetIdentifier(const std::string&name,const std::string&type,const std::string&pkg)const;
public:
    std::unique_ptr<std::istream>getInputStream(const std::string&resname)override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname,int width,int height)override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(int id,int width,int height)override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream&,int width,int height)override;
};

}
#endif
