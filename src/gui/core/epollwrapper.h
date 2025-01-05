#ifndef __IOEVENT_PROCESSER_H__
#define __IOEVENT_PROCESSER_H__

#include <vector>

#if defined(__linux__)||defined(__unix__)
#include <sys/epoll.h>
#elif (defined(_WIN32)||defined(_WIN64))||defined(_MSVC_VER)
//Fast epoll for windows:
//https://github.com/piscisaureus/wepoll
#include <core/wepoll.h>
#endif

namespace cdroid{
class IOEventProcessor{
protected:
    IOEventProcessor();
public:
    virtual ~IOEventProcessor();
    static IOEventProcessor* create();
    virtual int addFd(int socket,struct epoll_event&) = 0;
    virtual int removeFd(int fd) = 0;
    virtual int modifyFd(int fd,struct epoll_event&)=0;
    virtual int waitEvents(std::vector<epoll_event>& events, uint32_t timeout) = 0;
};
}/*endof namespace cdroid*/

#endif/*__IOEVENT_PROCESSER_H__*/
