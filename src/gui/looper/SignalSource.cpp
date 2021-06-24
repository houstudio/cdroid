#include <looper/SignalSource.h>
#include <looper/Platform.h>
#ifdef LOOPER_LINUX
# include <looper/SignalFD.h>
#else
# include <looper/GenericSignalSource.h>
#endif
#include <unistd.h>

namespace cdroid
{

SignalSource::SignalSource(bool manage_proc_mask)
    : FileSource(-1, FileEvents::INPUT),
      m_manage_proc_mask(manage_proc_mask)
{
    sigemptyset(&m_sigs);
    if (m_manage_proc_mask)
        sigprocmask(SIG_BLOCK, nullptr, &m_old_sigs);
}

SignalSource::SignalSource(const sigset_t *sigs, bool manage_proc_mask)
    : FileSource(-1, FileEvents::INPUT),
      m_manage_proc_mask(manage_proc_mask)
{
    if (sigs)
        m_sigs = *sigs;
    else
        sigemptyset(&m_sigs);
    if (m_manage_proc_mask)
        sigprocmask(SIG_BLOCK, nullptr, &m_old_sigs);
}

SignalSource::~SignalSource()
{
    try {
        if (m_manage_proc_mask) {
            sigset_t ss;
            sigfillset(&ss);
            sigprocmask(SIG_UNBLOCK, &ss, nullptr);
            sigprocmask(SIG_BLOCK, &m_old_sigs, nullptr);
        }
        close(fd);
    } catch (...) {
        /* pass */
    }
}

void SignalSource::add(int signo)
{
    sigaddset(&m_sigs, signo);
    update();
}

void SignalSource::remove(int signo)
{
    sigdelset(&m_sigs, signo);
    update();
}

void SignalSource::update()
{
    update_signals(&m_sigs, m_manage_proc_mask);
}

} // namespace cdroid
