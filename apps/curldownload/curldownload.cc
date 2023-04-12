#include <curldownload.h>
#include <core/looper.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

using namespace cdroid;

CurlDownloader::CurlDownloader(){
    curl_global_init(CURL_GLOBAL_ALL);
    mMultiHandle = curl_multi_init();
    curl_multi_setopt(mMultiHandle, CURLMOPT_SOCKETFUNCTION,SocketCallback);
    //when curl_multi_add_handle is called to add a handle,StartTimeoutCallback will be called
    curl_multi_setopt(mMultiHandle, CURLMOPT_TIMERFUNCTION,StartTimeoutCallback);
    curl_multi_setopt(mMultiHandle, CURLMOPT_TIMERDATA, this);

    struct itimerspec new_value={{0,0},{1,0}};
    mTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    const int rc=timerfd_settime(mTimerFD, 0, &new_value,nullptr);
    LOGD("timerfd_settime(%d) error:%d",mTimerFD,rc);
    Looper::getDefault()->addFd(mTimerFD,0,Looper::EVENT_INPUT,TimerFDCallback,this);
}

CurlDownloader::~CurlDownloader(){
    curl_multi_cleanup(mMultiHandle);
    curl_global_cleanup();
}

int CurlDownloader::PollCallback(int fd, int events, void*data){
    if(events&Looper::EVENT_INPUT){
	 CURL *curl = (CURL*)data;
    }
}

int CurlDownloader::TimerFDCallback(int fd, int events, void* data){
    int count,still_running;
    char *done_url;
    read(fd, &count, sizeof(uint64_t));
    CurlDownloader*thiz =(CurlDownloader*)data;

    CURLMsg *message;
    int pending; 
    LOGD("TimerFDCallback(%d) events=%x",fd,events);
    while ((message = curl_multi_info_read(thiz->mMultiHandle, &pending))) {
        switch (message->msg) {
        case CURLMSG_DONE:
            curl_easy_getinfo(message->easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
            printf("%s DONE\n", done_url);
            curl_multi_remove_handle(thiz->mMultiHandle, message->easy_handle);
            curl_easy_cleanup(message->easy_handle);
            break;
        default:
            fprintf(stderr, "CURLMSG default\n");
            abort();
        }
    }    
}

int CurlDownloader::StartTimeoutCallback(CURLM *multi, long timeout_ms, void * data){
    struct itimerspec ts={{0,0},{0,0}} ;
    CurlDownloader* thiz = (CurlDownloader*)data;
    if(timeout_ms==0) timeout_ms = 10;
    ts.it_interval.tv_sec  = timeout_ms / 1000 ;
    ts.it_interval.tv_nsec = (timeout_ms % 1000) * 1000000 ;
    timerfd_settime(thiz->mTimerFD,0,&ts, NULL) ;
    LOGD("multi_timer_cb: Setting timeout to %ld ms", timeout_ms);
}

int CurlDownloader::SocketCallback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp){

}

static size_t write_handler(char *ptr, size_t size, size_t nmemb, void *userdata){
    LOG_DUMP("",(const BYTE*)ptr,size); 
}

int CurlDownloader::addUrl(const std::string&url){
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_handler);
        
    curl_multi_add_handle (mMultiHandle, curl);
}
