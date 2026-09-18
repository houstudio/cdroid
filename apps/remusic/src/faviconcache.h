// Station favicon disk cache: download once into Context#getCacheDir()
// /favicons/<hash>.<ext>, serve the file path afterwards. Callback lands on
// the main looper (synchronously when already cached); empty path = miss.
#ifndef __REMUSIC_FAVICONCACHE_H__
#define __REMUSIC_FAVICONCACHE_H__

#include <functional>
#include <string>

namespace cdroid { class Context; }

namespace remusic {

class FaviconCache {
public:
    using Cb = std::function<void(const std::string& filePath)>;
    static void load(cdroid::Context* ctx, const std::string& url, Cb cb);
};

} // namespace remusic
#endif
