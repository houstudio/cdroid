//
// TimerFD.h - This file is part of the Grinder library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#ifndef LOOPER_LINUX_TIMERFD_H
#define LOOPER_LINUX_TIMERFD_H

#include <looper/Platform.h>
#ifdef LOOPER_LINUX

#include <looper/FileSource.h>

namespace cdroid
{

class TimerFD : public FileSource
{
public:
    TimerFD(int timeout_ms, bool one_shot=false);
    ~TimerFD();

    void arm(int timeout=-1);
    void disarm();

    bool one_shot() const
    {
        return m_one_shot;
    }
    void set_one_shot(bool one_shot)
    {
        disarm();
        m_one_shot = one_shot;
        arm();
    }

    int timeout() const
    {
        return m_timeout;
    }
    void set_timeout(int timeout_ms)
    {
        disarm();
        m_timeout = timeout_ms;
        arm();
    }

    bool check() override;
    bool dispatch(EventHandler &func) override;

private:
    int m_timeout;
    bool m_one_shot;
};

} // namespace cdroid

#endif // LOOPER_LINUX

#endif // LOOPER_LINUX_TIMERFD_H
