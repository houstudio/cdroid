//
// TimeoutSource.cpp - This file is part of the UI library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#include <looper/TimeoutSource.h>
#include <looper/Utility.h>
#include <iostream>

using namespace std;

namespace cdroid
{

TimeoutSource::TimeoutSource(int timeout)
    : m_timeout(timeout),
      m_next_expiry(time_now() + timeout)
{
}

bool TimeoutSource::is_ready(int &max_timeout)
{
    uint64_t now = time_now();
    if (now >= m_next_expiry) {
        m_next_expiry = (now + m_timeout) - (now - m_next_expiry);
        return true;
    } else {
        max_timeout = m_next_expiry - now;
        return false;
    }
}

bool TimeoutSource::prepare(int &max_timeout)
{
    return is_ready(max_timeout);
}

bool TimeoutSource::check()
{
    int unused = 0;
    return is_ready(unused);
}

} // namespace cdroid
