#include <looper/GenericSignalSource.h>
#include <looper/FileSource.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>

using namespace std;

namespace cdroid
{

static const int SIG_MAX = 128;
static const int PIPE_READ = 0;
static const int PIPE_WRITE = 1;
static int pipe_fds[2] = { -1, -1 };

static void signal_handler(int signo)
{
    uint32_t usigno = signo;
    ::write(pipe_fds[PIPE_WRITE], &usigno, sizeof(uint32_t));
}

GenericSignalSource::GenericSignalSource(bool manage_proc_mask)
    : GenericSignalSource(nullptr, manage_proc_mask)
{
}

GenericSignalSource::GenericSignalSource(const sigset_t *sigs, bool manage_proc_mask)
    : SignalSource(sigs, manage_proc_mask)
{
    if (pipe_fds[0] == -1 && pipe_fds[1] == -1) {
        pipe(pipe_fds);
        fd = pipe_fds[PIPE_READ];
    }

    if (manage_proc_mask)
        sigprocmask(SIG_UNBLOCK, sigs, nullptr);
}

GenericSignalSource::~GenericSignalSource()
{
    ::close(pipe_fds[0]);
    ::close(pipe_fds[1]);
    pipe_fds[0] = pipe_fds[1] = -1;

    sigset_t fullset;
    sigfillset(&fullset);
    for (int i = 0; i < SIG_MAX; i++) {
        int res = sigismember(&fullset, i);
        if (res > 0)
            signal(i, SIG_DFL);
    }
}

bool GenericSignalSource::dispatch(EventHandler &func)
{
    uint32_t sig = 0;
    if (::read(fd, &sig, sizeof(uint32_t)) == sizeof(uint32_t)) {
        signo = sig;
        return FileSource::dispatch(func);
    }
    return true;
}

void GenericSignalSource::update_signals(const sigset_t *sigs, bool manage_proc_mask)
{
    for (int i = 0; i < SIG_MAX; i++) {
        int res = sigismember(sigs, i);
        if (res > 0)
            signal(i, signal_handler);
        else if (res == 0)
            signal(i, SIG_DFL);
    }

    if (manage_proc_mask)
        sigprocmask(SIG_UNBLOCK, sigs, nullptr);
}

} // namespace cdroid
