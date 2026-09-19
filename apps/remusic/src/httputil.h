// Tiny libcurl helpers shared by the online content clients.
#ifndef __REMUSIC_HTTPUTIL_H__
#define __REMUSIC_HTTPUTIL_H__

#include <functional>
#include <string>

namespace remusic {

/** Blocking GET with a 15s timeout; empty string on any failure. */
std::string httpGet(const std::string& url);

/** Why the last httpGet on THIS thread failed ("" when it succeeded) —
 *  thread-local, safe to read right after an empty return. */
std::string lastHttpError();

/** Streaming download to destPath (".part" then rename). Progress runs on
 *  the transfer thread; returns false on any failure (partial removed). */
bool httpDownload(const std::string& url, const std::string& destPath,
        const std::function<void(long done, long total)>& progress);

/** Percent-encode via curl_easy_escape (caller frees nothing). */
std::string urlEncode(const std::string& s);

} // namespace remusic
#endif
