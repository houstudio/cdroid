/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <curldownload.h>
#include <curl/curl.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <unistd.h>
#include <cstring>
#include <cdlog.h>

namespace cdroid{

static void ms2timespec(int ms, struct timespec *ts){
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

CurlDownloader::CurlDownloader(Looper*looper):mLooper(looper){
    curl_global_init(CURL_GLOBAL_ALL);
    mMulti = curl_multi_init();
    mActiveHandles = 0;
    curl_multi_setopt(mMulti, CURLMOPT_SOCKETFUNCTION,SocketCallback);
    curl_multi_setopt(mMulti, CURLMOPT_SOCKETDATA, this);
    //when curl_multi_add_handle is called to add a handle,MultiTimeoutCallback will be called
    curl_multi_setopt(mMulti, CURLMOPT_TIMERFUNCTION,MultiTimeCallback);
    curl_multi_setopt(mMulti, CURLMOPT_TIMERDATA, this);

    struct itimerspec new_value={{0,0},{1,0}};
    ms2timespec(1000,&new_value.it_value);
    ms2timespec(0,&new_value.it_interval);
    mTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    const int rc = timerfd_settime(mTimerFD, 0, &new_value,nullptr);
    mLooper = looper?looper:Looper::getForThread();
    LOGE_IF(looper==nullptr,"Looper is null,you must call Looper::prepare in your calling Thread before Using CurlDownloader");
    mLooper->addFd(mTimerFD,0,Looper::EVENT_INPUT,TimerCallback,this);
    LOGE_IF(rc,"timerfd_settime(%d) error:%d",mTimerFD,rc);
}

CurlDownloader::~CurlDownloader(){
    curl_multi_cleanup(mMulti);
    curl_global_cleanup();
    mLooper->removeFd(mTimerFD);
    close(mTimerFD);
}

CURL * CurlDownloader::createConnection(ConnectionData* connection) {
    CURL *curl = curl_easy_init();
    LOGE_IF(curl==nullptr,"curl_easy_init", curl);
    if (curl) {
        CURLcode err;
        err = curl_easy_setopt(curl, CURLOPT_URL, connection->getUrl().c_str());
        err = curl_easy_setopt(curl, CURLOPT_PRIVATE, connection);
        err = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , WriteHandler);
        err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl);
        err = curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE,10240*1024);
        err = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        //err = curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
        err = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  // 启用详细日志
        err = curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, DebugCallback);  // 设置调试回调
        err = curl_easy_setopt(curl, CURLOPT_DEBUGDATA, curl);
        err = curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,ProgressCallback);
        err = curl_easy_setopt(curl, CURLOPT_PROGRESSDATA,curl);

        LOGD("CURL %p: %s", curl,connection->getUrl().c_str());
    }
    return curl;
}

int CurlDownloader::addConnection(ConnectionData* connection){
    CURL * easy = nullptr;
    if(connection && (easy=createConnection(connection)) ){
        CURLMcode errm = curl_multi_add_handle(mMulti, easy);
        LOGE_IF(errm,"curl_multi_add_handle(%p)=%d",easy,errm);
        mActiveHandles++;
        mEasys.push_back(easy);
    }
    return mEasys.size(); 
}

int CurlDownloader::TimerCallback(int fd, int events, void* data){
    int running;
    CurlDownloader*thiz =(CurlDownloader*)data;
    CURLMcode errm = curl_multi_socket_action(thiz->mMulti,CURL_SOCKET_TIMEOUT,0,&running);
    //LOGD_IF(running,"errm=%d runnings=%d",errm,running);
    thiz->cleanUp(running);
    return running;
}

void CurlDownloader::checkTimeout() {
    for(int i= mEasys.size()-1 ; i >= 0; i --){
        ConnectionData* priv;
        CURL*easy = mEasys[i];
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &priv);
        if (priv->hasElapsed(5000) && ! priv->isStoppedByTimeout()) {
           cleanUpConnection(priv, easy, 0.0, CURLE_OK, 0, 1);
        }
    }
    cleanUp(-1);
}

void CurlDownloader::cleanUpConnection(ConnectionData* priv, CURL *easy,
        double total_time,int res, int httpStatus, int stoppedByTimeout) {
    priv->onConnectionComplete(total_time, res, httpStatus, stoppedByTimeout);

    auto it = std::find(mEasys.begin(),mEasys.end(),easy);
    if(it!=mEasys.end())mEasys.erase(it);
    curl_multi_remove_handle(mMulti, easy);

    curl_easy_cleanup(easy);
    mActiveHandles--;
	LOGV("mEasys.size=%d/%d priv=%p recv %d bytes",mEasys.size(),mActiveHandles,priv,priv->getRecvBytes());
    delete priv;
}

void CurlDownloader::cleanUp(int still_running) {
    int msgsInQueue , httpStatus;
    struct CURLMsg * curlMsg;
    double totalTime;

    while ((curlMsg = curl_multi_info_read(mMulti, &msgsInQueue)) != NULL)  {
        if (curlMsg->msg == CURLMSG_DONE) {
            ConnectionData* priv;
            CURL* curl = curlMsg->easy_handle;
            CURLcode result = curlMsg->data.result;
            curl_easy_getinfo(curl, CURLINFO_PRIVATE, &priv);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
            if(result == CURLE_OPERATION_TIMEDOUT){
                LOGD("%p [%s] timeout",priv,priv->getUrl().c_str());
                cleanUpConnection(priv, curl, totalTime, result, httpStatus, 1);
            }else if(result != CURLE_OK){
                LOGD("%p [%s] error:%s",priv,priv->getUrl().c_str(),curl_easy_strerror(result));
                cleanUpConnection(priv, curl, totalTime, result, httpStatus, 0);
            }else{
                LOGD("%p:%p [%s] completed successfully[%d，%d]",priv,curl,priv->getUrl().c_str(),result,httpStatus);
                cleanUpConnection(priv, curl, totalTime, result, httpStatus, 0);
            }
        }
    }

    if (mActiveHandles== 0) {
        LOGD("Nothing is left, done!");
        mLooper->removeFd(mTimerFD);
    }
}

void CurlDownloader::cleanConnections(){
    for(auto easy:mEasys){
        ConnectionData* priv;
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &priv);
        curl_multi_remove_handle(mMulti, easy);
        curl_easy_cleanup(easy);
        delete priv;
    }
    mActiveHandles = 0;
    mEasys.clear();
}

int CurlDownloader::MultiTimeCallback(CURLM *multi, long timeout_ms, void * data){
    CurlDownloader*thiz =(CurlDownloader*)data;
    struct itimerspec new_value={{0,0},{0,0}};
    if(timeout_ms==0)timeout_ms=10;
    ms2timespec(timeout_ms,&new_value.it_value);
    ms2timespec(timeout_ms,&new_value.it_interval);
    timerfd_settime(thiz->mTimerFD, 0/*TFD_TIMER_ABSTIME*/, &new_value,NULL);
    LOGV("Setting timeout to %ld ms", timeout_ms);
    thiz->mLooper->addFd(thiz->mTimerFD,0,Looper::EVENT_INPUT,TimerCallback,thiz);
    return 0;    
}

static int CURLAction2Event(int action){
    int events = 0;
    if(action & CURL_POLL_IN)events |= Looper::EVENT_INPUT;
    if(action & CURL_POLL_OUT)events |= Looper::EVENT_OUTPUT;
    return events;
}

int CurlDownloader::SocketCallback(CURL *easy, int socket, int action, void *userp, void *socketp){
    const char* whatstr[] = { "none", "IN", "OUT", "INOUT", "REMOVE" };
    CurlDownloader* thiz = (CurlDownloader*) userp;
    if (action == CURL_POLL_REMOVE) {
        thiz->mLooper->removeFd(socket);
        LOGV("easy=%p %d action = %s",easy,socket, whatstr[action]);
    } else {
        const int kind = CURLAction2Event(action);
        LOGV("easy=%p %d action = %s",easy,socket, whatstr[action]);
        thiz->mLooper->addFd(socket,0,kind,EventHandler,thiz);
    }
    return 0;
}

size_t CurlDownloader::WriteHandler(void *ptr, size_t size, size_t nmemb, void *userdata) {
    const int receivedBytes = size * nmemb;
    CURL *curl = (CURL *) userdata;
    char* priv_in  = nullptr;
    double content_size;

    CURLcode err = curl_easy_getinfo(curl, CURLINFO_PRIVATE, &priv_in);
    err = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_size);
    LOGE_IF(err,"curl_easy_getinfo (CURLINFO_PRIVATE)", err);
    LOGV("%p recved %d/%.f bytes",curl,receivedBytes,content_size);
    ConnectionData * priv = (ConnectionData *) priv_in;
    priv->onDataReady(ptr, receivedBytes);
    priv->mReceivedBytes +=receivedBytes;
    return receivedBytes;
}

int CurlDownloader::ProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow){
    ConnectionData * priv;
    curl_easy_getinfo((CURL*)clientp, CURLINFO_PRIVATE, &priv);
    LOGV("clientp=%p download=%.f/%.f %s",clientp,dlnow,dltotal,priv->getUrl().c_str());
    priv->onDownloadProgress(dlnow,dltotal);
    return 0;
}

int CurlDownloader::EventHandler(int fd, int events, void *arg){
    CurlDownloader*thiz = (CurlDownloader*) arg;
    int running_handles,max_retry =5;
    CURLMcode errm;
    do {
        errm = curl_multi_socket_action(thiz->mMulti, fd, 0, &running_handles);
        LOGE_IF(errm,"curl_multi_socket_action", errm);
        LOGV("calling curl_multi_socket_action for fd %d events=%d runnings=%d errm=%d", fd,events,running_handles,errm);
    } while (errm == CURLM_CALL_MULTI_PERFORM && max_retry-- > 0);
    thiz->cleanUp(running_handles);
    return errm==CURLM_OK;//return 0 will caused fd removed by Looper
}

int CurlDownloader::DebugCallback(void *handle, int type, char *data, size_t size, void *userp) {
    const char *text;
    (void)handle; /* prevent compiler warning */
    (void)userp;
#if !defined(NDEBUG)
    switch (type) {
    case CURLINFO_TEXT:
        LOGV("CURL: %.*s", (int)size, data);
        break;
    case CURLINFO_HEADER_OUT:
        LOGV("CURL > %.*s", (int)size, data);
        break;
    case CURLINFO_DATA_OUT:
        LOGD("CURL > %d bytes data", (int)size);
        break;
    case CURLINFO_SSL_DATA_OUT:
        LOGD("CURL > %d bytes SSL data", (int)size);
        break;
    case CURLINFO_HEADER_IN:
        LOGV("CURL < %.*s", (int)size, data);
        break;
    case CURLINFO_DATA_IN:
        LOGV("CURL < %d bytes data", (int)size);
        break;
    case CURLINFO_SSL_DATA_IN:
        LOGD("CURL < %d bytes SSL data", (int)size);
        break;
    default: /* in case a new one is introduced to shock us */
        return 0;
    }
#endif
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////

CurlDownloader::ConnectionData::ConnectionData(const std::string&input) {
    url = input;
    totalTime  = 0;
    httpStatus = 0;
    curlResult = CURLE_OK;
    mReceivedBytes = 0;
    stopppedByTimeout = 0;
    startTime = SystemClock::uptimeMillis();
    LOGD("%p [%s]",this,url.c_str());
}

CurlDownloader::ConnectionData::~ConnectionData() {
    LOGD("%p %d bytes [%s]",this,mReceivedBytes,url.c_str());
}

const std::string CurlDownloader::ConnectionData::getUrl()const{
    return url;
}

int CurlDownloader::ConnectionData::getHttpStatus() const{
    return httpStatus;
}

int CurlDownloader::ConnectionData::getRecvBytes()const {
    return mReceivedBytes;
}

int CurlDownloader::ConnectionData::hasElapsed(long timeout) {
    auto currentTime = SystemClock::uptimeMillis();
    return (startTime + timeout < currentTime) ? 1 : 0;
}

int CurlDownloader::ConnectionData::isStoppedByTimeout() const{
    return stopppedByTimeout;
}

void CurlDownloader::ConnectionData::onDataReady(void* input, size_t size) {
    //data = (char*)(data?realloc(data, (nbBytes + size)):malloc(size));
    //LOGE_IF(data==nullptr,"malloc/realloc data=%p", data);
    //(void) memcpy(data + nbBytes, input, size);
    //nbBytes += size;
    //LOGV("Got %d/%d bytes for connection %s\n",size, nbBytes, url.c_str());
}

void CurlDownloader::ConnectionData::onConnectionComplete(double time,int r/*CURLcode*/, int status, int stoppedByTimeout) {
    totalTime = time;
    curlResult  = r;
    httpStatus = status;
    stopppedByTimeout = 1;
    //LOGD("%p: %s  %ld bytes received",this,url.c_str(),nbBytes);
}

void CurlDownloader::ConnectionData::onDownloadProgress(double now,double total){
}

}/*endof namespace*/
