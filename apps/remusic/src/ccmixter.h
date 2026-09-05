// ccMixter (ccmixter.org) client — the on-demand fallback source for
// networks that cannot reach Audius. Key-free JSON API (`f=json`), charts
// via sort=rank, text/tag search via search_text; every upload is
// CC-licensed. The media host hotlink-checks Referer, so FFmpeg must send
// one (ffmpegbackend.cc does for *.ccmixter.org).
// Rows are delivered as AudiusTrack (the panel's shared row model).
#ifndef __REMUSIC_CCMIXTER_H__
#define __REMUSIC_CCMIXTER_H__

#include <functional>
#include <string>
#include <vector>

#include "audius.h"

namespace remusic {

class CcMixter {
public:
    using Cb = std::function<void(std::vector<AudiusTrack>, const std::string& error)>;

    /** Rank-sorted chart; `tag` narrows it (usertags like "electronic"),
     *  empty = the whole chart — browsable with no search term. */
    static void chart(const std::string& tag, Cb onDone);
    static void search(const std::string& text, Cb onDone);
};

} // namespace remusic
#endif
