#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <lib2d/ingenic2d.h>
#include <libhardware2/fb.h>



#define DST_WIDTH  1920
#define DST_HEIGHT 1080

#define DRAW_X    0
#define DRAW_Y    0

#define DRAW_WIDTH      1280
#define DRAW_HEIGHT     720


#define DRAW_COLOR       0xff00ff00

#include "utils.c"

static int fb_fd;
static struct fb_device_info fb_info;

int main(void)
{
    int ret;

    fb_fd = fb_open("/dev/fb0", &fb_info);
    if (fb_fd < 0) {
        fprintf(stderr, "failed to open fb\n");
        return -1;
    }

    fb_enable(fb_fd);


    struct ingenic_2d *ingenic_2d = ingenic_2d_open();
    if (!ingenic_2d)
        return -1;

    struct ingenic_2d_frame *dst_frame = ingenic_2d_alloc_frame(ingenic_2d, DST_WIDTH, DST_HEIGHT, INGENIC_2D_ARGB8888);
    if (!dst_frame)
        return -1;

    struct ingenic_2d_rect dst_rect = ingenic_2d_rect_init(dst_frame, DRAW_X,
                                                            DRAW_Y, DRAW_WIDTH, DRAW_HEIGHT);

    ret = ingenic_2d_fill_rect(ingenic_2d, &dst_rect, DRAW_COLOR);
    if (ret < 0)
        return -1;

    fb_display(fb_fd, &fb_info, dst_rect.frame);

    file_write_data("/usr/data/output.raw", dst_frame->addr[0], dst_frame->stride * dst_frame->align_height);

    ingenic_2d_free_frame(ingenic_2d, dst_frame);

    ingenic_2d_close(ingenic_2d);

    fb_close(fb_fd, &fb_info);

    return 0;
}