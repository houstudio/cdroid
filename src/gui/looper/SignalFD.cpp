//
// SignalFD.cpp - This file is part of the Grinder library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#include <looper/SignalFD.h>

#ifdef LOOPER_LINUX

#include <unistd.h>
#include <sys/signalfd.h>

namespace cdroid
{

SignalFD::SignalFD(bool manage_proc_mask)
    : SignalFD(nullptr, manage_proc_mask)
{
}

SignalFD::SignalFD(const sigset_t *sigs, bool manage_proc_mask)
    : SignalSource(sigs, manage_proc_mask)
{
    if (sigs && manage_proc_mask)
        sigprocmask(SIG_BLOCK, sigs, nullptr);
}

bool SignalFD::dispatch(EventHandler &func)
{
    signalfd_siginfo info;
    if (read(fd, &info, sizeof(signalfd_siginfo)) == sizeof(signalfd_siginfo)) {
        signo = info.ssi_signo;
        return FileSource::dispatch(func);
    }
    return true;
}

void SignalFD::update_signals(const sigset_t *sigs, bool manage_proc_mask)
{
    fd = signalfd(fd, sigs, SFD_NONBLOCK | SFD_CLOEXEC);
    if (manage_proc_mask) {
        sigset_t allset;
        ::sigfillset(&allset);
        ::sigprocmask(SIG_UNBLOCK, &allset, nullptr);
        ::sigprocmask(SIG_BLOCK, sigs, nullptr);
    }
}

} // namespace cdroid

#endif // LOOPER_LINUX
