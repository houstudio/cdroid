#ifndef _LIBHARDWARE2_H_
#define _LIBHARDWARE2_H_

#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

int mcu_open(void);

int mcu_close(int mcu_fd);

int mcu_shutdown(int mcu_fd);

int mcu_reset(int mcu_fd);

int mcu_bootup(int mcu_fd);

int mcu_write_mem(int mcu_fd, unsigned int offset, void *src, int len);

int mcu_write_firmware2(int mcu_fd, FILE *firmware_file);

int mcu_write_firmware(int mcu_fd, const char *firmware_path);

int mcu_write_data(int mcu_fd, void *src, int len);

int mcu_read_data(int mcu_fd, void *src, int len);

int mcu_read_str(int mcu_fd, void *src, int len);

int mcu_read_str_timeout(int mcu_fd, void *src, int len, unsigned long us);

int mcu_write_data_timeout(int mcu_fd, void *src, int len, unsigned long us);

int mcu_read_data_timeout(int mcu_fd, void *dst, int len, unsigned long us);

#ifdef  __cplusplus
}
#endif

#endif /* _LIBHARDWARE2_H_ */
