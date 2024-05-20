
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>

#include <libhardware2/rmem.h>
#include <libhardware2/fb.h>


#include <gc_hal.h>
#include <gc_hal_base.h>
#include <gc_hal_raster.h>
#include <libhardware2/fb.h>

//#include <libutils2/boot_time.h>

//#include <sys/cachectl.h>
//DCACHE defined in asm/cachectl.h
#define DCACHE 3
#include <stdlib.h>

#include <lib2d/ingenic2d.h>
#include <libhardware2/rmem.h>

struct ingenic_2d {
    gcoOS       g_os;
    gcoHAL      g_hal;
    gco2D       g_2d;

    int rmem_fd;
};

struct g2d_frame {
    struct ingenic_2d_frame frame;
    int is_rmem_frame;
    gcoSURF surf;
};


static int ingenic_format_to_2d_format(enum ingenic_2d_format format)
{
    switch (format) {
        case INGENIC_2D_ARGB8888: return gcvSURF_A8R8G8B8;
        case INGENIC_2D_RGB565: return gcvSURF_R5G6B5;
        case INGENIC_2D_XRGB8888: return gcvSURF_X8R8G8B8;
        case INGENIC_2D_YUYV422: return gcvSURF_YUY2;
        case INGENIC_2D_UYVY422: return gcvSURF_UYVY;
        default:
            fprintf(stderr, "ingenic_2d: not support this format = %d\n", format);
            break;
    }

    return -1;
}

static void to_2d_rect(struct ingenic_2d_rect *ingenic_2d, gcsRECT *rect)
{
    rect->left = ingenic_2d->x;
    rect->top = ingenic_2d->y;
    rect->right = ingenic_2d->x + ingenic_2d->w;
    rect->bottom = ingenic_2d->y + ingenic_2d->h;
}

static int ingenic_2d_work_out(struct ingenic_2d *ingenic_2d, struct g2d_frame *src, struct g2d_frame *dst)
{

    int ret;

    if (src) {
        ret = gcoSURF_CPUCacheOperation(src->surf, gcvCACHE_CLEAN);
        if (ret < 0) {
            fprintf(stderr, "ingenic_2d : failed to cache srcframe\n");
            return -1;
        }
    }

    if (dst) {
        ret = gcoSURF_CPUCacheOperation(dst->surf, gcvCACHE_CLEAN);
        if (ret < 0) {
            fprintf(stderr, "ingenic_2d: failed to cache dst_frame\n");
            return -1;
        }
    }

    ret = gco2D_Flush(ingenic_2d->g_2d);
    if (ret < 0){
        fprintf(stderr, "ingenic_2d: failed to flush\n");
        return -1;
    }

    ret = gcoHAL_Commit(ingenic_2d->g_hal, gcvTRUE);
    if (ret < 0) {
        fprintf(stderr, "inegnic_2d: failed to commit\n");
        return -1;
    }

    return 0;

}


static int init_frame_info(struct ingenic_2d *ingenic_2d, struct g2d_frame *frame, int width ,int height, enum ingenic_2d_format format)
{

    gceSURF_FORMAT g2d_format = ingenic_format_to_2d_format(format);
    if (g2d_format < 0) {
        return -1;
    }

    int ret = gcoSURF_Construct(ingenic_2d->g_hal,
                            width,
                            height,
                            1,
                            gcvSURF_BITMAP,
                            g2d_format,
                            gcvPOOL_USER,
                            &frame->surf);
    if (ret < 0) {
        fprintf(stderr, "failed to construct 2d surf\n");
        return -1;
    }


    struct ingenic_2d_frame *ingenic_frame = &frame->frame;

    ingenic_frame->width = width;
    ingenic_frame->height = height;
    ingenic_frame->format = format;

    ret = gcoSURF_GetAlignedSize(frame->surf, &ingenic_frame->align_width, &ingenic_frame->align_height, &ingenic_frame->stride);
    if (ret < 0) {
        fprintf(stderr, "failed to get surf size message\n");
        gcoSURF_Destroy(frame->surf);
        return -1;
    }

    ingenic_frame->frame_size = ingenic_frame->align_height * ingenic_frame->stride;

    return 0;
}

static void deinit_frame_info(struct g2d_frame *frame)
{
    gcoSURF_Destroy(frame->surf);
    memset(frame, 0, sizeof(*frame));
}

static int set_frame_buffer_by_user(struct g2d_frame *frame, unsigned long phyaddr, void *addr, int size)
{
    struct ingenic_2d_frame *ingenic_frame = &frame->frame;
    if (ingenic_frame->frame_size < 0) {
        fprintf(stderr, "ingenic_2d :failed to set frame buffer\n");
        return -1;
    }

    int ret;

    ret = gcoSURF_MapUserSurface(frame->surf, ingenic_frame->stride, (gctPOINTER)addr, phyaddr);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to init phy frame, map user surface err\n");
        return -1;
    }

    ret = gcoSURF_Lock(frame->surf, ingenic_frame->phyaddr, (gctPOINTER * )ingenic_frame->addr);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d : failed to get surf mem\n");
        return -1;
    }

    return 0;
}

static int get_frame_buffer(struct ingenic_2d *ingenic_2d, struct g2d_frame *frame)
{
    int ret;
    struct ingenic_2d_frame *ingenic_frame = &frame->frame;

    unsigned long phyaddr;
    void *addr = rmem_alloc(ingenic_2d->rmem_fd, &phyaddr, ingenic_frame->frame_size);
    if (!addr)
        return -1;

    ret = set_frame_buffer_by_user(frame, phyaddr, addr, ingenic_frame->frame_size);
    if (ret < 0) {
        fprintf(stderr, "failed to set frame buffer\n");
        return -1;
    }

    frame->is_rmem_frame = 1;

    memset(ingenic_frame->addr[0], 0, ingenic_frame->align_height * ingenic_frame->stride);
    cacheflush((void*)ingenic_frame->addr[0], ingenic_frame->align_height * ingenic_frame->stride, DCACHE);

    return 0;
}

static void put_frame_buffer(struct ingenic_2d *ingenic_2d, struct g2d_frame *frame)
{
    struct ingenic_2d_frame *ingenic_frame = &frame->frame;

    gcoSURF_Unlock(frame->surf, ingenic_frame->addr[0]);

    if (frame->is_rmem_frame)
        rmem_free(ingenic_2d->rmem_fd, ingenic_frame->addr[0], ingenic_frame->phyaddr[0], ingenic_frame->frame_size);
}

static int g2d_driver_init(struct ingenic_2d *ingenic_2d)
{
    int ret;

    gcoOS       g_os;
    gcoHAL      g_hal;
    gco2D       g_2d;

    ret = gcoOS_Construct(NULL, &g_os);
    if (ret < 0) {
        printf("ingenic_2d: Failed to construct OS object (status = %d)\n", ret);
        return -1;
    }

    /* Construct the gcoHAL object. */
    ret = gcoHAL_Construct(NULL, g_os, &g_hal);
    if (ret < 0) {
        printf("ingenic_2d: Failed to construct GAL object (status = %d)\n", ret);
        goto destroy_os;
    }

    ret = gcoHAL_Get2DEngine(g_hal, &g_2d);
    if (ret < 0) {
        printf("ingenic_2d : Failed to get 2D engine object (ret = %d)\n", ret);
        goto destroy_hal;
    }

    unsigned long contiguous_size = 0;
    void *contiguous_phy = 0;

    gcoHAL_QueryVideoMemory(g_hal, NULL, NULL, NULL, NULL, &contiguous_phy, &contiguous_size);

    if (contiguous_size > 0) {
        fprintf(stderr, "g2d driver is alloced %ld mem, set kerenl g2d driver config to be zero!!\n", contiguous_size);
    }

    ingenic_2d->g_2d = g_2d;
    ingenic_2d->g_hal = g_hal;
    ingenic_2d->g_os = g_os;

    return 0;

destroy_hal:
    gcoHAL_Destroy(g_hal);
destroy_os:
    gcoOS_Destroy(g_os);

    return -1;

}

static void g2d_deinit(struct ingenic_2d *ingenic_2d)
{
    gcoHAL_Commit(ingenic_2d->g_hal, 1);
    gcoHAL_Destroy(ingenic_2d->g_hal);
    gcoOS_Destroy(ingenic_2d->g_os);
}


/*---------------------------------------------------------------------------------------------------------------------------------*/

struct ingenic_2d *ingenic_2d_open(void)
{
    int ret;

    struct ingenic_2d *ingenic_2d = malloc(sizeof(*ingenic_2d));
    if (!ingenic_2d) {
        fprintf(stderr, "failed to alloc g2d handle\n");
        return NULL;
    }

    ret = g2d_driver_init(ingenic_2d);
    if (ret < 0)
        goto free_2d;

    int rmem_fd = rmem_open();
    if (rmem_fd < 0) {
        fprintf(stderr, "ingenic_2d: failed to open, rmem open err\n");
        goto deinit_2d;
    }

    ingenic_2d->rmem_fd = rmem_fd;

    return ingenic_2d;

deinit_2d:
    g2d_deinit(ingenic_2d);
free_2d:
    free(ingenic_2d);

    return NULL;
}

void ingenic_2d_close(struct ingenic_2d *ingenic_2d)
{
    g2d_deinit(ingenic_2d);

    rmem_close(ingenic_2d->rmem_fd);

    free(ingenic_2d);
}

struct ingenic_2d_frame *ingenic_2d_alloc_frame(struct ingenic_2d *ingenic_2d, int width, int height,
                                                enum ingenic_2d_format format)
{
    struct g2d_frame *frame = malloc(sizeof(*frame));
    if (!frame) {
        fprintf(stderr, "ingenic_2d :failed to alloc 2d frame\n");
        return NULL;
    }

    int ret = init_frame_info(ingenic_2d, frame, width, height, format);
    if (ret < 0) {
        goto free_frame;
    }

    ret = get_frame_buffer(ingenic_2d, frame);
    if (ret < 0)
        goto deinit_frame;


    return &frame->frame;

deinit_frame:
     deinit_frame_info(frame);
free_frame:
    free(frame);

    return NULL;
}

void ingenic_2d_free_frame(struct ingenic_2d *ingenic_2d, struct ingenic_2d_frame *frame)
{
    struct g2d_frame *g2d_frame = (struct g2d_frame *)frame;

    put_frame_buffer(ingenic_2d, g2d_frame);
    deinit_frame_info(g2d_frame);

    free(g2d_frame);
}


struct ingenic_2d_frame *ingenic_2d_alloc_frame_by_user(struct ingenic_2d *ingenic_2d,
                                                        int width, int height,enum ingenic_2d_format format,
                                                        unsigned long phyaddr, void *addr, int addrsize)
{
    struct g2d_frame *frame = malloc(sizeof(*frame));
    if (!frame) {
        fprintf(stderr, "ingenic_2d :failed to alloc 2d frame\n");
        return NULL;
    }

    int ret = init_frame_info(ingenic_2d, frame, width, height, format);
    if (ret < 0) {
        goto free_frame;
    }

    ret = set_frame_buffer_by_user(frame, phyaddr, addr, addrsize);
    if (ret < 0)
        goto deinit_frame;

    return &frame->frame;

deinit_frame:
     deinit_frame_info(frame);
free_frame:
    free(frame);

    return NULL;

}


struct ingenic_2d_rect ingenic_2d_rect_init(struct ingenic_2d_frame *frame, int x, int y, int w, int h)
{
    struct ingenic_2d_rect rect;

    rect.frame = frame;
    rect.x = x < 0 ? 0 : x;
    rect.y = y < 0 ? 0 : y;
    rect.w = w < 0 ? frame->width : w;
    rect.h = h < 0 ? frame->height : h;

    if (x + w > frame->width)
        rect.w = frame->width - x;
    if (y + h > frame->height)
        rect.h = frame->height - y;

    return rect;
}


int ingenic_2d_rotate(struct ingenic_2d *ingenic_2d, enum ingenic_2d_rotate_angle angle,
                      struct ingenic_2d_rect *src,struct ingenic_2d_rect *dst)
{
    int ret;

    struct ingenic_2d_frame *s_frame = src->frame;
    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT s_format = ingenic_format_to_2d_format(s_frame->format);
    if (s_format < 0)
        return -1;
    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT src_rect = {0};
    gcsRECT dst_rect = {0};

    to_2d_rect(src, &src_rect);
    to_2d_rect(dst, &dst_rect);

    ret = gco2D_SetKernelSize(ingenic_2d->g_2d, 9, 9);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: 2d set kernel size failed\n");
        return -1;
    }


    ret = gco2D_SetColorSource(ingenic_2d->g_2d, s_frame->phyaddr[0],
                                            s_frame->stride,
                                            s_format,
                                            gcvSURF_0_DEGREE,
                                            s_frame->align_width,
                                            gcvFALSE,
                                            gcvSURF_OPAQUE,
                                            0);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: 2d set colorsource failed\n");
        return -1;
    }

    ret = gco2D_SetSource(ingenic_2d->g_2d, &src_rect);
    if (ret < 0) {
        fprintf(stderr, "inegnic_2d: 2d set source range failed\n");
        return -1;
    }


    ret = gco2D_SetClipping(ingenic_2d->g_2d, &src_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: 2d set clipping failed\n");
        return -1;
    }

    ret = gco2D_SetTargetEx(ingenic_2d->g_2d, d_frame->phyaddr[0],
                            d_frame->stride,
                            angle,
                            d_frame->align_width,
                            d_frame->align_height);
    if (ret < 0) {
        fprintf(stderr, "inengic_2d: 2d set target failed\n");
        return -1;
    }


    struct g2d_frame *gs_frame = (struct g2d_frame *)s_frame;
    struct g2d_frame *gd_frame = (struct g2d_frame *)d_frame;

    ret = gcoSURF_FilterBlit(gs_frame->surf, gd_frame->surf, &src_rect, &dst_rect, NULL);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to rotater, filter blit err\n");
        return -1;
    }


    return ingenic_2d_work_out(ingenic_2d, gs_frame, gd_frame);

}


int ingenic_2d_scale(struct ingenic_2d *ingenic_2d, struct ingenic_2d_rect *src, struct ingenic_2d_rect *dst)
{
    int ret;

    struct ingenic_2d_frame *s_frame = src->frame;
    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT s_format = ingenic_format_to_2d_format(s_frame->format);
    if (s_format < 0)
        return -1;
    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT src_rect = {0};
    gcsRECT dst_rect = {0};

    to_2d_rect(src, &src_rect);
    to_2d_rect(dst, &dst_rect);

    ret = gco2D_SetClipping(ingenic_2d->g_2d, &src_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to scale, set clipping err\n");
        return -1;
    }

    ret = gco2D_SetKernelSize(ingenic_2d->g_2d, 1, 1);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to scale, set kernel size err\n");
        return -1;
    }

    struct g2d_frame *gs_frame = (struct g2d_frame *)s_frame;
    struct g2d_frame *gd_frame = (struct g2d_frame *)d_frame;

    ret = gcoSURF_FilterBlit(gs_frame->surf, gd_frame->surf, &src_rect, &dst_rect, NULL);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to scale, filter blit err\n");
        return -1;
    }

    return ingenic_2d_work_out(ingenic_2d, gs_frame, gd_frame);
}

int ingenic_2d_fill_rect(struct ingenic_2d *ingenic_2d, struct ingenic_2d_rect *dst, int color)
{
    int ret;

    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT dst_rect = {0};

    to_2d_rect(dst, &dst_rect);

    gcoBRUSH bgBrush;

    ret = gco2D_ConstructSingleColorBrush(ingenic_2d->g_2d, 1, color, 0, &bgBrush);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to fill, construct single color err\n");
        return -1;
    }


    ret = gco2D_FlushBrush(ingenic_2d->g_2d, bgBrush, d_format);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to fill, flush brush err\n");
        return -1;
    }


    ret = gco2D_SetTarget(ingenic_2d->g_2d, d_frame->phyaddr[0], d_frame->stride, 0, d_frame->width);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to fill, set target err\n");
        return -1;
    }

    ret = gco2D_SetClipping(ingenic_2d->g_2d, &dst_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to fill, set clipping err\n");
        return -1;
    }

    ret = gco2D_Blit(ingenic_2d->g_2d, 1, &dst_rect, 0xF0, 0xF0, d_format);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to fill, blit err\n");
    }

    gcoBRUSH_Destroy(bgBrush);

    return ingenic_2d_work_out(ingenic_2d, NULL, (struct g2d_frame *)d_frame);
}

int ingenic_2d_blend(struct ingenic_2d *ingenic_2d,
                     struct ingenic_2d_rect *src, struct ingenic_2d_rect *dst, int global_alpha)
{
    int ret;

    struct ingenic_2d_frame *s_frame = src->frame;
    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT s_format = ingenic_format_to_2d_format(s_frame->format);
    if (s_format < 0)
        return -1;
    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT src_rect = {0};
    gcsRECT dst_rect = {0};

    to_2d_rect(src, &src_rect);
    to_2d_rect(dst, &dst_rect);

    int src_alpha = global_alpha;

    if (global_alpha > 0xf8)
        src_alpha = 0xff;
    if (global_alpha < 0x08)
        src_alpha = 0;

    gco2D_SetKernelSize(ingenic_2d->g_2d, 9, 9);

    ret = gco2D_EnableAlphaBlend(ingenic_2d->g_2d,
                    src_alpha, 0xff,
                    gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
                    gcvSURF_GLOBAL_ALPHA_SCALE, gcvSURF_GLOBAL_ALPHA_SCALE,
                    gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,
                    gcvSURF_COLOR_STRAIGHT, gcvSURF_COLOR_STRAIGHT);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to enable alpha blend\n");
        return -1;
    }


    ret = gco2D_SetColorSource(ingenic_2d->g_2d, s_frame->phyaddr[0],
                                            s_frame->stride,
                                            s_format,
                                            gcvSURF_0_DEGREE,
                                            s_frame->align_width,
                                            gcvFALSE,
                                            gcvSURF_OPAQUE,
                                             0);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to blend, set colorsource err\n");
        return -1;
    }


    ret = gco2D_SetSource(ingenic_2d->g_2d, &src_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to blend, set sourc err\n");
        return -1;
    }

    ret = gco2D_SetClipping(ingenic_2d->g_2d, &dst_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to blend, set clipping err\n");
        return -1;
    }


    ret = gco2D_SetTarget(ingenic_2d->g_2d, d_frame->phyaddr[0], d_frame->stride,
                          gcvFALSE, d_frame->align_width);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to blend, failed to set target\n");
        return -1;
    }


    ret = gco2D_Blit(ingenic_2d->g_2d, 1, &dst_rect, 0xCC, 0xCC, d_format);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to blend, blit err\n");
        return -1;
    }

    ret = ingenic_2d_work_out(ingenic_2d, (struct g2d_frame *)s_frame, (struct g2d_frame *)d_frame);

    gco2D_DisableAlphaBlend(ingenic_2d->g_2d);

    return ret;
}

int ingenic_2d_draw_lines(struct ingenic_2d *ingenic_2d,
                          struct ingenic_2d_rect *dst,
                          struct ingenic_2d_line *lines, int line_count, int line_color)
{
    int ret;

    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT dst_rect = {0};

    to_2d_rect(dst, &dst_rect);

    ret = gco2D_SetClipping(ingenic_2d->g_2d, &dst_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to draw lines, set clipping err\n");
        return -1;
    }

    ret = gco2D_SetTarget(ingenic_2d->g_2d, d_frame->phyaddr[0], d_frame->stride, gcvSURF_0_DEGREE, d_frame->align_width);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to draw lines, set target err\n");
        return -1;
    }


    ret = gco2D_ColorLine(ingenic_2d->g_2d, line_count, (gcsRECT *)lines, line_color, 0xCC, 0xCC, d_format);
    if (ret < 0) {
        fprintf(stderr, "ingenic 2d: failed to draw lines, color line err\n");
        return -1;
    }

    return ingenic_2d_work_out(ingenic_2d, NULL, (struct g2d_frame *)d_frame);
}


int ingenic_2d_filp(struct ingenic_2d *ingenic_2d,
                    struct ingenic_2d_rect *src,
                    struct ingenic_2d_rect *dst, int x_filp, int y_filp)
{
    int ret;

    struct ingenic_2d_frame *s_frame = src->frame;
    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT s_format = ingenic_format_to_2d_format(s_frame->format);
    if (s_format < 0)
        return -1;
    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT src_rect = {0};
    gcsRECT dst_rect = {0};

    to_2d_rect(src, &src_rect);
    to_2d_rect(dst, &dst_rect);

    ret = gco2D_SetColorSource(ingenic_2d->g_2d, s_frame->phyaddr[0],
                                           s_frame->stride,
                                           s_format,
                                           gcvSURF_0_DEGREE,
                                           s_frame->align_width,
                                           gcvFALSE,
                                           gcvSURF_OPAQUE,
                                           0);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to filp, set color source err\n");
        return ret;
    }


    ret = gco2D_SetKernelSize(ingenic_2d->g_2d, 9, 9);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to filp, set kernel size err\n");
        return -1;
    }


    ret = gco2D_SetSource(ingenic_2d->g_2d, &src_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d :failed to filp, set source rect err\n");
        return -1;
    }

    ret = gco2D_SetClipping(ingenic_2d->g_2d, &dst_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to filp, set clipping err");
        return -1;
    }

	ret = gco2D_SetTargetEx(ingenic_2d->g_2d, d_frame->phyaddr[0],
                            d_frame->stride,
                            gcvSURF_0_DEGREE,
                            d_frame->align_width,
                            d_frame->align_height);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to set filp, set targetex err\n");
        return -1;
    }


    ret = gco2D_SetBitBlitMirror(ingenic_2d->g_2d, x_filp, y_filp);
    if (ret < 0 ) {
        fprintf(stderr, "ingenic_2d: failed to filp, set blit mirror err");
        return -1;
    }

    ret = gco2D_Blit(ingenic_2d->g_2d, 1, &dst_rect, 0xCC, 0xCC, d_format);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to filp, blit err\n");
        return -1;
    }

    ret = gco2D_SetBitBlitMirror(ingenic_2d->g_2d, 0, 0);
    if (ret < 0 ) {
        fprintf(stderr, "ingenic_2d: failed to filp, set blit mirror err");
        return -1;
    }

    return ingenic_2d_work_out(ingenic_2d, (struct g2d_frame *)s_frame, (struct g2d_frame *)d_frame);
}

int ingenic_2d_convert(struct ingenic_2d *ingenic_2d, struct ingenic_2d_rect *src, struct ingenic_2d_rect *dst)
{
    int ret;

    struct ingenic_2d_frame *s_frame = src->frame;
    struct ingenic_2d_frame *d_frame = dst->frame;

    gceSURF_FORMAT s_format = ingenic_format_to_2d_format(s_frame->format);
    if (s_format < 0)
        return -1;
    gceSURF_FORMAT d_format = ingenic_format_to_2d_format(d_frame->format);
    if (d_format < 0)
        return -1;

    gcsRECT src_rect = {0};
    gcsRECT dst_rect = {0};

    to_2d_rect(src, &src_rect);
    to_2d_rect(dst, &dst_rect);

    ret = gco2D_SetClipping(ingenic_2d->g_2d, &dst_rect);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to convert, set clipping err\n");
        return -1;
    }

    ret = gco2D_SetKernelSize(ingenic_2d->g_2d, 1, 1);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to convert, set kernel size err\n");
        return -1;
    }

    struct g2d_frame *gs_frame = (struct g2d_frame *)s_frame;
    struct g2d_frame *gd_frame = (struct g2d_frame *)d_frame;

    ret = gcoSURF_FilterBlit(gs_frame->surf, gd_frame->surf, &src_rect, &dst_rect, NULL);
    if (ret < 0) {
        fprintf(stderr, "ingenic_2d: failed to convert, filter blit err\n");
        return -1;
    }

    return ingenic_2d_work_out(ingenic_2d, gs_frame, gd_frame);
}
