/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <fstream>
#include <core/context.h>
#include <core/iostreams.h>   // AssetInputStream
#include <gesture/gesturelibrary.h>
#include <gesture/gesturelibraries.h>
#include <porting/cdlog.h>

namespace cdroid{

// AOSP GestureLibraries has one private library per door: FileGestureLibrary
// owns a filesystem path; ResourceGestureLibrary holds the @RawRes int and
// loads through Resources.openRawResource — the id resolves to the asset path
// through the resource table before anything is opened, never by name.

class GestureLibraries::FileGestureLibrary:public GestureLibrary {
private:
    std::string mPath;
public:
    explicit FileGestureLibrary(const std::string& path):mPath(path){
    }

    bool isReadOnly() const override{
        return false;   // AOSP asks the file; decided at save() time here
    }

    bool save() override{
        if (!mStore->hasChanged()) return true;
        std::ofstream ofs(mPath);
        mStore->save(ofs, true);
        const bool result = ofs.good();
        if(!result) LOGD("Could not save the gesture library in %s",mPath.c_str());
        return result;
    }

    bool load() override{
        std::ifstream ifs(mPath);
        if(!ifs) return false;
        // GestureStore::load is void (AOSP throws IOException instead)
        mStore->load(ifs, true);
        return true;
    }
};

class GestureLibraries::ResourceGestureLibrary:public GestureLibrary {
private:
    Context* mContext;
    int mResourceId;
public:
    ResourceGestureLibrary(Context*ctx,int resourceId):mContext(ctx),mResourceId(resourceId){
    }

    bool isReadOnly() const override{
        return true;
    }

    bool save() override{
        return false;
    }

    bool load() override{
        if(Asset*asset = mContext ? mContext->openRawResource(mResourceId) : nullptr){
            AssetInputStream fs(asset);   // owns and deletes the Asset
            mStore->load(fs);
            return true;
        }
        LOGD("Could not load the gesture library from raw resource #%d",mResourceId);
        return false;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

GestureLibraries::GestureLibraries() {
}

GestureLibrary* GestureLibraries::fromFile(const std::string& path) {
    return new FileGestureLibrary(path);
}

GestureLibrary* GestureLibraries::fromRawResource(Context* context, int resourceId) {
    return new ResourceGestureLibrary(context, resourceId);
}

}/*endof namespace*/
