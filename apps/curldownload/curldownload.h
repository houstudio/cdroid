#pragma once
#include <curl/curl.h>
#include <string>

class CurlDownloader{
private:
    int mTimerFD;
    CURLM *mMultiHandle;
    static int SocketCallback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);
    static int PollCallback(int fd, int events, void*data);
    static int TimerFDCallback(int fd, int events, void* data);
    static int StartTimeoutCallback(CURLM *multi, long timeout_ms, void * data);
public:
    CurlDownloader();
    ~CurlDownloader();
    int addUrl(const std::string&url);   
};


