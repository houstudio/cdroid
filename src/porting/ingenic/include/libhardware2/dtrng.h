#ifndef __LIBHARDWARE2_DTRNG_H__
#define __LIBHARDWARE2_DTRNG_H__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 获取 DTRNG 生成的随机数
 * @param value 获取随机数的值
 * @return 返回0表示成功，返回负数表示失败
 */
int dtrng_get_random_number(unsigned int *value);

#ifdef  __cplusplus
}
#endif

#endif