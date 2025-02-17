#include <fstream>
#include <core/context.h>
#include <gesture/gesturelibrary.h>
#include <gesture/gesturelibraries.h>
#include <porting/cdlog.h>

namespace cdroid{

class GestureLibraries::StreamGestureLibrary:public GestureLibrary {
    // Either a file or an fd is used
private:
    Context* mContext;
    std::string mResourceId;
public:
    StreamGestureLibrary(Context*ctx,const std::string&resid):mContext(ctx),mResourceId(resid){
    }

    bool isReadOnly() const{
        return mContext!=nullptr;
    }

    bool save() override{
        if (!mStore->hasChanged()) return true;
        bool result = false;

        if (mResourceId.size()&&(mContext==nullptr)) {
            std::ofstream ofs(mResourceId);

            //noinspection ResultOfMethodCallIgnored
            mStore->save(ofs, true);
            result = true;
            LOGD("Could not save the gesture library in ",mResourceId.c_str());
        } else {
            FATAL("Could not save the gesture library to %s",mResourceId.c_str());
        }
        return result;
    }

    bool load() override{
        if (!mResourceId.empty()) {
            if(mContext){
                auto fs=mContext->getInputStream(mResourceId,nullptr);
                if(fs&&(*fs))mStore->load(*fs);
            }else{
                std::ifstream fs(mResourceId);
                if(fs)mStore->load(fs, true);
            }
            LOGD("load the gesture library from %s",mResourceId.c_str());
            return true;
        }

        return false;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

GestureLibraries::GestureLibraries() {
}

GestureLibrary* GestureLibraries::fromFile(const std::string& path) {
    return new StreamGestureLibrary(nullptr,path);
}

GestureLibrary* GestureLibraries::fromRawResource(Context* context, const std::string&resourceId) {
    return new StreamGestureLibrary(context, resourceId);
}

}/*endof namespace*/
