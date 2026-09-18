#ifndef __DESKCLOCK_SNACKBARMANAGER_H__
#define __DESKCLOCK_SNACKBARMANAGER_H__
/*********************************************************************************
 * Port of com.android.deskclock.widget.toast.SnackbarManager — a no-op stub.
 * CDROID has no Snackbar/Material toast surface; show()/dismiss() drop their
 * payload (only the silent-alarm-warning snackbar uses it upstream).
 *********************************************************************************/
namespace cdroid {
namespace deskclock {

class SnackbarManager {
public:
    static void show(void* /*snackbar*/) {}
    static void dismiss() {}
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SNACKBARMANAGER_H__
