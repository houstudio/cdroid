#include <core/process.h>
#include <core/looper.h>
namespace cdroid{
Process::Process()
    : pid_(-1), state_(NotRunning), exitCode_(-1) {
    stdinPipe_[0] = stdinPipe_[1] = -1;
    stdoutPipe_[0] = stdoutPipe_[1] = -1;
    stderrPipe_[0] = stderrPipe_[1] = -1;
    mLooper = nullptr;
}

Process::Process(Looper*looper)
    :Process(){
    mLooper=looper;
}

Process::~Process() {
    if (state_ == Running) {
        kill();
    }
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }
}

void Process::setListener(const std::function<void(Process&)>onFinished){
    setListener(onFinished,nullptr);
}

void Process::setListener(const std::function<void(Process&)>onFinished,const std::function<void(Process&)>onError){
    this->onFinishListener = onFinished;
    this->onErrorListener = onError;
}

void Process::start(const std::string& program, const std::vector<std::string>& arguments) {
    if (state_ != NotRunning) {
        throw std::runtime_error("Process is already running");
    }

    // Create pipes
    if (pipe(stdinPipe_) == -1 || pipe(stdoutPipe_) == -1 || pipe(stderrPipe_) == -1) {
        throw std::runtime_error("Failed to create pipes");
    }

    pid_ = fork();
    if (pid_ == -1) {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid_ == 0) {
        // Child process
        dup2(stdinPipe_[0], STDIN_FILENO);
        dup2(stdoutPipe_[1], STDOUT_FILENO);
        dup2(stderrPipe_[1], STDERR_FILENO);

        close(stdinPipe_[0]);
        close(stdinPipe_[1]);
        close(stdoutPipe_[0]);
        close(stdoutPipe_[1]);
        close(stderrPipe_[0]);
        close(stderrPipe_[1]);

        // Convert arguments to char* array
        std::vector<char*> args;
        args.push_back(const_cast<char*>(program.c_str()));
        for (const auto& arg : arguments) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);

        execvp(program.c_str(), args.data());
        _exit(127); // If execvp fails
    } else {
        // Parent process
        close(stdinPipe_[0]);
        close(stdoutPipe_[1]);
        close(stderrPipe_[1]);

        state_ = Running;

        if(mLooper==nullptr){
            // Start monitoring thread
            monitorThread_ = std::thread(&Process::monitorProcess, this);
        }else{
            const int flags = fcntl(stdoutPipe_[0], F_GETFL, 0);
            fcntl(stdoutPipe_[0], F_SETFL, flags | O_NONBLOCK);
            fcntl(stderrPipe_[0], F_SETFL, flags | O_NONBLOCK);
            mLooper->addFd(stdoutPipe_[0],0,Looper::EVENT_INPUT,pipeFdCallback,this);
            mLooper->addFd(stderrPipe_[0],0,Looper::EVENT_INPUT,pipeFdCallback,this);
        }
    }
}

void Process::write(const std::string& data) {
    if (state_ != Running) {
        throw std::runtime_error("Process is not running");
    }
    auto ret=::write(stdinPipe_[1], data.c_str(), data.size());
    LOGE_IF(ret<0,"write Error");
}

std::string Process::readAllStandardOutput() {
    std::lock_guard<std::mutex> lock(outputMutex_);
    std::string result = stdoutBuffer_;
    stdoutBuffer_.clear();
    return result;
}

std::string Process::readAllStandardError() {
    std::lock_guard<std::mutex> lock(outputMutex_);
    std::string result = stderrBuffer_;
    stderrBuffer_.clear();
    return result;
}

void Process::terminate() {
    if (state_ == Running) {
        ::kill(pid_, SIGTERM);
    }
}

void Process::kill() {
    if (state_ == Running) {
        ::kill(pid_, SIGKILL);
    }
}

int Process::exitCode() const {
    return exitCode_;
}

Process::State Process::state() const {
    return static_cast<State>(state_.load());
}

int Process::pipeFdCallback(int fd, int events, void* data){
    Process*thiz =(Process*)data;
    int status,bytesRead;
    char buffer[128];
    if((fd==thiz->stdoutPipe_[0])&&(events&Looper::EVENT_INPUT)){
        do{
            bytesRead =read(fd, buffer, sizeof(buffer));
            if(bytesRead>0)thiz->stdoutBuffer_.append(buffer, bytesRead);
        }while(bytesRead>0);
    }else if((fd==thiz->stderrPipe_[0])&&(events&Looper::EVENT_INPUT)){
        do{
            bytesRead = read(fd, buffer, sizeof(buffer));
            if(bytesRead>0)thiz->stderrBuffer_.append(buffer, bytesRead);
        }while(bytesRead>0);
    }
    pid_t result = waitpid(thiz->pid_, &status, WNOHANG);
    if (result == thiz->pid_) {
        if (WIFEXITED(status)) {
            thiz->exitCode_ = WEXITSTATUS(status);
        } else {
            thiz->exitCode_ = -1;
        }
        thiz->state_ = NotRunning;
        if (thiz->onFinishListener) {
            thiz->onFinishListener(*thiz);
        }
    }else if(result==-1){
        /*if(errno==ECHILD){ LOGE("No child process."); }
        else if(errno==EINTR){LOGE("waitpid interrupted by signal, retrying...");}*/
        thiz->exitCode_ = -1;
        if(thiz->onErrorListener){
            thiz->onErrorListener(*thiz);
        }
    }
    /*(result==0)child process is running DO NOTHING*/
    return 0;
}

void Process::monitorProcess() {
    struct pollfd fds[2];
    fds[0].fd = stdoutPipe_[0];
    fds[0].events = POLLIN;
    fds[1].fd = stderrPipe_[0];
    fds[1].events = POLLIN;

    char buffer[256];
    while (state_ == Running) {
        int ret = poll(fds, 2, 100); // 100ms timeout
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                int bytesRead = read(stdoutPipe_[0], buffer, sizeof(buffer));
                if (bytesRead > 0) {
                    std::lock_guard<std::mutex> lock(outputMutex_);
                    stdoutBuffer_.append(buffer, bytesRead);
                }
            }
            if (fds[1].revents & POLLIN) {
                int bytesRead = read(stderrPipe_[0], buffer, sizeof(buffer));
                if (bytesRead > 0) {
                    std::lock_guard<std::mutex> lock(outputMutex_);
                    stderrBuffer_.append(buffer, bytesRead);
                }
            }
        }

        // Check if the process has exited
        int status;
        pid_t result = waitpid(pid_, &status, WNOHANG);
        if (result == pid_) {
            if (WIFEXITED(status)) {
                exitCode_ = WEXITSTATUS(status);
            } else {
                exitCode_ = -1;
            }
            state_ = NotRunning;
            if (onFinishListener) {
                onFinishListener(*this);
            }
            break;
        }
    }
}
}/*endof namespace*/
