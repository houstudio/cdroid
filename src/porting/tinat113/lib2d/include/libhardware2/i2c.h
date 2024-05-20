/*
 *  Copyright (C) 2020 Ingenic Semiconductor Co., Ltd.
 *
 *  Ingenic library hardware version2
 *
 */
#ifndef __HARDWARE2_I2C_H__
#define __HARDWARE2_I2C_H__

#ifdef  __cplusplus
extern "C" {
#endif


/**
 * @brief   打开i2c设备
 * @param   bus_num       i2c总线号
 * @return  >0 :          成功返回i2c文件描述符i2c_fd，
 *          <0 :          失败返回错误值(负数)
 */
int i2c_open(int bus_num);


/**
 * @brief   关闭i2c设备
 * @param   i2c_fd        i2c设备文件描述符
 * @return  空
 */
void i2c_close(int i2c_fd);

/**
 * @brief   探测i2c设备
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @return  =1            设备地址忙
 *          =0            设备地址有响应
 *          <0            设备地址无响应
 */
int i2c_detect(int i2c_fd, uint16_t device_addr);

/**
 * @brief   i2c读
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @param   buffer        读buffer
 * @param   size          buffer大小
 * @return  =0            读取成功
 *          <0            读取失败
 */
int i2c_read(int i2c_fd, uint16_t device_addr, void *buffer, int size);

/**
 * @brief   i2c写
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @param   buffer        写buffer
 * @param   size          buffer大小
 * @return  =0            写入成功
 *          <0            写入失败
 */
int i2c_write(int i2c_fd, uint16_t device_addr, void *buffer, int size);

/**
 * @brief   i2c读寄存器(寄存器8bit)
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @param   reg_addr      寄存器地址(8bit)
 * @param   buffer        读buffer
 * @param   size          buffer大小
 * @return  =0            读取成功
 *          <0            读取失败
 */
int i2c_read_reg(int i2c_fd, uint16_t device_addr, uint8_t reg_addr,void *buffer, int size);

/**
 * @brief   i2c写寄存器(寄存器8bit)
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @param   reg_addr      寄存器地址(8bit)
 * @param   buffer        写buffer
 * @param   size          buffer大小
 * @return  =0            写入成功
 *          <0            写入失败
 */
int i2c_write_reg(int i2c_fd, uint16_t device_addr, uint8_t reg_addr,void *buffer, int size);

/**
 * @brief   i2c读寄存器(寄存器16bit)
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @param   reg_addr      寄存器地址(16bit)
 * @param   buffer        读buffer
 * @param   size          buffer大小
 * @return  =0            读取成功
 *          <0            读取失败
 */
int i2c_read_reg_16(int i2c_fd, uint16_t device_addr, uint16_t reg_addr,void *buffer, int size);

/**
 * @brief   i2c写寄存器(寄存器16bit)
 * @param   i2c_fd        i2c设备文件描述符
 * @param   device_addr   i2c设备地址
 * @param   reg_addr      寄存器地址(16bit)
 * @param   buffer        写buffer
 * @param   size          buffer大小
 * @return  =0            写入成功
 *          <0            写入失败
 */
int i2c_write_reg_16(int i2c_fd, uint16_t device_addr, uint16_t reg_addr,void *buffer, int size);

#ifdef  __cplusplus
}
#endif

#endif //__HARDWARE_I2C_H__
