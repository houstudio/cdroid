#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <lib2d/ingenic2d.h>
#include <libhardware2/fb.h>


#define SRC_GLOBAL_ALPHA    0xff

#define SRC_WIDTH        1280
#define SRC_HEIGHT       720


#define DST_WIDTH     1920
#define DST_HEIGHT    1080

#define src_file_path "/usr/data/src_1280_720.rgb"
#define dst_file_path "/usr/data/dst_1920_1080.rgb"

#include "utils.c"


static int fb_fd;
static struct fb_device_info fb_info;

int main(void)
{
    int ret;
    FILE *srcfile = fopen(src_file_path, "rb");
    if (!srcfile) {
        fprintf(stderr, "failed to open src faile\n");
        return -1;
    }

    FILE *dstfile = fopen(dst_file_path, "rb");
    if (!dstfile) {
        fprintf(stderr, "failed to open dst_file\n");
        fclose(srcfile);
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

    read_raw(srcfile, src_frame->addr[0], src_frame->align_height, SRC_WIDTH * 4, src_frame->stride);
    read_raw(dstfile, dst_frame->addr[0], dst_frame->align_height, DST_WIDTH * 4, dst_frame->stride);


    struct ingenic_2d_rect src_rect = ingenic_2d_rect_init(src_frame, 0, 0, SRC_WIDTH, SRC_HEIGHT);

    struct ingenic_2d_rect dst_rect = ingenic_2d_rect_init(dst_frame, (DST_WIDTH - SRC_WIDTH) / 2,
                                                            (DST_HEIGHT - SRC_HEIGHT) / 2, SRC_WIDTH, SRC_HEIGHT);

    ret = ingenic_2d_blend(ingenic_2d, &src_rect, &dst_rect, SRC_GLOBAL_ALPHA);
    if (ret < 0)
        return -1;

    fb_display(fb_fd, &fb_info, dst_rect.frame);

    file_write_data("/usr/data/output.raw", dst_frame->addr[0], dst_frame->stride * dst_frame->align_height);

    ingenic_2d_free_frame(ingenic_2d, src_frame);
    ingenic_2d_free_frame(ingenic_2d, dst_frame);

    ingenic_2d_close(ingenic_2d);

    fb_close(fb_fd, &fb_info);

    return 0;

}