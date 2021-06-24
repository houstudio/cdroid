//
// FileSource.h - This file is part of the Grinder library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#ifndef LOOP_FILESOURCE_H
#define LOOP_FILESOURCE_H

#include <looper/EventSource.h>
#include <looper/FileEvents.h>
#include <string>

namespace cdroid
{

class FileSource : public EventSource
{
public:
    int fd, revents;
    FileEvents events;
    FileSource(int fd, FileEvents events=FileEvents::NONE)
        : fd(fd), revents(0), events(events) {}

    bool prepare(int&) override
    {
        return false;
    }

    bool check() override
    {
        if (events == FileEvents::NONE || revents == 0)
            return false;
        if (((revents & FileEvents::INPUT) && (events & FileEvents::INPUT)) ||
            ((revents & FileEvents::OUTPUT) && (events & FileEvents::OUTPUT)) ||
            ((revents & FileEvents::PRIORITY) && (events & FileEvents::PRIORITY)) ||
            ((revents & FileEvents::ERROR) && (events & FileEvents::ERROR)) ||
            ((revents & FileEvents::HANGUP) && (events &  FileEvents::HANGUP))) {
            return true;
        }
        return false;
    }

    bool dispatch(EventHandler &func)
    {
        return func(*this);
    }

    bool is_file_source() const override final
    {
        return true;
    }
};

} // namespace cdroid

#endif // LOOPER_FILESOURCE_H
