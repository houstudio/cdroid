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
    private:
	std::string url;
	char* data;
	double totalTime;
	int nbBytes;
	int res;
	int httpStatus;
	time_t startTime;
	int stopppedByTimeout;
    public:
	ConnectionData(const std::string&url);
	virtual ~ConnectionData();
	const std::string getUrl()const;
	int getHttpStatus() const;
	int getNbBytes()const;
	int hasElapsed(long timeout);
	int isStoppedByTimeout() const;
	void onDataRead(char* input, size_t size);
	void onConnectionComplete(double time,int r, int status, int stoppedByTimeout);
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
    static int EventHandler(int fd, int events, void *data);
    static size_t WriteHandler(char *ptr, size_t size, size_t nmemb, void *userdata);
    static int ProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    void setsock(SockInfo*f, int socket, void*e, int act);
    void addsock(int socket, void *easy, int action);
    void remove_sock(SockInfo *fdp);
    void check_for_timeout();
    void cleanup(int still_running);
    void cleanup_one_connection(ConnectionData* priv,void*easy,
        double total_time,int res, int httpStatus, int stoppedByTimeout);
    void*createConnection(ConnectionData* connection);
public:
    CurlDownloader(cdroid::Looper*looper=nullptr);
    ~CurlDownloader();
    int addConnection(ConnectionData* connections); 
};
}/*endof namespace*/
#endif //__CURL_DOWNLOADER_H__

