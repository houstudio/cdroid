//
// TimerFD.cpp - This file is part of the Grinder library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#include <looper/TimerFD.h>

#ifdef LOOPER_LINUX

#include <cstdint>
#include <iostream>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace std;

namespace cdroid
{

TimerFD::TimerFD(int timeout_ms, bool one_shot)
    : FileSource(::timerfd_create(CLOCK_MONOTONIC,
                                  TFD_NONBLOCK | TFD_CLOEXEC),
                 FileEvents::INPUT),
      m_timeout(timeout_ms),
      m_one_shot(one_shot)
{
    arm();
}

TimerFD::~TimerFD()
{
    if (fd >= 0)
        ::close(fd);
}

static inline void ms_to_timespec(int ms, struct timespec *ts)
{
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

void TimerFD::arm(int timeout)
{
    if (timeout >= 0)
        m_timeout = timeout;
    itimerspec its {{0,0},{0,0}};
    ms_to_timespec(m_timeout, &its.it_value);
    if (! m_one_shot) {
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
    }
    ::timerfd_settime(fd, 0, &its, nullptr);
}

void TimerFD::disarm()
{
    itimerspec its {{0,0},{0,0}};
    ::timerfd_settime(fd, 0, &its, nullptr);
}

bool TimerFD::check()
{
    return (revents & FileEvents::INPUT);
}

bool TimerFD::dispatch(EventHandler &func)
{
    uint64_t count = 0;
    if (::read(fd, &count, sizeof(uint64_t)) == sizeof(uint64_t))
        return FileSource::dispatch(func);
    return true;
}

} // namespace cdroid

#endif // LOOPER_LINUX
