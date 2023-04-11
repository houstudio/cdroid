#include <wpa_ctrl.h>

namespace cdroid{

class WpaClientSocket {
public:
    /*static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_METHOD(Start);
    static NAN_METHOD(Bind);
    static NAN_METHOD(Write);*/
private:
    void poll();
    static int PollCallback(int fd, int events, void*data);
private:
    struct wpa_ctrl *mCtrl;
    struct wpa_ctrl *mMonitor;
    class Looper*mLooper;
    bool needBind;
public:
    WpaClientSocket();
    ~WpaClientSocket();
    int start();
    int bind(const char *path);
    int write(const char *command, char *reply, size_t *reply_len);
};

}
