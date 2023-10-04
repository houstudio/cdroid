#ifndef __LIBHARDWARE2_DBOX_H__
#define __LIBHARDWARE2_DBOX_H__

typedef enum {
    DBOX_COLOR_RED,                     /* red */
    DBOX_COLOR_BLACK,                   /* black */
    DBOX_COLOR_GREEN,                   /* green */
    DBOX_COLOR_YELLOW,                  /* yellow */
}dbox_color_t;

typedef enum {
    DBOX_MODE_RECT,                     /* draw rectangle window*/
    DBOX_MODE_RANGE,                    /* draw the corner window*/
    DBOX_MODE_HORI,                     /* draw horizontal line*/
    DBOX_MODE_VERT,                     /* draw vertical line*/
}dbox_mode_t;

typedef struct {
    unsigned int start_point_x;
    unsigned int start_point_y;
    unsigned int box_width;
    unsigned int box_height;
    unsigned int line_width;
    unsigned int line_lenght;           /* only used in DBOX_MODE_RANGE */
    unsigned int box_mode;
    dbox_color_t color_mode;
}dbox_ram_t;

typedef struct {
    unsigned int box_pbuf;
    unsigned int img_w;
    unsigned int img_h;
    unsigned int boxs_num;
    unsigned int is_rgba;
    dbox_ram_t ram_para[24];
}dbox_param_t;

/**
 * @brief 获得dbox设备句柄
 * @return 成功返回设备句柄,失败返回-1
 */
int dbox_open(void);

/**
 * @brief 关闭dbox设备句柄
 * @param fd dbox设备句柄,由dbox_open()函数获得
 * @return 成功返回0,失败返回-1
 */
int dbox_close(int fd);

/**
 * @brief 启动dbox绘制
 * @param dbox_fd dbox设备句柄,由dbox_open()函数获得
 * @param dbox_para dbox的信息指针, 指向用户设置的绘制信息
 * @return 成功返回0,失败返回非0
 */
int dbox_start_draw(const int fd, const dbox_param_t *dbox_param);

#endif /* __LIBHARDWARE2_DBOX_H__ */