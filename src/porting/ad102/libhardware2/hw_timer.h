#ifndef __LIBHARDWARE2_HW_TIMER_H__
#define __LIBHARDWARE2_HW_TIMER_H__

#ifdef  __cplusplus
extern "C" {
#endif

int hw_timer_open(const char *dev_name);

int hw_timer_close(int dev_fd);

int hw_timer_start(int dev_fd, unsigned long usecs);

int hw_timer_stop(int dev_fd);

int hw_timer_wait(int dev_fd);

#ifdef  __cplusplus
}
#endif

#endif
