#ifndef LOOP_GENERICSIGNALSOURCE_H
#define LOOP_GENERICSIGNALSOURCE_H

#include <looper/SignalSource.h>

namespace cdroid
{

class GenericSignalSource : public SignalSource
{
public:
    GenericSignalSource(bool manage_proc_mask=false);
    GenericSignalSource(const sigset_t *sigs=nullptr, bool manage_proc_mask=false);
    ~GenericSignalSource();
    bool dispatch(EventHandler &func) override;

protected:
    void update_signals(const sigset_t *sigs, bool manage_proc_mask);
};

} // namespace cdroid

#endif // LOOPER_GENERICSIGNALSOURCE_H
