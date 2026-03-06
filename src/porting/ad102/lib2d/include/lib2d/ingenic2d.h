#ifndef __INGENIC2D_H__
#define __INGENIC2D_H__

/*2d 操作句柄*/
struct ingenic_2d;


/*2d格式，暂时不支持planer的格式，目标格式不支持yuv格式*/
enum ingenic_2d_format {
    INGENIC_2D_ARGB8888,
    INGENIC_2D_RGB565,
    INGENIC_2D_XRGB8888,
    INGENIC_2D_YUYV422,
    INGENIC_2D_UYVY422,
};

/*旋转角度枚举*/
enum ingenic_2d_rotate_angle {
    ROTATE_0,
    ROTATE_90,
    ROTATE_180,
    ROTATE_270,
};

/*2d数据源以及目标数据的描述帧*/
struct ingenic_2d_frame {
    int width;                                      /*用户申请的宽*/
    int height;                                     /*用户申请的高*/
    int align_width;                                /*2d模块实际操作的宽，目前测到的分辨率暂时都不需要对齐，暂时先保留*/
    int align_height;                               /*2d模块实际操作的高，目前测到的分辨率暂时都不需要对齐，暂时先保留*/
    int stride;                                     /*一行大小（字节）*/
    int frame_size;                                 /*一帧大小(字节)*/
    void *addr[3];                                  /*帧数据虚拟地址，暂时不支持plane的格式，所以暂时只需使用[0]地址*/
    unsigned int phyaddr[3];                        /*帧数据物理地址，暂时不支持plane的格式，所以暂时只需使用[0]地址*/
    enum ingenic_2d_format format;                  /*帧格式*/
};

/*功能性api的操作矩形，包含了实际操作的有效区域以及帧描述*/
struct ingenic_2d_rect {
    int x;                                          /*有效区域的水平起始位置*/
    int y;                                          /*有效区域的垂直起始位置*/
    int w;                                          /*有效区域的宽度*/
    int h;                                          /*有效区域的高度*/
    struct ingenic_2d_frame *frame;                 /*数据描述帧*/
};

/*描述一线的结构体，两点描述一线*/
struct ingenic_2d_line {
    int x0,y0;                                      /*第一个点的坐标*/
    int x1,y1;                                      /*第二个点的坐标*/
};

/**
 * @brief 打开2d设备句柄
 * @param NULL
 * @return 成功返回2d句柄,失败返回NULL
 */
struct ingenic_2d *ingenic_2d_open(void);


/**
 * @brief 关闭 2d设备句柄
 * @param ingenic_2d 2d设备句柄
 * @return 无
 */
void ingenic_2d_close(struct ingenic_2d *ingenic_2d);


/**
 * @brief 初始化并获取2d操作矩形
 * @param frame 2d帧描述
 * @param x  有效区域的水平起始位置
 * @param y  有效区域的垂直起始位置
 * @param w  有效区域的宽
 * @param h  有效区域的高
 * @return 成功返回操作矩形，失败返回NULL
 */
struct ingenic_2d_rect ingenic_2d_rect_init(struct ingenic_2d_frame *frame,
                                             int x, int y, int w, int h);



/**
 * @brief 申请并获取2d帧描述(图像数据存放的内存由内部申请)
 * @param ingenic_2d 2d设备句柄
 * @param width      帧的宽度
 * @param height     帧的高度
 * @param format     帧的格式
 * @return 成功返回帧描述，失败返回NULL
 */
struct ingenic_2d_frame *ingenic_2d_alloc_frame(struct ingenic_2d *ingenic_2d,
                                                int width, int height,
                                                enum ingenic_2d_format format);


/**
 * @brief 申请并获取2d帧描述(图像数据存放的内存由外部导入)
 * @param ingenic_2d 2d设备句柄
 * @param width      帧的宽度
 * @param height     帧的高度
 * @param format     帧的格式
 * @param phyaddr    内存的物理地址
 * @param addr       内存的虚拟地址
 * @param addrsize   内存的大小
 * @return 成功返回帧描述，失败返回NULL
 */
struct ingenic_2d_frame *ingenic_2d_alloc_frame_by_user(struct ingenic_2d *ingenic_2d,
                                                        int width, int height,enum ingenic_2d_format format,
                                                        unsigned long phyaddr, void *addr, int addrsize);


/**
 * @brief 释放2d帧描述 （若是外部导入的物理内存，只会释放2d帧结构体）
 * @param ingenic_2d 2d设备句柄
 * @param width      2d帧描述
 * @return 无
 */
void ingenic_2d_free_frame(struct ingenic_2d *ingenic_2d, struct ingenic_2d_frame *frame);


/**
 * @brief 旋转图像
 * @param ingenic_2d 2d设备句柄
 * @param angle      旋转角度
 * @param src        源的操作矩形
 * @param dst        目标的操作矩形
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_rotate(struct ingenic_2d *ingenic_2d,
                     enum ingenic_2d_rotate_angle angle,
                     struct ingenic_2d_rect *src,
                     struct ingenic_2d_rect *dst);


/**
 * @brief 缩放图像
 * @param ingenic_2d 2d设备句柄
 * @param src        源的操作矩形
 * @param dst        目标的操作矩形
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_scale(struct ingenic_2d *ingenic_2d,
                     struct ingenic_2d_rect *src,
                     struct ingenic_2d_rect *dst);


/**
 * @brief 矩形填充
 * @param ingenic_2d 2d设备句柄
 * @param dst        目标的操作矩形
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_fill_rect(struct ingenic_2d *ingenic_2d, struct ingenic_2d_rect *dst, int color);


/**
 * @brief 图像叠加
 * @param ingenic_2d        2d设备句柄
 * @param src               源的操作矩形
 * @param dst               目标的操作矩形
 * @param global_alpha      源数据叠加时的整体透明度
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_blend(struct ingenic_2d *ingenic_2d,
                     struct ingenic_2d_rect *src,
                     struct ingenic_2d_rect *dst, int global_alpha);



/**
 * @brief 画线
 * @param ingenic_2d        2d设备句柄
 * @param lines             描述线的结构体数组
 * @param line_count        线的个数
 * @param line_color        线的颜色
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_draw_lines(struct ingenic_2d *ingenic_2d,
                          struct ingenic_2d_rect *dst,
                          struct ingenic_2d_line *lines, int line_count, int line_color);


/**
 * @brief 翻转
 * @param ingenic_2d        2d设备句柄
 * @param src               源的操作矩形
 * @param dst               目标的操作矩形
 * @param x_filp            是否使能水平翻转
 * @param y_filp            是否使能垂直翻转
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_filp(struct ingenic_2d *ingenic_2d,
                    struct ingenic_2d_rect *src,
                    struct ingenic_2d_rect *dst,
                    int x_filp, int y_filp);


/**
 * @brief 格式转换
 * @param ingenic_2d        2d设备句柄
 * @param src               源的操作矩形
 * @param dst               目标的操作矩形
 * @return 成功返回0，失败返回负数
 */
int ingenic_2d_convert(struct ingenic_2d *ingenic_2d,
                       struct ingenic_2d_rect *src,
                       struct ingenic_2d_rect *dst);

#endif

