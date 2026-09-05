#include "httputil.h"

#ifdef REMUSIC_ONLINE

#include <curl/curl.h>

#include <porting/cdlog.h>

namespace remusic {

static size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    ((std::string*)userdata)->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string httpGet(const std::string& url) {
    static const bool sInit = curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
    (void) sInit;
    std::string body;
    CURL* curl = curl_easy_init();
    if (curl == nullptr) return body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "remusic-cdroid/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    const CURLcode rc = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (rc != CURLE_OK) {
        LOGE("httpGet: curl %s (%s)", curl_easy_strerror(rc), url.c_str());
        body.clear();
    }
    return body;
}

std::string urlEncode(const std::string& s) {
    CURL* curl = curl_easy_init();
    if (curl == nullptr) return s;
    char* esc = curl_easy_escape(curl, s.c_str(), (int)s.size());
    std::string out = esc ? esc : s;
    if (esc) curl_free(esc);
    curl_easy_cleanup(curl);
    return out;
}

} // namespace remusic

#endif // REMUSIC_ONLINE
