#ifndef __IOEVENT_PROCESSER_H__
#define __IOEVENT_PROCESSER_H__

#include <vector>

#if defined(__linux__)||defined(__unix__)
#include <sys/epoll.h>
#elif (defined(_WIN32)||defined(_WIN64))||defined(_MSVC_VER)
#include <Windows.h>
typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events;    /* Epoll events */
    epoll_data_t data;  /* User data variable */
};
#define EPOLLIN  1
#define EPOLLOUT 4
#define EPOLLERR 8
#define EPOLLHUP 0x10

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_MOD 2
#define EPOLL_CTL_DEL 3
#endif

namespace cdroid{
class IOEventProcessor{
protected:
    IOEventProcessor();
public:
    virtual ~IOEventProcessor();
    static IOEventProcessor* create();
    virtual int addFd(int socket, uint32_t events) = 0;
    virtual int removeFd(int fd) = 0;
    virtual int waitEvents(std::vector<epoll_event>& events, uint32_t timeout) = 0;
};
}/*endof namespace cdroid*/

#endif/*__IOEVENT_PROCESSER_H__*/
