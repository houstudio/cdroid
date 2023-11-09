#include <curldownload.h>
#include <core/looper.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <cstring>
using namespace cdroid;

struct SockInfo {
    curl_socket_t sockfd;
    CURL *easy;
    int action;
    //struct event ev;
    int evset;
};
static void ms2timespec(int ms, struct timespec *ts);

CurlDownloader::CurlDownloader(){
    curl_global_init(CURL_GLOBAL_ALL);
    mMultiHandle = curl_multi_init();
    mActiveHandles = 0;
    curl_multi_setopt(mMultiHandle, CURLMOPT_SOCKETFUNCTION,SocketCallback);
    curl_multi_setopt(mMultiHandle, CURLMOPT_SOCKETDATA, this);
    //when curl_multi_add_handle is called to add a handle,StartTimeoutCallback will be called
    curl_multi_setopt(mMultiHandle, CURLMOPT_TIMERFUNCTION,MultiTimeCallback);
    curl_multi_setopt(mMultiHandle, CURLMOPT_TIMERDATA, this);

    struct itimerspec new_value={{0,0},{1,0}};
    ms2timespec(1000,&new_value.it_value);
    ms2timespec(0,&new_value.it_interval);
    mTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    int rc=timerfd_settime(mTimerFD, 0, &new_value,nullptr);
    Looper::getDefault()->addFd(mTimerFD,0,Looper::EVENT_INPUT,TimerCallback,this);
    LOGD("timerfd_settime(%d) error:%d",mTimerFD,rc);

    ms2timespec(1000,&new_value.it_interval);
    mClockFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    rc =timerfd_settime(mClockFD, 0, &new_value,nullptr);
    Looper::getDefault()->addFd(mClockFD,0,Looper::EVENT_INPUT,ClockCallback,this);
    LOGD("timerfd_settime(%d) error:%d",mClockFD,rc);
}

CurlDownloader::~CurlDownloader(){
    curl_multi_cleanup(mMultiHandle);
    curl_global_cleanup();
}

CURL * CurlDownloader::createConnection(ConnectionData* connection) {
    CURL *curl = curl_easy_init();
    LOGE_IF(curl==nullptr,"curl_easy_init", curl);
    if (curl) {
        CURLcode err = curl_easy_setopt(curl, CURLOPT_URL, connection->getUrl().c_str());
        LOGE_IF(err,"curl_easy_setopt (CURLOPT_URL)", err);

        err = curl_easy_setopt(curl, CURLOPT_PRIVATE, connection);
        LOGE_IF(err,"curl_easy_setopt (CURLOPT_PRIVATE)", err);

        err = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , WriteHandler);
        LOGE_IF(err,"curl_easy_setopt (CURLOPT_WRITEFUNCTION)", err);

        err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl);
        LOGE_IF(err,"curl_easy_setopt (CURLOPT_WRITEDATA)", err);
/*
        -- does not seem to work with curl_multi --
        err = curl_easy_setopt(curl, CURLOPT_TIMEOUT, EASY_TIMEOUT);
        LOGE_IF(err,"curl_easy_setopt (CURLOPT_TIMEOUT)", err);
*/

	LOGD("Setting easy options for  %s <END>", connection->getUrl().c_str());
    }
    return curl;
}

int CurlDownloader::addConnection(ConnectionData* connection){
    if(connection){
        CURL * easy = createConnection(connection);
        if (easy != NULL) {
            CURLMcode errm = curl_multi_add_handle(mMultiHandle, easy);
            LOGE_IF(errm,"curl_multi_add_handle", errm);
            mActiveHandles++;
            mEasys.push_back(easy);
        }
    }
    return mEasys.size(); 
}

int CurlDownloader::PollCallback(int fd, int events, void*data){
    if(events&Looper::EVENT_INPUT){
	 CURL *curl = (CURL*)data;
    }
    LOGD("fd=%d events=%d",fd,events);
    return 1;
}

int CurlDownloader::TimerCallback(int fd, int events, void* data){
    int still_running;
    CurlDownloader*thiz =(CurlDownloader*)data;
    CURLMcode errm = curl_multi_socket_action(thiz->mMultiHandle,
         CURL_SOCKET_TIMEOUT,0,&still_running);
    LOGV("errm=%d runnings=%d",errm,still_running);
    thiz->cleanup(still_running);
    return still_running;
}

int CurlDownloader::ClockCallback(int fd, int events, void* data){
    CurlDownloader*thiz = (CurlDownloader*)data;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    //evtimer_add(instance->getClockEv(), &timeout);
    thiz->check_for_timeout();
    return 1;
}

void CurlDownloader::check_for_timeout() {
    for(auto easy:mEasys){
        ConnectionData* priv;
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &priv);
        time_t timeout = 5;
	if (priv->hasElapsed(timeout) && ! priv->isStoppedByTimeout()) {
           cleanup_one_connection(priv, easy, 0.0, CURLE_OK, 0, 1);
        }
    }
    cleanup(-1);
}

void CurlDownloader::cleanup_one_connection(ConnectionData* priv, CURL *easy,
        double total_time, CURLcode res, int httpStatus, int stoppedByTimeout) {
    priv->onConnectionComplete(total_time, res, httpStatus, stoppedByTimeout);
    curl_multi_remove_handle(mMultiHandle, easy);
    curl_easy_cleanup(easy);
    mActiveHandles--;
}

void CurlDownloader::cleanup(int still_running) {
    int msgs_in_queue;
    struct CURLMsg * curlm_msg;
    CURL *easy;
    CURLcode res;
    int httpStatus;
    double total_time;
    ConnectionData* priv;

    while ((curlm_msg = curl_multi_info_read(mMultiHandle, &msgs_in_queue)) != NULL)  {
        easy = curlm_msg->easy_handle;
        res = curlm_msg->data.result;
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &priv);
	curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME, &total_time);
	curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &httpStatus);
	if (curlm_msg->msg == CURLMSG_DONE) {
	    cleanup_one_connection(priv, easy, total_time, res, httpStatus, 0);
	}
    }

    if (still_running != -1) {
        LOGV("CLEANUP running = %d , left = %d", still_running,mActiveHandles);
    }

    if (mActiveHandles== 0) {
        LOGD("Nothing is left, done!");
	Looper::getDefault()->removeFd(mTimerFD);
	Looper::getDefault()->removeFd(mClockFD);
        //evtimer_del(instance->getTimerEv());
        //evtimer_del(instance->getClockEv());
    }
}

int CurlDownloader::MultiTimeCallback(CURLM *multi, long timeout_ms, void * data){
    CurlDownloader*thiz =(CurlDownloader*)data;
    struct itimerspec new_value={{0,0},{0,0}};
    if(timeout_ms==0)timeout_ms=10;
    ms2timespec(timeout_ms,&new_value.it_value);
    ms2timespec(timeout_ms,&new_value.it_interval);
    timerfd_settime(thiz->mTimerFD, 0/*TFD_TIMER_ABSTIME*/, &new_value,NULL);
    LOGD("Setting timeout to %ld ms", timeout_ms);
    Looper::getDefault()->addFd(thiz->mTimerFD,0,Looper::EVENT_INPUT,TimerCallback,thiz);
    //evtimer_add(instance->getTimerEv(), &timeout);
    return 0;    
}

int CurlDownloader::SocketCallback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp){
    const char* whatstr[] = { "none", "IN", "OUT", "INOUT", "REMOVE" };
    LOGD("socket_callback_from_curl s  = %d action = %s <START>", s, whatstr[action]);

    CurlDownloader* thiz = (CurlDownloader*) userp;

    SockInfo *fdp = (SockInfo *) socketp;
    if (action == CURL_POLL_REMOVE) {
  	thiz->remove_sock(fdp);
    } else {
	if (!fdp) {
	    thiz->addsock(s, easy, action);
	} else {
	    thiz->setsock(fdp, s, easy, action);
	}
    }
    return 0;
}

size_t CurlDownloader::WriteHandler(char *ptr, size_t size, size_t nmemb, void *userdata) {
    int received = size * nmemb;
    CURL *curl = (CURL *) userdata;
    char* priv_in  = NULL;

    CURLcode err = curl_easy_getinfo(curl, CURLINFO_PRIVATE, &priv_in);
    LOGE_IF(err,"curl_easy_getinfo (CURLINFO_PRIVATE)", err);
    LOGD("====%d bytes",size*nmemb);
    printf("%.*s\n", size*nmemb,ptr);
    ConnectionData * priv = (ConnectionData *) priv_in;
    priv->onDataRead(ptr, received);
    return received;
}

int CurlDownloader::EventHandler(int fd, int events, void *arg){
    CurlDownloader*thiz = (CurlDownloader*) arg;
    LOGD("event_handler socket = %d events = %d", fd, events);
    int running_handles;
    CURLMcode errm;
    int max_retry = 5;
    do {
        LOGD("calling curl_multi_socket_action for fd %d <BEFORE>", fd);
        errm = curl_multi_socket_action(thiz->mMultiHandle, fd, 0, &running_handles);
        LOGE_IF(errm,"curl_multi_socket_action", errm);
        LOGD("calling curl_multi_socket_action for fd %d <AFTER> runnings=%d errm=%d", fd,running_handles,errm);
    } while (errm == CURLM_CALL_MULTI_PERFORM && max_retry-- > 0);
    thiz->cleanup(running_handles);
    return errm==CURLM_OK;//return 0 will caused fd removed by Looper
}

void CurlDownloader::setsock(SockInfo*f, curl_socket_t s, CURL*e, int act){
    LOGD("setsock s = %d act = %d", s, act);

    int kind = 0;
    if(act & CURL_POLL_IN ) kind |= Looper::EVENT_INPUT;
    if(act & CURL_POLL_OUT) kind |= Looper::EVENT_OUTPUT;// | EV_PERSIST;
    f->sockfd = s;
    f->action = act;
    f->easy = e;
    if (f->evset) {
        //event_del(&f->ev);
    }
    Looper::getDefault()->addFd(f->sockfd,0,kind,EventHandler,this);
    //event_set(&f->ev, f->sockfd, kind, event_handler, instance);
    f->evset = 1;
    //event_add(&f->ev, NULL);
}

void CurlDownloader::addsock(curl_socket_t s, CURL *easy, int action) {
    SockInfo *fdp = (SockInfo *) calloc(1, sizeof(SockInfo));
    LOGE_IF(fdp==nullptr,"calloc (SockInfo)", fdp);
    setsock(fdp, s, easy, action);
    curl_multi_assign(mMultiHandle, s, fdp);
}

void CurlDownloader::remove_sock(SockInfo *fdp) {
    if (fdp->evset) {
        //event_del(&fdp->ev);
        Looper::getDefault()->removeFd(fdp->sockfd);
    }
    LOGD("remove socket %d evset=%d",fdp->sockfd,fdp->evset);
    free(fdp);
}
///////////////////////////////////////////////////////////////////////////////////////////////

CurlDownloader::ConnectionData::ConnectionData(const std::string&input) {
    url = input;
    nbBytes = 0;
    totalTime = 0;
    httpStatus = 0;
    res = CURLE_OK;
    data = nullptr;
    stopppedByTimeout = 0;
    (void) ctime(&startTime);
}

CurlDownloader::ConnectionData::~ConnectionData() {
    free(data);
}

const std::string CurlDownloader::ConnectionData::getUrl()const{
    return url;
}

int CurlDownloader::ConnectionData::getHttpStatus() const{
    return httpStatus;
}

int CurlDownloader::ConnectionData::getNbBytes()const {
    return nbBytes;
}

int CurlDownloader::ConnectionData::hasElapsed(time_t timeout) {
    time_t currentTime;
    (void) ctime(&currentTime);
    return (startTime + timeout < currentTime) ? 1 : 0;
}

int CurlDownloader::ConnectionData::isStoppedByTimeout() const{
    return stopppedByTimeout;
}

void CurlDownloader::ConnectionData::onDataRead(char* input, size_t size) {
    data = (data == NULL) ? (char* ) malloc(size) : (char* ) realloc(data, (nbBytes + size));
    LOGE_IF(data==nullptr,"malloc/realloc data", data);
    (void) memcpy(data + nbBytes, input, size);
    nbBytes += size;
    LOGD("Got %d bytes for connection %s\n", nbBytes, url.c_str());
}

void CurlDownloader::ConnectionData::onConnectionComplete(double time, CURLcode r, int status, int stoppedByTimeout) {
    totalTime = time;
    res  = r;
    httpStatus = status;
    stopppedByTimeout = 1;
    LOGD("%s download %ld bytes finished",url.c_str(),nbBytes);
}

static void ms2timespec(int ms, struct timespec *ts){
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

///////////////////////////////////////////////////////////////////////////////////////
