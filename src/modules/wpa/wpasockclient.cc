#include "wpasockclient.h"
#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <core/looper.h>
namespace cdroid{

WpaClientSocket::WpaClientSocket() {
    this->needBind = true;
    mLooper=Looper::getDefault();
    LOGD("mLooper=%p",mLooper);
}

WpaClientSocket::~WpaClientSocket() {
    //uv_close((uv_handle_t*)&this->poll_handle, (uv_close_cb)WpaClientSocket::PollCloseCallback);
    if (mCtrl != nullptr){
        mLooper->removeFd(wpa_ctrl_get_fd(mCtrl));
        wpa_ctrl_close(mCtrl);
    }
    if (mMonitor != nullptr) {
        mLooper->removeFd(wpa_ctrl_get_fd(mMonitor));
        wpa_ctrl_detach(mMonitor);
        wpa_ctrl_close(mMonitor);
    }
}

int WpaClientSocket::start() {
    int ret = 0;//uv_poll_start(&this->poll_handle, UV_READABLE, WpaClientSocket::PollCallback);
    LOGD("mCtrl.fd=%d",wpa_ctrl_get_fd(mCtrl));
    mLooper->addFd(wpa_ctrl_get_fd(mCtrl),0,Looper::EVENT_INPUT|Looper::EVENT_OUTPUT,WpaClientSocket::PollCallback,this);
    LOGD("addFd");
    //mLooper->addFd(wpa_ctrl_get_fd(mMonitor),0,Looper::EVENT_INPUT,WpaClientSocket::PollCallback,this);
    return ret;
}

int WpaClientSocket::bind(const char *path) {
    mCtrl = wpa_ctrl_open(path);
    if (mCtrl == nullptr) {
	LOGD("mCtrl=%p",mCtrl);
        return -1;
    }

    mMonitor = wpa_ctrl_open(path);
    if (mMonitor == nullptr) {
	 LOGD("mMonitor",mMonitor);
        return -1;
    }

    if (wpa_ctrl_attach(mMonitor) < 0) {
	LOGD("wpa_ctrl_attach monitor error");
        return -1;
    }

    return 0;
}

int WpaClientSocket::write(const char *command, char *reply, size_t *reply_len) {
    int ret = wpa_ctrl_request(mCtrl, command, strlen(command), reply, reply_len, nullptr);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

void WpaClientSocket::poll() {
    char buf[2048];
    size_t len = sizeof(buf) - 1;

    if (wpa_ctrl_recv(mMonitor, buf, &len) == 0) {
        /*Local<Value> argv[2] = {
          Nan::New("data").ToLocalChecked(),
          Nan::CopyBuffer(buf, len).ToLocalChecked()
        };*/
        //Nan::MakeCallback(Nan::New<Object>(this->This), Nan::New("emit").ToLocalChecked(), 2, argv);
    }
}

int WpaClientSocket::PollCallback(int fd, int events, void*data) {
    WpaClientSocket *p = (WpaClientSocket *)data;
    if(events&Looper::EVENT_INPUT){
        char buf[2048];
        size_t len = sizeof(buf) - 1;
	wpa_ctrl_recv(p->mMonitor, buf, &len);
	buf[len]=0;
	printf("%s",buf);
    }
}
/*
void WpaClientSocket::PollCloseCallback(uv_poll_t * handle) {
  delete handle;
}*/

}
