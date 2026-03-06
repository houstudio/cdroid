#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libhardware2/fb.h>
#include <lib2d/ingenic2d.h>

#define SRC_WIDTH  1080
#define SRC_HEIGHT 1920

#define DST_WIDTH 1080
#define DST_HEIGHT 1920

#define SRC_RECT_WIDTH  800
#define SRC_RECT_HEIGHT 480

#define DST_RECT_WIDTH  720
#define DST_RECT_HEIGHT 1280

#define FILE_PATH "/usr/data/1080_1920_rgb.yuv"

#include "utils.c"

static int fb_fd;
static struct fb_device_info fb_info;


int main(void)
{
    int ret;
    FILE *infile = fopen(FILE_PATH, "rb");
    if (!infile) {
        fprintf(stderr, "failed to open faile %s\n", FILE_PATH);
        return -1;
    }

    fb_fd = fb_open("/dev/fb0", &fb_info);
    if (fb_fd < 0) {
        fprintf(stderr, "failed to open fb\n");
        return -1;
    }

    fb_enable(fb_fd);


    struct ingenic_2d *ingenic_2d = ingenic_2d_open();
    if (!ingenic_2d)
        return -1;

    struct ingenic_2d_frame *src_frame = ingenic_2d_alloc_frame(ingenic_2d, SRC_WIDTH, SRC_HEIGHT, INGENIC_2D_ARGB8888);
    if (!src_frame)
        return -1;

    struct ingenic_2d_frame *dst_frame = ingenic_2d_alloc_frame(ingenic_2d, DST_WIDTH, DST_HEIGHT, INGENIC_2D_ARGB8888);
    if (!dst_frame)
        return -1;

    read_raw(infile, src_frame->addr[0], SRC_HEIGHT, SRC_WIDTH * 4, src_frame->stride);


    struct ingenic_2d_rect src_rect = ingenic_2d_rect_init(src_frame, (SRC_WIDTH - SRC_RECT_WIDTH) / 2,
                                                            (SRC_HEIGHT - SRC_RECT_HEIGHT) / 2, SRC_RECT_WIDTH, SRC_RECT_HEIGHT);


    struct ingenic_2d_rect dst_rect = ingenic_2d_rect_init(dst_frame, 0,
                                                            0, DST_RECT_WIDTH, DST_RECT_HEIGHT);

    ret = ingenic_2d_scale(ingenic_2d, &src_rect, &dst_rect);
    if (ret < 0)
        return -1;

    fb_display(fb_fd, &fb_info,dst_rect.frame);

    file_write_data("/usr/data/output.raw", dst_frame->addr[0], dst_frame->stride * dst_frame->align_height);


    ingenic_2d_free_frame(ingenic_2d, src_frame);
    ingenic_2d_free_frame(ingenic_2d, dst_frame);

    ingenic_2d_close(ingenic_2d);

    fb_close(fb_fd, &fb_info);

    return 0;
}