//
// Looper.cpp - This file is part of the Looper library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#include <looper/EventLoop.h>
#include <looper/EventSource.h>
#include <looper/FileSource.h>
#include <looper/IdleSource.h>
#include <looper/TimeoutSource.h>
#include <algorithm>
#include <unordered_set>
#include <poll.h>
#include <sys/epoll.h>
#include <iostream>
#include <cdtypes.h>
#include <unistd.h>

using namespace std;

namespace cdroid {

struct Looper::Private {
    bool quit;
    int exit_code;
    int epoolfd;
    uint64_t time_last;
    vector<EventSource*> sources;
    vector<EventSource*> remove_sources;
    vector<FileSource*> pollable_sources;
    vector<struct pollfd> poll_fds;
    unordered_set<EventSource*> source_set;
    vector<IdleSource*> idle_sources;
    vector<TimeoutSource*> timeout_sources;

    Private() : quit(false), exit_code(0) {
        epoolfd=epoll_create(128);
    }
    void add_event_source(EventSource*source) {
        sources.emplace_back(source);
        source_set.emplace(source);
        if (source->is_idle_source())
            idle_sources.emplace_back(dynamic_cast<IdleSource*>(source));
        else if (source->is_timeout_source())
            timeout_sources.emplace_back(dynamic_cast<TimeoutSource*>(source));
        else if (source->is_file_source())
            pollable_sources.emplace_back(dynamic_cast<FileSource*>(source));
    }
    void remove_event_source(EventSource*source) {
        if (source->is_idle_source()) {
            idle_sources.erase(remove(begin(idle_sources),end(idle_sources), source), end(idle_sources));
        } else if (source->is_timeout_source()) {
            timeout_sources.erase(remove(begin(timeout_sources),end(timeout_sources), source),end(timeout_sources));
        } else if (source->is_file_source()) {
            pollable_sources.erase(remove(begin(pollable_sources),end(pollable_sources), source),end(pollable_sources));
        }
        sources.erase(remove(begin(sources),end(sources), source), end(sources));
        source_set.erase(source);
    }
    ~Private() {
        for (auto source : sources)
            delete source;
        close(epoolfd);
    }
};

Looper*Looper::mInst=nullptr;
Looper*Looper::getDefault(){
    if(mInst==nullptr)
       mInst=new Looper();
    return mInst;
}

Looper::Looper()
    : impl(make_shared<Private>()) {
}

void Looper::iteration() {
    int max_timeout =10;//-1;
    int n_ready = 0;
    // prepare and check for already ready event sources
    for (auto source : impl->sources) {
        if (source->is_idle_source())
            continue;

        int source_timeout = -1;

        if (source->prepare(source_timeout)) {
            source->loop_data.ready = true;
            n_ready++;
        } else if (source_timeout > 0 && (max_timeout == -1 || source_timeout < max_timeout)) {
            max_timeout = source_timeout;
        }
    }

    // poll all of the pollable event sources
    impl->poll_fds.resize(impl->pollable_sources.size());
    for (size_t i = 0; i < impl->pollable_sources.size(); i++) {
        if (! impl->pollable_sources[i]->loop_data.ready && impl->pollable_sources[i]->fd >= 0) {
            impl->poll_fds[i].fd = impl->pollable_sources[i]->fd;
            impl->poll_fds[i].events = static_cast<int>(impl->pollable_sources[i]->events);
            impl->poll_fds[i].revents = impl->pollable_sources[i]->revents = 0;
        }
    }

    // if there's nothing else ready and there are idle event sources, make
    // poll() return immediately.
    if (n_ready < 1 && impl->idle_sources.size() > 0)
        max_timeout = 0;

    if (::poll(impl->poll_fds.data(), impl->poll_fds.size(), max_timeout) >0) {
        for (size_t i = 0; i < impl->pollable_sources.size(); i++)
            impl->pollable_sources[i]->revents = impl->poll_fds[i].revents;
    }

    // now check if any more sources are ready after polling
    for (auto source : impl->sources) {
        if (source->is_idle_source())
            continue;
        if (! source->loop_data.ready) {
            if (source->check()) {
                source->loop_data.ready = true;
                n_ready++;
            }
        }
    }

    impl->remove_sources.clear();
    if (n_ready > 0) {
        // dispatch all ready event sources
        for (int i=0; i<impl->sources.size(); i++) {
            EventSource *source=impl->sources[i];
            if (! source->is_idle_source() && source->loop_data.ready &&
                    source->loop_data.handler) {
                if (! source->dispatch(source->loop_data.handler))
                    impl->remove_sources.emplace_back(source);
                source->loop_data.ready = false;
            }
        }
    } else {
        // nothing else ready, dispatch idle event sources
        for (auto source : impl->idle_sources) {
            if (source->loop_data.handler &&
                    ! source->dispatch(source->loop_data.handler)) {
                impl->remove_sources.emplace_back(source);
            }
        }
    }

    // prune event sources which requested to be removed
    for (auto source : impl->remove_sources)
        remove_event_source(source);
}

bool Looper::contains_source(EventSource *source) const {
    return (impl->source_set.count(source) > 0);
}

int Looper::run() {
    impl->quit = false;
    impl->exit_code = 0;
    while (! impl->quit)
        iteration();
    return impl->exit_code;
}

void Looper::quit(int exit_code) {
    impl->exit_code = exit_code;
    impl->quit = true;
}

bool Looper::add_event_source(EventSource *source, EventHandler handler) {
    if (contains_source(source))
        return false;

    source->loop_data.ready = false;
    source->loop_data.handler = handler ? move(handler) : nullptr;

    impl->add_event_source(source);

    return true;
}

bool Looper::set_source_handler(EventSource *source, EventHandler handler) {
    if (contains_source(source))
        source->loop_data.handler = handler;
    return false;
}

bool Looper::remove_event_source(EventSource *source) {
    if (! contains_source(source))
        return false;
    impl->remove_event_source(source);
    delete source;
    return true;
}

EventSource *Looper::add_file(int fd, FileEvents events, EventHandler handler) {
    auto source = new FileSource(fd, events);
    if (! add_event_source(source, handler)) {
        delete source;
        return nullptr;
    }
    return source;
}

EventSource *Looper::add_idle(EventHandler handler) {
    auto source = new IdleSource;
    if (! add_event_source(source, handler)) {
        delete source;
        return nullptr;
    }
    return source;
}

EventSource *Looper::add_timeout(int timeout_ms, EventHandler handler) {
    auto source = new TimeoutSource(timeout_ms);
    if (! add_event_source(source, handler)) {
        delete source;
        return nullptr;
    }
    return source;
}

} // namespace Looper
