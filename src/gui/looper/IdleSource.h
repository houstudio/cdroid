//
// IdleSource.h - This file is part of the UI library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#ifndef LOOPIDLESOURCE_H
#define LOOPIDLESOURCE_H

#include <looper/EventSource.h>

namespace cdroid
{

class IdleSource : public EventSource
{
public:
    bool prepare(int&) override final
    {
        return true;
    }
    bool check() override final
    {
        return true;
    }
    bool dispatch(EventHandler &func) override
    {
        return func(*this);
    }
    bool is_idle_source() const override final
    {
        return true;
    }
};

} // namespace cdroid

#endif // LOOPER_IDLESOURCE_H
