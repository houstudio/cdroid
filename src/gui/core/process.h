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
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

namespace cdroid{
class Looper;
class Process {
private:
    pid_t pid_;
    int stdinPipe_[2];
    int stdoutPipe_[2];
    int stderrPipe_[2];
    std::atomic<int> state_;
    std::atomic<int> exitCode_;
    std::thread monitorThread_;
    std::mutex outputMutex_;
    Looper*mLooper;
    std::string stdoutBuffer_;
    std::string stderrBuffer_;
    std::function<void(Process&)> onFinishListener;
    std::function<void(Process&)> onErrorListener;
private:
    void monitorProcess();
    static int pipeFdCallback(int fd, int events, void* data);
public:
    enum State {
        NotRunning,
        Starting,
        Running
    };

    Process();
    Process(Looper*looper);
    ~Process();
    void exec(const std::string& program,const std::vector<std::string>&arguments);
    void exec(const std::string& program,const std::vector<std::string>&arguments,const std::function<void(Process&)>onFinished);
    void exec(const std::string& program,const std::vector<std::string>&arguments,
            const std::function<void(Process&)>onFinished,const std::function<void(Process&)>onError);
    int  wait()const;
    void write(const std::string& data);
    std::string readAllStandardOutput();
    std::string readAllStandardError();
    void terminate();
    void kill();
    int exitCode()const;
    State state()const;
};
}/*endof namespace*/
#endif /*__PROCESS_H__*/
