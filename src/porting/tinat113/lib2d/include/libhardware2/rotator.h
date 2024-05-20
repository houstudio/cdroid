#ifndef __LIBHARDWARE2_ROTATOR_H__
#define __LIBHARDWARE2_ROTATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

enum rotator_fmt {
    ROTATOR_RGB555,
    ROTATOR_ARGB1555,
    ROTATOR_RGB565,
    ROTATOR_RGB888,
    ROTATOR_ARGB8888,//default
    ROTATOR_Y8,
    ROTATOR_YUV422,
    ROTATOR_NV12,
};

enum rotator_convert_order {
    ROTATOR_ORDER_RGB_TO_RGB,//default
    ROTATOR_ORDER_RBG_TO_RGB,
    ROTATOR_ORDER_GRB_TO_RGB,
    ROTATOR_ORDER_GBR_TO_RGB,
    ROTATOR_ORDER_BRG_TO_RGB,
    ROTATOR_ORDER_BGR_TO_RGB,
};

enum rotator_mirror {
    ROTATOR_NO_MIRROR,
    ROTATOR_MIRROR,
};

enum rotator_angle {
    ROTATOR_ANGLE_0,
    ROTATOR_ANGLE_90,
    ROTATOR_ANGLE_180,
    ROTATOR_ANGLE_270,
};

struct rotator_config_data {
    void *src_buf;
    void *dst_buf;

    unsigned int frame_height;
    unsigned int frame_width;

    unsigned int src_stride;
    unsigned int dst_stride;

    enum rotator_fmt src_fmt;
    enum rotator_convert_order convert_color;
    enum rotator_fmt dst_fmt;

    enum rotator_mirror horizontal_mirror;
    enum rotator_mirror vertical_mirror;
    enum rotator_angle rotate_angle;
};

int rotator_open(void);
int rotator_complete_conversion(int handle, struct rotator_config_data *data);
void rotator_close(int handle);

#ifdef __cplusplus
}
#endif

#endif
