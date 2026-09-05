#include "faviconcache.h"

#ifdef REMUSIC_ONLINE

#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "httputil.h"
#include <core/context.h>
#include <core/handler.h>
#include <core/looper.h>

namespace remusic {

static std::string extOf(const std::string& url) {
    const size_t slash = url.find_last_of('/');
    const size_t dot = url.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return ".png";
    std::string ext = url.substr(dot);
    if (ext.size() > 5 || ext.find_first_not_of(".abcdefghijklmnopqrstuvwxyz0123456789")
            != std::string::npos)
        return ".png";
    return ext;
}

static std::string cachePath(cdroid::Context* ctx, const std::string& url) {
    static const char* kDir = "favicons";
    const std::string dir = ctx->getCacheDir() + "/" + kDir;
    mkdir(dir.c_str(), 0755);   // EEXIST is fine
    return dir + "/" + std::to_string(std::hash<std::string>{}(url)) + extOf(url);
}

void FaviconCache::load(cdroid::Context* ctx, const std::string& url, Cb cb) {
    if (ctx == nullptr || url.empty()
            || (url.compare(0, 7, "http://") != 0 && url.compare(0, 8, "https://") != 0)) {
        cb(std::string());
        return;
    }
    const std::string path = cachePath(ctx, url);
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && st.st_size > 0) {
        cb(path);   // synchronous hit — caller is already on the UI thread
        return;
    }
    std::thread([url, path, cb = std::move(cb)]() mutable {
        const std::string bytes = httpGet(url);
        std::string result;
        if (!bytes.empty()) {
            const std::string tmp = path + ".tmp";
            FILE* f = fopen(tmp.c_str(), "wb");
            if (f != nullptr) {
                fwrite(bytes.data(), 1, bytes.size(), f);
                fclose(f);
                if (rename(tmp.c_str(), path.c_str()) == 0) result = path;
                else unlink(tmp.c_str());
            }
        }
        static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
        sMain.post([result, cb = std::move(cb)]() { cb(result); });
    }).detach();
}

} // namespace remusic

#endif // REMUSIC_ONLINE
