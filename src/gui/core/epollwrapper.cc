#include <map>
#include <stdexcept>
#include <core/looper.h>
#include <core/epollwrapper.h>
#include <porting/cdlog.h>

#if defined(__linux__)||defined(__unix__)
#include <unistd.h>
#define EPOLL_HANDLE int
#define INVALID_HANDLE_VALUE (intptr_t(-1))
#elif defined(_WIN32)||defined(_WIN64)
#define EPOLL_HANDLE HANDLE
#define INVALID_HANDLE_VALUE (intptr_t(-1))
#include <WinSock2.h>
#define close _close
#endif

//REFERENCES:
//Fast epoll for windows:https://github.com/piscisaureus/wepoll 
#define USE_SELECT 1
namespace cdroid{

#if (defined(_WIN32)||defined(_WIN64)||defined(__linux__)||defined(__unix__))&&!defined(USE_SELECT)
class EPOLL:public IOEventProcessor {
private:
    EPOLL_HANDLE epfd;
    int maxEvents;
private:
    uint32_t toEpollEvents(uint32_t events) const{
        uint32_t epollEvents = 0;
        if (events & Looper::EVENT_INPUT) epollEvents |= EPOLLIN;
        if (events & Looper::EVENT_OUTPUT)epollEvents |= EPOLLOUT;
        if (events & Looper::EVENT_HANGUP)epollEvents |= EPOLLHUP;
        if (events & Looper::EVENT_ERROR) epollEvents |= EPOLLERR;
        return epollEvents;
    }
    uint32_t toLoopEvents(uint32_t events){
        uint32_t loopEvents = 0;
        if (events & EPOLLIN) loopEvents|=Looper::EVENT_INPUT;
        if (events &EPOLLOUT) loopEvents|=Looper::EVENT_OUTPUT;
        if (events &EPOLLHUP) loopEvents|=Looper::EVENT_HANGUP;
        if (events &EPOLLERR) loopEvents|=Looper::EVENT_ERROR;
        return loopEvents;
    }
public:
    explicit EPOLL(int maxEvents = 10) : maxEvents(maxEvents) {
        epfd = epoll_create1(0);
        if (epfd == (EPOLL_HANDLE)INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to create epoll file descriptor");
        }
    }

    ~EPOLL() {
#if defined(__linux__)||defined(__unix__)
        close(epfd);
#elif defined(_WIN32)||defined(_WIN64)
        epoll_close(epfd);
#endif
    }


    int addFd(int fd,struct epoll_event&event)override {
        event.events = toEpollEvents(event.events);
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1) {
            return -1;
        }
        return 0;
    }

    int removeFd(int fd)override {
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
            return -1;
        }
        return 0;
    }

    int modifyFd(int fd,struct epoll_event&event) override{
        event.events = toEpollEvents(event.events);
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) == -1) {
            return -1;
        }
        return 0;
    }

    int waitEvents(std::vector<epoll_event>& activeFDs, uint32_t timeout) override{
        struct std::vector<epoll_event> events(maxEvents);
        const int numEvents = epoll_wait(epfd, events.data(), maxEvents, timeout);
        if (numEvents == -1) {
            return -1;
        }
        activeFDs.clear();
        for (int i = 0; i < numEvents; ++i) {
            events[i].events=toLoopEvents(events[i].events);
            activeFDs.push_back(events[i]);
        }
        return int(activeFDs.size());
    }
};

#else

class SELECTOR : public IOEventProcessor {
private:
    fd_set readSet;
    fd_set writeSet;
    int maxFD;
    std::map<int,uint64_t>mSeqs;
public:
    SELECTOR() {
        maxFD = 0;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
#if defined(_WIN32)||defined(_WIN64)
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        LOGI("WSAStartup=%d", result);
#endif
    }
    ~SELECTOR()override {
#if defined(_WIN32)||defined(_WIN64)
        WSACleanup();
#endif
    }
    int addFd(int fd, struct epoll_event& e) override {
        if (e.events & Looper::EVENT_INPUT)  FD_SET(fd, &readSet);
        if (e.events & Looper::EVENT_OUTPUT) FD_SET(fd, &writeSet);
        if (fd > maxFD) maxFD = fd;
        mSeqs.insert({fd,uint64_t(e.data.u64)});
        return 0;
    }

    int removeFd(int fd) override {
        auto it = mSeqs.find(fd);
        FD_CLR(fd, &readSet);
        FD_CLR(fd, &writeSet);
        if(it!=mSeqs.end()) mSeqs.erase(it);
        if (fd == maxFD) {
            for (int i = maxFD - 1; i >= 0; --i) {
                if (FD_ISSET(i, &readSet) || FD_ISSET(i, &writeSet)) {
                    maxFD = i;
                    break;
                }
            }
        }
        return 0;
    }

    int modifyFd(int fd,struct epoll_event&e)override{
        auto it = mSeqs.find(fd);
        FD_CLR(fd,&readSet);
        FD_CLR(fd,&writeSet);
        if(it!=mSeqs.end())it->second = e.data.u64;
        if(e.events & Looper::EVENT_INPUT) FD_SET(fd,&readSet);
        if(e.events & Looper::EVENT_OUTPUT) FD_SET(fd,&writeSet);
        if(fd>maxFD) maxFD = fd;
        return 0;
    }

    int waitEvents(std::vector<epoll_event>& activeFDs,uint32_t ms) override {
        struct timeval tv;
        fd_set tmpReadSet = readSet;
        fd_set tmpWriteSet = writeSet;
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        int numEvents = select(maxFD + 1, &tmpReadSet, &tmpWriteSet, nullptr, &tv);
        if (numEvents == -1) {
            //LOGW("Failed to select file descriptors，select's return value seems some error in MSVC");
            return 0;
        }
        activeFDs.clear();
        for (int i = 0; i <= maxFD; ++i) {
            epoll_event e;
            e.data.fd = i;
            auto it =mSeqs.find(i);
            if (FD_ISSET(i, &tmpReadSet)) {
                e.events = Looper::EVENT_INPUT;
                if(it!=mSeqs.end()) e.data.u64=it->second;
                activeFDs.push_back(e);
            }else if (FD_ISSET(i, &tmpWriteSet)) {
                e.events = Looper::EVENT_OUTPUT;
                if(it!=mSeqs.end())e.data.u64=it->second;
                activeFDs.push_back(e);
            }
        }
        return (int)activeFDs.size();
    }
};
#endif

IOEventProcessor::IOEventProcessor() {}
IOEventProcessor::~IOEventProcessor() {}
IOEventProcessor* IOEventProcessor::create(){
#if (defined(_WIN32)||defined(_WIN64)||(defined(__linux__)||defined(__unix__)))&&!defined(USE_SELECT)
    return new EPOLL();
#else
    return new SELECTOR();
#endif    
}

}/*endof namespace*/
