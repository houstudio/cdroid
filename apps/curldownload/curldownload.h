#ifndef __CURL_DOWNLOADER_H__
#define __CURL_DOWNLOADER_H__
#include <string>
#include <vector>

namespace cdroid{
struct SockInfo;
class Looper;
class CurlDownloader{
public:
    class ConnectionData {
    protected:
        friend CurlDownloader;
	    std::string url;
	    double totalTime;
	    int curlResult;
	    int httpStatus;
	    time_t startTime;
        size_t mReceivedBytes;
	    int stopppedByTimeout;
    public:
	    ConnectionData(const std::string&url);
	    virtual ~ConnectionData();
	    const std::string getUrl()const;
	    int getHttpStatus() const;
	    int getRecvBytes()const;
	    int hasElapsed(long timeout);
	    int isStoppedByTimeout() const;
	    virtual void onDataReady(void* input, size_t size);
        virtual void onDownloadProgress(double done,double total);
	    virtual void onConnectionComplete(double time,int r, int status, int stoppedByTimeout);
    };
private:
    int mTimerFD;
    void *mMulti;
    int mActiveHandles;
    Looper*mLooper;
    std::vector<void*>mEasys;
    static int SocketCallback(void *easy,int socket, int action, void *userp, void *socketp);
    static int TimerCallback(int fd, int events, void* data);
    static int MultiTimeCallback(void*multi, long timeout_ms, void * data);
    static int DebugCallback(void *handle, int type, char *data, size_t size, void *userp);
    static int EventHandler(int fd, int events, void *data);
    static size_t WriteHandler(void *ptr, size_t size, size_t nmemb, void *userdata);
    static int ProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    void checkTimeout();
    void cleanUp(int still_running);
    void cleanUpConnection(ConnectionData* priv,void*easy,
        double total_time,int res, int httpStatus, int stoppedByTimeout);
    void*createConnection(ConnectionData* connection);
public:
    CurlDownloader(cdroid::Looper*looper=nullptr);
    ~CurlDownloader();
    int addConnection(ConnectionData* connections); 
    void cleanConnections();
};
}/*endof namespace*/
#endif //__CURL_DOWNLOADER_H__

