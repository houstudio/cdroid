/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
