// Tiny libcurl helpers shared by the online content clients.
#ifndef __REMUSIC_HTTPUTIL_H__
#define __REMUSIC_HTTPUTIL_H__

#include <string>

namespace remusic {

/** Blocking GET with a 15s timeout; empty string on any failure. */
std::string httpGet(const std::string& url);

/** Percent-encode via curl_easy_escape (caller frees nothing). */
std::string urlEncode(const std::string& s);

} // namespace remusic
#endif
