//
// SignalFD.h - This file is part of the Grinder library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#ifndef LOOPER_LINUX_SIGNALFD_H
#define LOOPER_LINUX_SIGNALFD_H

#include <looper/Platform.h>
#ifdef LOOPER_LINUX

#include <looper/SignalSource.h>

namespace cdroid
{

class SignalFD : public SignalSource
{
public:
    SignalFD(bool manage_proc_mask=false);
    SignalFD(const sigset_t *sigs, bool manage_proc_mask=false);

    bool dispatch(EventHandler &func) override;

protected:
    void update_signals(const sigset_t *sigs, bool manage_proc_mask);
};

} // namespace cdroid

#endif // LOOPER_LINUX

#endif // LOOPER_LINUX_SIGNALFD_H
