#include <alooper.h>
#include <unistd.h>
#include <string.h>
#include <sys/eventfd.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <limits.h>

namespace alooper{

static constexpr int EPOLL_SIZE_HINT = 8;
// Maximum number of file descriptors for which to retrieve poll events each iteration.
static constexpr int EPOLL_MAX_EVENTS = 16;

void Looper::Looper::Request::initEventItem(struct epoll_event* eventItem) const{
}

Looper::Looper(bool allowNonCallbacks) :
        mAllowNonCallbacks(allowNonCallbacks), mSendingMessage(false),
        mPolling(false), mEpollFd(-1), mEpollRebuildRequired(false),
        mNextRequestSeq(0), mResponseIndex(0), mNextMessageUptime(LONG_MAX) {
    //mWakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    //LOGE_IF(mWakeEventFd < 0, "Could not make wake event fd: %s",strerror(errno));
    //AutoMutex _l(mLock);
    rebuildEpollLocked();
}

void Looper::rebuildEpollLocked() {
    // Close old epoll instance if we have one.
    if (mEpollFd >= 0) {
        LOGV("%p ~ rebuildEpollLocked - rebuilding epoll set", this);
        close(mEpollFd);
    }

    // Allocate the new epoll instance and register the wake pipe.
    mEpollFd = epoll_create(EPOLL_SIZE_HINT);
    LOGE_IF(mEpollFd < 0, "Could not create epoll instance: %s", strerror(errno));

    struct epoll_event eventItem;
    memset(& eventItem, 0, sizeof(epoll_event)); // zero out unused members of data field union
    eventItem.events = EPOLLIN;
    eventItem.data.fd = mWakeEventFd;
    int result = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mWakeEventFd, & eventItem);
    LOGE_IF(result != 0, "Could not add wake event fd to epoll instance: %s",strerror(errno));

    for (auto it=mRequests.begin();it!=mRequests.end(); it++) {
        const Request& request = it->second;
        struct epoll_event eventItem;
        request.initEventItem(&eventItem);

        int epollResult = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, request.fd, & eventItem);
        if (epollResult < 0) {
            LOGE("Error adding epoll events for fd %d while rebuilding epoll set: %s",request.fd, strerror(errno));
        }
    }
}
}
