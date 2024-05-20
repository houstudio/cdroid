#ifndef _LIBHARDWARE2_GPIO_PS2_H_
#define _LIBHARDWARE2_GPIO_PS2_H_

#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

int ps2_read_raw_byte(int fd, unsigned char *data);

int ps2_read_raw_byte_timeout(int fd, unsigned char *buf, int timeout_ms);

int ps2_write_raw_byte(int fd, unsigned char byte);

int ps2_write_str(int fd, void *buf, int size);

int ps2_open(const char *dev_name);

void ps2_close(int fd);

#ifdef  __cplusplus
}
#endif

#endif /* _LIBHARDWARE2_GPIO_PS2_H_ */