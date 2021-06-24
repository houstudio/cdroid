//
// FileEvents.h - This file is part of the UI library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#ifndef LOOP_FILEEVENTS_H
#define LOOP_FILEEVENTS_H

#include <poll.h>

namespace cdroid
{

enum class FileEvents {
    NONE     = 0,
    INPUT    = POLLIN,
    OUTPUT   = POLLOUT,
    PRIORITY = POLLPRI,
    ERROR    = POLLERR,
    HANGUP   = POLLHUP,
};

} // namespace cdroid

static inline int operator&(cdroid::FileEvents ev1, cdroid::FileEvents ev2)
{
    return static_cast<int>(ev1) & static_cast<int>(ev2);
}

static inline int operator&(int ev1, cdroid::FileEvents ev2)
{
    return ev1 & static_cast<int>(ev2);
}

static inline cdroid::FileEvents operator|(cdroid::FileEvents ev1, cdroid::FileEvents ev2)
{
    return static_cast<cdroid::FileEvents>(static_cast<int>(ev1) | static_cast<int>(ev2));
}

#endif // LOOP_FILEEVENTS_H

