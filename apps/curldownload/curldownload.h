#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
struct SockInfo;
class CurlDownloader{
public:
    class ConnectionData {
    private:
	std::string url;
	char* data;
	double totalTime;
	int nbBytes;
	CURLcode res;
	int httpStatus;
	time_t startTime;
	int stopppedByTimeout;
    public:
	ConnectionData(const std::string&url);
	virtual ~ConnectionData();
	const std::string getUrl()const;
	int getHttpStatus() const;
	int getNbBytes()const;
	int hasElapsed(time_t timeout);
	int isStoppedByTimeout() const;
	void onDataRead(char* input, size_t size);
	void onConnectionComplete(double time, CURLcode r, int status, int stoppedByTimeout);
    };
private:
    int mTimerFD;
    CURLM *mMulti;
    int mActiveHandles;
    std::vector<CURL*>mEasys;
    static int SocketCallback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);
    static int TimerCallback(int fd, int events, void* data);
    static int MultiTimeCallback(CURLM *multi, long timeout_ms, void * data);
    static int EventHandler(int fd, int events, void *data);
    static size_t WriteHandler(char *ptr, size_t size, size_t nmemb, void *userdata);
    static int ProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    void setsock(SockInfo*f, curl_socket_t s, CURL*e, int act);
    void addsock(curl_socket_t s, CURL *easy, int action);
    void remove_sock(SockInfo *fdp);
    void check_for_timeout();
    void cleanup(int still_running);
    void cleanup_one_connection(ConnectionData* priv, CURL *easy,
        double total_time, CURLcode res, int httpStatus, int stoppedByTimeout);
    CURL*createConnection(ConnectionData* connection);
public:
    CurlDownloader();
    ~CurlDownloader();
    int addConnection(ConnectionData* connections); 
};


