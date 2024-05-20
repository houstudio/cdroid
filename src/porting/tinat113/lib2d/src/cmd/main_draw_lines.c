#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <lib2d/ingenic2d.h>
#include <libhardware2/fb.h>


#define DST_WIDTH     1920
#define DST_HEIGHT    1080

#define LINE_COLOR    0xff578845

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

    struct ingenic_2d_line lines[8];

    lines[0].x0   = 0;
    lines[0].y0    = 0;
    lines[0].x1  = dst_frame->width;
    lines[0].y1 = dst_frame->height;

    lines[1].x0   = dst_frame->width;
    lines[1].y0    = 0;
    lines[1].x1  = 0;
    lines[1].y1 = dst_frame->height;

    lines[2].x0   = 0;
    lines[2].y0    = dst_frame->height/2;
    lines[2].x1  = dst_frame->width;
    lines[2].y1 = dst_frame->height/2;

    lines[3].x0   = dst_frame->width/2;
    lines[3].y0    = 0;
    lines[3].x1  = dst_frame->width/2;
    lines[3].y1 = dst_frame->height;

    lines[4].x0   = dst_frame->width/4;
    lines[4].y0    = 0;
    lines[4].x1  = dst_frame->width/4;
    lines[4].y1 = dst_frame->height;

    lines[5].x0   = dst_frame->width*3/4;
    lines[5].y0    = 0;
    lines[5].x1  = dst_frame->width*3/4;
    lines[5].y1 = dst_frame->height;

    lines[6].x0   = 0;
    lines[6].y0    = dst_frame->height/4;
    lines[6].x1  = dst_frame->width;
    lines[6].y1 = dst_frame->height/4;

    lines[7].x0   = 0;
    lines[7].y0    = dst_frame->height*3/4;
    lines[7].x1  = dst_frame->width;
    lines[7].y1 = dst_frame->height*3/4;


    struct ingenic_2d_rect dst_rect = ingenic_2d_rect_init(dst_frame, -1, -1, -1, -1);

    ret = ingenic_2d_draw_lines(ingenic_2d, &dst_rect, lines, 8, LINE_COLOR);
    if (ret < 0)
        return -1;


    fb_display(fb_fd, &fb_info,dst_rect.frame);

    file_write_data("/usr/data/output.raw", dst_frame->addr[0], dst_frame->stride * dst_frame->align_height);

    ingenic_2d_free_frame(ingenic_2d, dst_frame);
    ingenic_2d_close(ingenic_2d);
    fb_close(fb_fd, &fb_info);

    return 0;

}