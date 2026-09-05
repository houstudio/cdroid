#include "httputil.h"

#ifdef REMUSIC_ONLINE

#include <curl/curl.h>
#include <cstdio>
#include <unistd.h>

#include <porting/cdlog.h>

namespace remusic {

static size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    ((std::string*)userdata)->append(ptr, size * nmemb);
    return size * nmemb;
}

static thread_local std::string tLastError;

std::string lastHttpError() { return tLastError; }

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
    tLastError.clear();
    if (rc != CURLE_OK) {
        LOGE("httpGet: curl %s (%s)", curl_easy_strerror(rc), url.c_str());
        tLastError = curl_easy_strerror(rc);
        body.clear();
    }
    return body;
}

namespace {
struct DownloadCtx {
    FILE* f;
    const std::function<void(long, long)>* progress;
};
size_t fileWriteCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    DownloadCtx* ctx = (DownloadCtx*) userdata;
    return fwrite(ptr, size, nmemb, ctx->f) == size * nmemb ? size * nmemb : 0;
}
int xferCb(void* userdata, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t, curl_off_t) {
    DownloadCtx* ctx = (DownloadCtx*) userdata;
    if (ctx->progress && *ctx->progress) (*ctx->progress)((long) dlNow, (long) dlTotal);
    return 0;
}
} // namespace

bool httpDownload(const std::string& url, const std::string& destPath,
        const std::function<void(long, long)>& progress) {
    static const bool sInit = curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
    (void) sInit;
    const std::string tmp = destPath + ".part";
    FILE* f = fopen(tmp.c_str(), "wb");
    if (f == nullptr) {
        LOGE("httpDownload: cannot open %s", tmp.c_str());
        return false;
    }
    DownloadCtx ctx = {f, &progress};
    CURL* curl = curl_easy_init();
    if (curl == nullptr) { fclose(f); unlink(tmp.c_str()); return false; }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "remusic-cdroid/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileWriteCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferCb);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);   // enable the progress cb
    const CURLcode rc = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(f);
    if (rc != CURLE_OK) {
        LOGE("httpDownload: curl %s (%s)", curl_easy_strerror(rc), url.c_str());
        unlink(tmp.c_str());
        return false;
    }
    if (rename(tmp.c_str(), destPath.c_str()) != 0) {
        unlink(tmp.c_str());
        return false;
    }
    return true;
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
