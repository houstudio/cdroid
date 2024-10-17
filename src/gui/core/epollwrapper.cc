#include <map>
#include <stdexcept>
#include <core/looper.h>
#include <core/epollwrapper.h>
namespace cdroid{

#if (defined(_WIN32)||defined(_WIN64)) && defined(USEIOCP_IN_WINDOWS)
class IOCP :public cdroid::IOEventProcessor {
private:
    HANDLE hCompletionPort;
    std::map<int, OVERLAPPED*> fdMap;
private:
    OVERLAPPED* getOverlapped(int fd) {
        auto it = fdMap.find(fd);
        return it != fdMap.end() ? it->second : nullptr;
    }
public:
    IOCP() {
        hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (hCompletionPort == NULL) {
            throw std::runtime_error("CreateIoCompletionPort failed");
        }
    }
    virtual ~IOCP() {
        CloseHandle(hCompletionPort);
    }

    int addFd(int fd, uint32_t events)override {
        if (CreateIoCompletionPort((HANDLE)fd, hCompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
            return -1; // 失败
        }
        OVERLAPPED* overlapped = new OVERLAPPED();
        ZeroMemory(overlapped, sizeof(OVERLAPPED));
        fdMap[fd] = overlapped;
        return 0;
    }
    int removeFd(int fd) override{// EPOLL_CTL_DEL
        auto it = fdMap.find(fd);
        if (it != fdMap.end()) {
            delete it->second;
            fdMap.erase(it);
            return 0;
        }
        return -1; // 成功
    }

    int modifyFd(int fd,uint32_t events) override{
	return -1;
    }
    int waitEvents(std::vector<epoll_event>& events, uint32_t timeout)override {
        OVERLAPPED_ENTRY entries[128];
        ULONG numEntriesReturned;

        BOOL result = GetQueuedCompletionStatusEx(hCompletionPort, entries, 128,
            &numEntriesReturned, timeout, FALSE);

        if (result == FALSE) {
            if (GetLastError() == WAIT_TIMEOUT) {
                return 0;
            }
            return -1; //failed
        }

        for (ULONG i = 0; i < numEntriesReturned; ++i) {
            int fd = entries[i].lpCompletionKey;
            DWORD bytesTransferred = entries[i].dwNumberOfBytesTransferred;

            epoll_event event;
            event.events = (bytesTransferred == 0) ? 0 : 1;
            event.data.fd = fd;
            events.push_back(event);
        }

        return numEntriesReturned;
    }
 };

#elif  (defined(__linux__)||defined(__unix__))
#include <unistd.h>
class EPOLL:public IOEventProcessor {
private:
    int epfd;
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
        if (epfd == -1) {
            throw std::runtime_error("Failed to create epoll file descriptor");
        }
    }

    ~EPOLL() {
        close(epfd);
    }


    int addFd(int fd, uint32_t events)override {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = toEpollEvents(events);
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

    int modifyFd(int fd, uint32_t events) override{
        struct epoll_event event;
        event.data.fd = fd;
        event.events = toEpollEvents(events);
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) == -1) {
            return -1;
        }
        return 0;
    }

    int waitEvents(std::vector<epoll_event>& activeFDs, uint32_t timeout) override{
        struct epoll_event events[maxEvents];
        const int numEvents = epoll_wait(epfd, events, maxEvents,timeout);
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

class SELECTPOR : public IOEventProcessor {
private:
    fd_set readSet;
    fd_set writeSet;
    int maxFD = 0;
public:
    int addFd(int fd, uint32_t events) override {
        if (events & Looper::EVENT_INPUT)  FD_SET(fd, &readSet);
        if (events & Looper::EVENT_OUTPUT) FD_SET(fd, &writeSet);
        if (fd > maxFD) maxFD = fd;
        return 0;
    }

    int removeFd(int fd) override {
        FD_CLR(fd, &readSet);
        FD_CLR(fd, &writeSet);
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

    int modifyFd(int fd,uint32_t events)override{
        FD_CLR(fd,&readSet);
        FD_CLR(fd,&writeSet);
        if(events & Looper::EVENT_INPUT) FD_SET(fd,&readSet);
        if(events & Looper::EVENT_OUTPUT) FD_SET(fd,&writeSet);
        if(fd>maxFD) maxFD = fd;
        return 0;
    }

    int waitEvents(std::vector<epoll_event>& activeFDs,uint32_t ms) override {
        struct timeval tv;
        fd_set tmpReadSet = readSet;
        fd_set tmpWriteSet = writeSet;
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        const int numEvents = select(maxFD + 1, &tmpReadSet, &tmpWriteSet, nullptr, &tv);
        if (numEvents == -1) {
            throw std::runtime_error("Failed to select file descriptors");
        }
        activeFDs.clear();
        for (int i = 0; i <= maxFD; ++i) {
            epoll_event e;
            e.data.fd = i;
            if (FD_ISSET(i, &tmpReadSet)) {
                e.events = Looper::EVENT_INPUT;
                activeFDs.push_back(e);
            }else if (FD_ISSET(i, &tmpWriteSet)) {
                e.events = Looper::EVENT_OUTPUT;
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
#if (defined(_WIN32)||defined(_WIN64))&&defined(USEIOCP_IN_WINDOWS)
    return new IOCP();
#elif defined(__linux__)||defined(__unix__)
    return new EPOLL();
#else
    return new SELECTPOR();
#endif    
}

}/*endof namespace*/
