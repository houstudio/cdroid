/*
 * Copyright (C) 2022-2023 ArtinChip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <video/artinchip_ge.h>

#include "ge_ops.h"
#include "ge_reg.h"
#include "mpp_list.h"
#include "mpp_ge.h"

#define DEFAULT_CMD_SIZE (32*1024 / 4)
#define CSC_COEFFS_NUM 12
#define VALID_SPACE (1024 / 4)
#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096

#define ALIGN_8B(x) (((x) + (7)) & ~(7))
#define ALIGN_2B(x) (((x) + (1)) & ~(1))

struct ge_dmabuf_head {
	struct mpp_list list;
	struct dma_buf_info dma_info;
};

struct ge_data {
	unsigned int src_phy_addr[3];
	unsigned int dst_phy_addr[3];
	unsigned int src_premul_en;
	unsigned int src_de_premul_en;
	unsigned int dst_de_premul_en;
	unsigned int out_premul_en;
	unsigned int src_alpha_coef;
	unsigned int dst_alpha_coef;
	unsigned int csc0_en;
	unsigned int csc1_en;
	unsigned int csc2_en;
	unsigned int blend_is_rgb;
	unsigned int src_ctrl;
	unsigned int out_ctrl;
};

struct cmd_queue {
	unsigned int *cmd_buf;
	struct mpp_list dma_list;
	unsigned int total_size;
	unsigned int write_offset;
	struct ge_data data;
};

static const unsigned int yuv2rgb_bt601[CSC_COEFFS_NUM] = {
	0x04a8, 0x0000, 0x0662, 0x3212,
	0x04a8, 0x1e70, 0x1cc0, 0x087a,
	0x04a8, 0x0811, 0x0000, 0x2eb4
};

static const unsigned int yuv2rgb_bt709[CSC_COEFFS_NUM] = {
	0x04a8, 0x0000, 0x0722, 0x3093,
	0x04a8, 0x1f27, 0x1ddf, 0x04ce,
	0x04a8, 0x0873, 0x0000, 0x2df2
};

static const unsigned int yuv2rgb_bt601_full[CSC_COEFFS_NUM] = {
	0x0400, 0x0000, 0x059c, 0x34ca,
	0x0400, 0x1ea1, 0x1d26, 0x0877,
	0x0400, 0x0717, 0x0000, 0x31d4
};

static const unsigned int yuv2rgb_bt709_full[CSC_COEFFS_NUM] = {
	0x0400, 0x0000, 0x064d, 0x3368,
	0x0400, 0x1f41, 0x1e22, 0x053e,
	0x0400, 0x076c, 0x0000, 0x3129
};

static const int rgb2yuv_bt601[CSC_COEFFS_NUM] = {
	66,  129, 25, 16,
	-38, -74,  112, 128,
	112, -94, -18, 128
};

static const int rgb2yuv_bt709[CSC_COEFFS_NUM] = {
	47, 157, 16, 16,
	-26, -87, 112, 128,
	112, -102, -10, 128
};

static const int rgb2yuv_bt601_full[CSC_COEFFS_NUM] = {
	77, 150, 29, 0,
	-42, -84, 128, 128,
	128, -106, -20, 128
};

static const int rgb2yuv_bt709_full[CSC_COEFFS_NUM] = {
	54, 183, 18, 0,
	-28, -98, 128, 128,
	128, -115, -11, 128
};

static void scaler0_en_cmd(struct mpp_ge *ge, unsigned int enable);

static inline void update_scaler0_cmd(struct mpp_ge *ge,
				      unsigned int input_w,
				      unsigned int input_h,
				      unsigned int output_w,
				      unsigned int output_h,
				      int dx, int dy,
				      int h_phase, int v_phase,
				      unsigned int channel);

static int set_premuliply(struct ge_data *data,
			  enum mpp_pixel_format src_format,
			  enum mpp_pixel_format dst_format,
			  int src_premul,
			  int dst_premul,
			  int is_fill_color,
			  struct ge_ctrl *ctrl);

static inline unsigned int ge_set_src_ctrl(unsigned int global_alpha,
					   unsigned int alpha_mode,
					   unsigned int premul_en,
					   unsigned int scan_order,
					   unsigned int func_select,
					   unsigned int fmt,
					   unsigned int v_flip,
					   unsigned int h_flip,
					   unsigned int rot0_ctrl,
					   unsigned int source_mode,
					   unsigned int csc0_en);

static void update_src_fillrect_cmd(struct mpp_ge *ge, unsigned int src_ctrl,
				    unsigned int  w, unsigned int  h,
				    unsigned int start_color);

static void set_alpha_rules(struct ge_data *data,
			    enum ge_pd_rules rules);

static void update_output_cmd(struct mpp_ge *ge, unsigned int out_ctrl,
			      unsigned int w, unsigned int h,
			      unsigned int stride0, unsigned int stride1,
			      unsigned int addr[]);

static void update_dst_cmd(struct mpp_ge *ge, unsigned int dst_ctrl,
			   unsigned int w, unsigned int h,
			   unsigned int stride0, unsigned int stride1,
			   unsigned int addr[]);

static void update_dst_disable_cmd(struct mpp_ge *ge);

static void update_src_cmd(struct mpp_ge *ge, unsigned int src_ctrl,
			   unsigned int w, unsigned int h,
			   unsigned int stride0, unsigned int stride1,
			   unsigned int addr[]);

static inline struct cmd_queue *to_cmdq(struct mpp_ge *ge)
{
	return (struct cmd_queue *)ge->priv;
}

/*
 *@addr[]: in/out addr
 *
 */
static int ge_buf_crop(unsigned int addr[], unsigned int stride[],
		       enum mpp_pixel_format format,
		       unsigned int x_offset,
		       unsigned int y_offset,
		       unsigned int width,
		       unsigned int height)
{
	int offset;

	switch (format) {
	case MPP_FMT_ARGB_8888:
	case MPP_FMT_ABGR_8888:
	case MPP_FMT_RGBA_8888:
	case MPP_FMT_BGRA_8888:
	case MPP_FMT_XRGB_8888:
	case MPP_FMT_XBGR_8888:
	case MPP_FMT_RGBX_8888:
	case MPP_FMT_BGRX_8888:
		addr[0] += x_offset * 4 + y_offset * stride[0];
		break;
	case MPP_FMT_RGB_888:
	case MPP_FMT_BGR_888:
		addr[0] += x_offset * 3 + y_offset * stride[0];
		break;
	case MPP_FMT_ARGB_1555:
	case MPP_FMT_ABGR_1555:
	case MPP_FMT_RGBA_5551:
	case MPP_FMT_BGRA_5551:
	case MPP_FMT_RGB_565:
	case MPP_FMT_BGR_565:
	case MPP_FMT_ARGB_4444:
	case MPP_FMT_ABGR_4444:
	case MPP_FMT_RGBA_4444:
	case MPP_FMT_BGRA_4444:
		addr[0] += x_offset * 2 + y_offset * stride[0];
		break;
	case MPP_FMT_YUV420P:
		addr[0] += x_offset + y_offset * stride[0];
		offset = (x_offset >> 1) + (y_offset >> 1) * stride[1];
		addr[1] += offset;
		addr[2] += offset;
		break;
	case MPP_FMT_NV12:
	case MPP_FMT_NV21:
		addr[0] += x_offset + y_offset * stride[0];
		addr[1] += x_offset + (y_offset >> 1) * stride[1];
		break;
	case MPP_FMT_YUV400:
		addr[0] += x_offset + y_offset * stride[0];
		break;
	case MPP_FMT_YUV422P:
		addr[0] += x_offset + y_offset * stride[0];
		offset = (x_offset >> 1) + y_offset * stride[1];
		addr[1] += offset;
		addr[2] += offset;
		break;
	case MPP_FMT_NV16:
	case MPP_FMT_NV61:
		addr[0] += x_offset + y_offset * stride[0];
		addr[1] += x_offset + y_offset * stride[1];
		break;
	case MPP_FMT_YUYV:
	case MPP_FMT_YVYU:
	case MPP_FMT_UYVY:
	case MPP_FMT_VYUY:
		addr[0] += (x_offset << 1) + y_offset * stride[0];
		break;
	case MPP_FMT_YUV444P:
		addr[0] += x_offset + y_offset * stride[0];
		addr[1] += x_offset + y_offset * stride[1];
		addr[2] += x_offset + y_offset * stride[1];
		break;
	default:
		return -1;
	}

	return 0;
}

static int dma_fd_to_addr(struct mpp_ge *ge, int dma_fd,
			  unsigned int *phy_addr)
{
	int ret = -1;
	struct cmd_queue *cmdq;
	struct ge_dmabuf_head *node = NULL, *head = NULL;

	if (!ge)
		return -1;

	cmdq = to_cmdq(ge);
	mpp_list_for_each_entry_safe(head, node, &cmdq->dma_list, list) {
		if (head->dma_info.fd == dma_fd) {
			*phy_addr = head->dma_info.phy_addr;
			ret = 0;
			break;
		}
	}

	return ret;
}

static int ge_add_dma_buf(struct mpp_ge *ge, int dma_fd)
{
	int ret;
	struct cmd_queue *cmdq;
	struct ge_dmabuf_head *head;

	if (!ge)
		return -1;

	head =(struct ge_dmabuf_head *)malloc(sizeof(*head));

	if (!head) {
		printf("%s() malloc failed\n", __func__);
		return -1;
	}

	memset(head, 0, sizeof(struct ge_dmabuf_head));
	head->dma_info.fd = dma_fd;
	cmdq = to_cmdq(ge);

	ret = ioctl(ge->dev_fd, IOC_GE_ADD_DMA_BUF, &head->dma_info);
	if (ret < 0) {
		printf("IOC_GE_ADD_DMA_BUF failed %d\n", ret);
		free(head);
		return -1;
	}

	mpp_list_add_tail(&head->list, &cmdq->dma_list);
	return 0;
}

static int ge_rm_dma_buf(struct mpp_ge *ge, int dma_fd)
{
	int ret;
	struct cmd_queue *cmdq;
	struct dma_buf_info dma_info;
	struct ge_dmabuf_head *node = NULL, *head = NULL;

	if (!ge)
		return -1;

	cmdq = to_cmdq(ge);

	mpp_list_for_each_entry_safe(head, node, &cmdq->dma_list, list) {
		if (head->dma_info.fd == dma_fd) {
			mpp_list_del(&head->list);
			free(head);
			break;
		}
	}

	dma_info.fd = dma_fd;
	ret = ioctl(ge->dev_fd, IOC_GE_RM_DMA_BUF, &dma_info);
	if (ret < 0) {
		printf("IOC_GE_RM_DMA_BUF failed:%d\n", ret);
		return -1;
	}

	return 0;
}

static inline int is_rgb(enum mpp_pixel_format format)
{
	switch (format) {
	case MPP_FMT_ARGB_8888:
	case MPP_FMT_ABGR_8888:
	case MPP_FMT_RGBA_8888:
	case MPP_FMT_BGRA_8888:
	case MPP_FMT_XRGB_8888:
	case MPP_FMT_XBGR_8888:
	case MPP_FMT_RGBX_8888:
	case MPP_FMT_BGRX_8888:
	case MPP_FMT_RGB_888:
	case MPP_FMT_BGR_888:
	case MPP_FMT_ARGB_1555:
	case MPP_FMT_ABGR_1555:
	case MPP_FMT_RGBA_5551:
	case MPP_FMT_BGRA_5551:
	case MPP_FMT_RGB_565:
	case MPP_FMT_BGR_565:
	case MPP_FMT_ARGB_4444:
	case MPP_FMT_ABGR_4444:
	case MPP_FMT_RGBA_4444:
	case MPP_FMT_BGRA_4444:
		return 1;
	default:
		break;
	}
	return 0;
}

static inline int need_blend(struct ge_ctrl *ctrl)
{
	if (ctrl->alpha_en || ctrl->ck_en)
		return 1;
	else
		return 0;
}

static int check_bitblt(struct mpp_ge *ge, struct ge_bitblt *blt)
{
	enum mpp_pixel_format src_format = blt->src_buf.format;
	enum mpp_pixel_format dst_format = blt->dst_buf.format;
	struct mpp_rect *src_rect = &blt->src_buf.crop;
	struct mpp_rect *dst_rect = &blt->dst_buf.crop;
	struct mpp_size *src_size = &blt->src_buf.size;
	struct mpp_size *dst_size = &blt->dst_buf.size;
	unsigned int scan_order = MPP_SCAN_ORDER_GET(blt->ctrl.flags);
	unsigned int rot0_degree = MPP_ROTATION_GET(blt->ctrl.flags);

	(void)ge;

	if (scan_order) {
		if (rot0_degree) {
			printf("scan order unsupport rot0\n");
			return -1;
		}
		if (blt->ctrl.dither_en) {
			printf("scan order unsupport dither\n");
			return -1;
		}
		if (!is_rgb(src_format) || !is_rgb(dst_format)) {
			printf("scan order just support rgb format\n");
			return -1;
		}
	}

	if (blt->ctrl.dither_en) {
		if (!is_rgb(dst_format)) {
			printf("invalid dst format with the dither func on\n");
			return -1;
		}
	}

	if (blt->src_buf.crop_en) {
		if (src_rect->x < 0 ||
		    src_rect->y < 0 ||
		    src_rect->x >= src_size->width ||
		    src_rect->y >= src_size->height) {
			printf("%s failed, invalid src crop\n", __func__);
			return -1;
		}
	}

	if (blt->dst_buf.crop_en) {
		if (dst_rect->x < 0 ||
		    dst_rect->y < 0 ||
		    dst_rect->x >= dst_size->width ||
		    dst_rect->y >= dst_size->height) {
			printf("%s failed, invalid dst crop\n", __func__);
			return -1;
		}
	}

	if (!blt->src_buf.crop_en) {
		src_rect->x = 0;
		src_rect->y = 0;
		src_rect->width = src_size->width;
		src_rect->height = src_size->height;
	}

	if (!blt->dst_buf.crop_en) {
		dst_rect->x = 0;
		dst_rect->y = 0;
		dst_rect->width = dst_size->width;
		dst_rect->height = dst_size->height;
	}

	switch (src_format) {
	case MPP_FMT_YUV420P:
	case MPP_FMT_NV12:
	case MPP_FMT_NV21:
		src_rect->x = src_rect->x & (~1);
		src_rect->y = src_rect->y & (~1);
		src_rect->width = src_rect->width & (~1);
		src_rect->height = src_rect->height & (~1);
		src_size->width = src_size->width & (~1);
		src_size->height = src_size->height & (~1);
		break;
	case MPP_FMT_YUV422P:
	case MPP_FMT_NV16:
	case MPP_FMT_NV61:
	case MPP_FMT_YUYV:
	case MPP_FMT_YVYU:
	case MPP_FMT_UYVY:
	case MPP_FMT_VYUY:
		src_rect->x = src_rect->x & (~1);
		src_rect->width = src_rect->width & (~1);
		src_size->height = src_size->height & (~1);
		break;
	default:
		break;
	}

	switch (dst_format) {
	case MPP_FMT_YUV420P:
	case MPP_FMT_NV12:
	case MPP_FMT_NV21:
		dst_rect->x = dst_rect->x & (~1);
		dst_rect->y = dst_rect->y & (~1);
		dst_rect->width = dst_rect->width & (~1);
		dst_rect->height = dst_rect->height & (~1);
		dst_size->width = dst_size->width & (~1);
		dst_size->height = dst_size->height & (~1);
		break;
	case MPP_FMT_YUV422P:
	case MPP_FMT_NV16:
	case MPP_FMT_NV61:
	case MPP_FMT_YUYV:
	case MPP_FMT_YVYU:
	case MPP_FMT_UYVY:
	case MPP_FMT_VYUY:
		dst_rect->x = dst_rect->x & (~1);
		dst_rect->width = dst_rect->width & (~1);
		dst_size->width = dst_size->width & (~1);
		break;
	default:
		break;
	}

	/* crop src invalid region */
	if ((src_rect->x + src_rect->width) > src_size->width)
		src_rect->width = src_size->width - src_rect->x;

	if ((src_rect->y + src_rect->height) > src_size->height)
		src_rect->height = src_size->height - src_rect->y;

	/* crop dst invalid region */
	if ((dst_rect->x + dst_rect->width) > dst_size->width)
		dst_rect->width = dst_size->width - dst_rect->x;

	if ((dst_rect->y + dst_rect->height) > dst_size->height)
		dst_rect->height = dst_size->height - dst_rect->y;

	if (src_rect->height > MAX_HEIGHT ||
			 src_rect->width > MAX_WIDTH) {
		printf("invalid src size, over the largest\n");
		return -1;
	}

	if (dst_rect->height > MAX_HEIGHT ||
			 dst_rect->width > MAX_WIDTH) {
		printf("invalid dst size, over the largest\n");
		return -1;
	}

	if (!is_rgb(src_format) &&
			(src_rect->width < 8 || src_rect->height < 8))  {
		printf("invalid src size, the min size of yuv is 8x8\n");
		return -1;
	}

	if (!is_rgb(dst_format) &&
			(dst_rect->width < 8 || dst_rect->height < 8))  {
		printf("invalid dst size, the min size of yuv is 8x8\n");
		return -1;
	}
	return 0;
}

static int check_fillrect(struct mpp_ge *ge,
			  struct ge_fillrect *fill)
{
	enum mpp_pixel_format dst_format = fill->dst_buf.format;
	struct mpp_rect *dst_rect = &fill->dst_buf.crop;
	struct mpp_size *dst_size = &fill->dst_buf.size;

	(void)ge;

	if (fill->dst_buf.crop_en) {
		if (dst_rect->x < 0 ||
		    dst_rect->y < 0 ||
		    dst_rect->x >= dst_size->width ||
		    dst_rect->y >= dst_size->height) {
			printf("%s failed, invalid dst crop\n", __func__);
			return -1;
		}
	}

	switch (fill->type) {
	case GE_NO_GRADIENT:
	case GE_H_LINEAR_GRADIENT:
	case GE_V_LINEAR_GRADIENT:
		break;
	default:
		printf("invalid type: %08x\n", fill->type);
		return -1;
	}

	if (!fill->dst_buf.crop_en) {
		dst_rect->x = 0;
		dst_rect->y = 0;
		dst_rect->width = dst_size->width;
		dst_rect->height = dst_size->height;
	}

	switch (dst_format) {
	case MPP_FMT_YUV420P:
	case MPP_FMT_NV12:
	case MPP_FMT_NV21:
		dst_rect->x = dst_rect->x & (~1);
		dst_rect->y = dst_rect->y & (~1);
		dst_rect->width = dst_rect->width & (~1);
		dst_rect->height = dst_rect->height & (~1);
		dst_size->width = dst_size->width & (~1);
		dst_size->height = dst_size->height & (~1);
		break;
	case MPP_FMT_YUV422P:
	case MPP_FMT_NV16:
	case MPP_FMT_NV61:
	case MPP_FMT_YUYV:
	case MPP_FMT_YVYU:
	case MPP_FMT_UYVY:
	case MPP_FMT_VYUY:
		dst_rect->x = dst_rect->x & (~1);
		dst_rect->width = dst_rect->width & (~1);
		dst_size->width = dst_size->width & (~1);
		break;
	default:
		break;
	}

	/* crop dst invalid region */
	if ((dst_rect->x + dst_rect->width) > dst_size->width)
		dst_rect->width = dst_size->width - dst_rect->x;

	if ((dst_rect->y + dst_rect->height) > dst_size->height)
		dst_rect->height = dst_size->height - dst_rect->y;

	if (dst_rect->width > MAX_WIDTH ||
		dst_rect->height > MAX_HEIGHT) {
		printf("invalid dst size, over the largest\n");
		return -1;
	}

	if (!is_rgb(dst_format) &&
			(dst_rect->width < 8 ||
			 dst_rect->height < 8)) {
		printf("invalid dst size, the min size of yuv is 8x8\n");
		return -1;
	}

	return 0;
}

static int check_format_and_size(struct mpp_ge *ge,
				 struct mpp_buf *src_buf,
				 struct mpp_buf *dst_buf)
{
	enum mpp_pixel_format src_format = src_buf->format;
	enum mpp_pixel_format dst_format = dst_buf->format;

	struct mpp_rect *src_rect = &src_buf->crop;
	struct mpp_rect *dst_rect = &dst_buf->crop;

	(void)ge;

	if (src_buf->crop_en) {
		if (src_rect->x < 0 ||
		    src_rect->y < 0 ||
		    src_rect->x >= src_buf->size.width ||
		    src_rect->y >= src_buf->size.height) {
			printf("%s failed, invalid src crop\n", __func__);
			return -1;
		}
	}

	if (dst_buf->crop_en) {
		if (dst_rect->x < 0 ||
		    dst_rect->y < 0 ||
		    dst_rect->x >= dst_buf->size.width ||
		    dst_rect->y >= dst_buf->size.height) {
			printf("%s failed, invalid dst crop\n", __func__);
			return -1;
		}
	}

	if (!src_buf->crop_en) {
		src_rect->x = 0;
		src_rect->y = 0;
		src_rect->width = src_buf->size.width;
		src_rect->height = src_buf->size.height;
	}

	if (!dst_buf->crop_en) {
		dst_rect->x = 0;
		dst_rect->y = 0;
		dst_rect->width = dst_buf->size.width;
		dst_rect->height = dst_buf->size.height;
	}

	switch (src_format) {
	case MPP_FMT_ARGB_8888:
	case MPP_FMT_ABGR_8888:
	case MPP_FMT_RGBA_8888:
	case MPP_FMT_BGRA_8888:
	case MPP_FMT_XRGB_8888:
	case MPP_FMT_XBGR_8888:
	case MPP_FMT_RGBX_8888:
	case MPP_FMT_BGRX_8888:
	case MPP_FMT_RGB_888:
	case MPP_FMT_BGR_888:
	case MPP_FMT_ARGB_1555:
	case MPP_FMT_ABGR_1555:
	case MPP_FMT_RGBA_5551:
	case MPP_FMT_BGRA_5551:
	case MPP_FMT_RGB_565:
	case MPP_FMT_BGR_565:
	case MPP_FMT_ARGB_4444:
	case MPP_FMT_ABGR_4444:
	case MPP_FMT_RGBA_4444:
	case MPP_FMT_BGRA_4444:
		break;
	default:
		printf("unsupport src format:%d\n", src_format);
		return -1;
	}

	switch (dst_format) {
	case MPP_FMT_ARGB_8888:
	case MPP_FMT_ABGR_8888:
	case MPP_FMT_RGBA_8888:
	case MPP_FMT_BGRA_8888:
	case MPP_FMT_XRGB_8888:
	case MPP_FMT_XBGR_8888:
	case MPP_FMT_RGBX_8888:
	case MPP_FMT_BGRX_8888:
	case MPP_FMT_RGB_888:
	case MPP_FMT_BGR_888:
	case MPP_FMT_ARGB_1555:
	case MPP_FMT_ABGR_1555:
	case MPP_FMT_RGBA_5551:
	case MPP_FMT_BGRA_5551:
	case MPP_FMT_RGB_565:
	case MPP_FMT_BGR_565:
	case MPP_FMT_ARGB_4444:
	case MPP_FMT_ABGR_4444:
	case MPP_FMT_RGBA_4444:
	case MPP_FMT_BGRA_4444:
		break;
	default:
		printf("unsupport dst format:%d\n", dst_format);
		return -1;
	}

	/* crop src invalid region */
	if ((src_rect->x + src_rect->width) >
	    src_buf->size.width)
		src_rect->width = src_buf->size.width -
				  src_rect->x;

	if ((src_rect->y + src_rect->height) >
	    src_buf->size.height)
		src_rect->height = src_buf->size.height -
				   src_rect->y;

	if (src_rect->width < 4 || src_rect->height < 4 ||
			src_rect->width > MAX_WIDTH ||
			src_rect->height > MAX_HEIGHT) {
		printf("unsupport src size\n");
		return -1;
	}

	/* crop dst invalid region */
	if ((dst_rect->x + dst_rect->width) >
	    dst_buf->size.width)
		dst_rect->width = dst_buf->size.width -
				  dst_rect->x;

	if ((dst_rect->y + dst_rect->height) >
	    dst_buf->size.height)
		dst_rect->height = dst_buf->size.height -
				   dst_rect->y;

	if (dst_rect->width < 4 || dst_rect->height < 4 ||
			dst_rect->width > MAX_WIDTH ||
			dst_rect->height > MAX_HEIGHT) {
		printf("unsupport dst size\n");
		return -1;
	}

	return 0;
}

static int update_scaler_cmd(struct mpp_ge *ge,
			     struct ge_bitblt *blt)
{
	enum mpp_pixel_format format;
	int in_w[2];
	int in_h[2];
	int out_w;
	int out_h;
	int channel_num;
	int scaler_en;
	int rot0_degree;
	int i;
	int dx[2];
	int dy[2];
	int h_phase[2];
	int v_phase[2];

	if (!ge)
		return -1;

	channel_num = 1;
	scaler_en = 1;
	format = blt->src_buf.format;
	rot0_degree = MPP_ROTATION_GET(blt->ctrl.flags);

	in_w[0] = blt->src_buf.crop.width;
	in_h[0] = blt->src_buf.crop.height;

	if (rot0_degree == MPP_ROTATION_90 ||
	    rot0_degree == MPP_ROTATION_270) {
		out_w = blt->dst_buf.crop.height;
		out_h = blt->dst_buf.crop.width;
	} else {
		out_w = blt->dst_buf.crop.width;
		out_h = blt->dst_buf.crop.height;
	}

	switch (format) {
	case MPP_FMT_ARGB_8888:
	case MPP_FMT_ABGR_8888:
	case MPP_FMT_RGBA_8888:
	case MPP_FMT_BGRA_8888:
	case MPP_FMT_XRGB_8888:
	case MPP_FMT_XBGR_8888:
	case MPP_FMT_RGBX_8888:
	case MPP_FMT_BGRX_8888:
	case MPP_FMT_RGB_888:
	case MPP_FMT_BGR_888:
	case MPP_FMT_ARGB_1555:
	case MPP_FMT_ABGR_1555:
	case MPP_FMT_RGBA_5551:
	case MPP_FMT_BGRA_5551:
	case MPP_FMT_RGB_565:
	case MPP_FMT_BGR_565:
	case MPP_FMT_ARGB_4444:
	case MPP_FMT_ABGR_4444:
	case MPP_FMT_RGBA_4444:
	case MPP_FMT_BGRA_4444:
		if (in_w[0] == out_w && in_h[0] == out_h)
			scaler_en = 0;
		else {
			dx[0] = (in_w[0] << 16) / out_w;
			dy[0] = (in_h[0] << 16) / out_h;
			h_phase[0] = dx[0] >> 1;
			v_phase[0] = dy[0] >> 1;
		}
		break;
	case MPP_FMT_YUV400:
		dx[0] = (in_w[0] << 16) / out_w;
		dy[0] = (in_h[0] << 16) / out_h;
		h_phase[0] = dx[0] >> 1;
		v_phase[0] = dy[0] >> 1;
		break;
	case MPP_FMT_YUV420P:
	case MPP_FMT_NV12:
	case MPP_FMT_NV21:
		channel_num = 2;
		in_w[1] = in_w[0] >> 1;
		in_h[1] = in_h[0] >> 1;

		dx[0] = (in_w[0] << 16) / out_w;
		dy[0] = (in_h[0] << 16) / out_h;
		h_phase[0] = dx[0] >> 1;
		v_phase[0] = dy[0] >> 1;

		dx[0] = dx[0] & (~1);
		dy[0] = dy[0] & (~1);
		h_phase[0] = h_phase[0] & (~1);
		v_phase[0] = v_phase[0] & (~1);

		/* change init phase */
		if (((dx[0] - h_phase[0]) >> 16) > 4) {
			h_phase[0] += (((dx[0] - h_phase[0]) >> 16) - 4) << 16;
		}

		if (((dy[0] - v_phase[0]) >> 16) > 3) {
			v_phase[0] += (((dy[0] - v_phase[0]) >> 16) - 4) << 16;
		}

		dx[1] = dx[0] >> 1;
		dy[1] = dy[0] >> 1;
		h_phase[1] = h_phase[0] >> 1;
		v_phase[1] = v_phase[0] >> 1;
		break;
	case MPP_FMT_YUV422P:
	case MPP_FMT_NV16:
	case MPP_FMT_NV61:
	case MPP_FMT_YUYV:
	case MPP_FMT_YVYU:
	case MPP_FMT_UYVY:
	case MPP_FMT_VYUY:
		channel_num = 2;

		in_w[1] = in_w[0] >> 1;
		in_h[1] = in_h[0];

		dx[0] = (in_w[0] << 16) / out_w;
		dy[0] = (in_h[0] << 16) / out_h;
		h_phase[0] = dx[0] >> 1;
		v_phase[0] = dy[0] >> 1;

		dx[0] = dx[0] & (~1);
		h_phase[0] = h_phase[0] & (~1);

		/* change init phase */
		if (((dx[0] - h_phase[0]) >> 16) > 4) {
			h_phase[0] += (((dx[0] - h_phase[0]) >> 16) - 4) << 16;
		}

		dx[1] = dx[0] >> 1;
		dy[1] = dy[0];
		h_phase[1] = h_phase[0] >> 1;
		v_phase[1] = v_phase[0];
		break;
	case MPP_FMT_YUV444P:
		channel_num = 2;
		in_w[1] = in_w[0];
		in_h[1] = in_h[0];

		dx[0] = (in_w[0] << 16) / out_w;
		dy[0] = (in_h[0] << 16) / out_h;
		h_phase[0] = dx[0] >> 1;
		v_phase[0] = dy[0] >> 1;

		dx[1] = dx[0];
		dy[1] = dy[0];
		h_phase[1] = h_phase[0];
		v_phase[1] = v_phase[0];
		break;
	default:
		scaler_en = 0;
		printf("scale invalid format: %d\n", format);
		return -1;
	}

	if (scaler_en) {
		if (MPP_SCAN_ORDER_GET(blt->ctrl.flags)) {
			printf("unsupport scan order\n");
			return -1;
		}

		if (is_rgb(format) &&
				(in_w[0] < 4 || in_h[0] < 4)) {
			printf("the min size of rgb is 4x4, when scaler enable\n");
			return -1;
		}

		if (is_rgb(blt->dst_buf.format) &&
				(out_h < 4 || out_w < 4)) {
			printf("the min size of rgb is 4x4, when scaler enable\n");
			return -1;
		}

		scaler0_en_cmd(ge, 1);
		for (i = 0; i < channel_num; i++)
			update_scaler0_cmd(ge, in_w[i], in_h[i],
					   out_w, out_h,
					   dx[i], dy[i],
					   h_phase[i], v_phase[i],
					   i);
	} else {
		scaler0_en_cmd(ge, 0);
	}

	return 0;
}

static int flush_cmd(struct cmd_queue *cmdq, int dev_fd)
{
	int ret;

	if (cmdq->write_offset) {
		ret = write(dev_fd, cmdq->cmd_buf, cmdq->write_offset * 4 );
		if (ret < 0) {
			printf( "flush_cmd: write() failed!\n" );
			return -1;
		}
		cmdq->write_offset = 0;
	}

	return 0;
}

static int clean_dma_buf_list(struct mpp_ge *ge)
{
	struct cmd_queue *cmdq;
	struct ge_dmabuf_head *node = NULL, *head = NULL;

	if (!ge)
		return -1;

	cmdq = to_cmdq(ge);
	mpp_list_for_each_entry_safe(head, node, &cmdq->dma_list, list) {
		mpp_list_del(&head->list);
		free(head);
	}

	return 0;
}

static int ge_plane_num(enum mpp_pixel_format format)
{
	switch (format) {
	case MPP_FMT_NV12:
	case MPP_FMT_NV21:
	case MPP_FMT_NV16:
	case MPP_FMT_NV61:
		return 2;
	case MPP_FMT_YUV420P:
	case MPP_FMT_YUV422P:
	case MPP_FMT_YUV444P:
		return 3;
	default:
		break;
	}

	return 1;
}

static inline int get_src_phy_addr(struct mpp_ge *ge,
				   struct mpp_buf *video_buf)
{
	struct cmd_queue *cmdq;
	int plane_num;
	int i;

	cmdq = to_cmdq(ge);

	plane_num = ge_plane_num(video_buf->format);

	if (video_buf->buf_type == MPP_DMA_BUF_FD) {
		for (i = 0; i < plane_num; i++) {
			if (dma_fd_to_addr(ge, video_buf->fd[i],
				&cmdq->data.src_phy_addr[i]) < 0) {
				printf("can't find src dma fd: %d\n", video_buf->fd[0]);
				return -1;
			}
		}
	} else {
		for (i = 0; i < plane_num; i++) {

			cmdq->data.src_phy_addr[i] = video_buf->phy_addr[i];
			printf("src_phy_addr[%d] = %x\n", i, video_buf->phy_addr[i]);

		}
	}

	return 0;
}

static inline int get_dst_phy_addr(struct mpp_ge *ge,
				   struct mpp_buf *video_buf)
{
	struct cmd_queue *cmdq;
	int plane_num;
	int i;

	cmdq = to_cmdq(ge);

	plane_num = ge_plane_num(video_buf->format);

	if (video_buf->buf_type == MPP_DMA_BUF_FD) {
		for (i = 0; i < plane_num; i++) {
			if(dma_fd_to_addr(ge, video_buf->fd[i],
				&cmdq->data.dst_phy_addr[i]) < 0) {
				printf("can't find dst dma fd: %d\n", video_buf->fd[0]);
				return -1;
			}
		}
	} else {
		for (i = 0; i < plane_num; i++) {

			cmdq->data.dst_phy_addr[i] = video_buf->phy_addr[i];
			printf("dst_phy_addr[%d] = %x\n", i, video_buf->phy_addr[i]);

		}
	}

	return 0;
}

static inline void make_cmd_buf_valid(struct cmd_queue *cmdq, int dev_fd)
{
	/* cur task need enough buf */
	if (cmdq->write_offset + VALID_SPACE > cmdq->total_size) {
		flush_cmd(cmdq,  dev_fd);
		cmdq->write_offset = 0;
	}
}

static inline unsigned int *get_cmd_buf(struct cmd_queue *cmdq, int dev_fd)
{
	// reserved space for cmd group header
	return &cmdq->cmd_buf[cmdq->write_offset + 1];
}

static inline int update_cmd_group(struct cmd_queue *cmdq,
				   unsigned int offset,
				   unsigned int length,
				   unsigned int task_end)
{
	cmdq->cmd_buf[cmdq->write_offset] = (offset << 16)
				| (length << 2) | task_end;

	/* cmd groud should be 8 bytes aligned */
	cmdq->write_offset += ALIGN_2B(length + 1);
	return 0;
}

static void set_alpha_rules_and_premul(struct ge_data *data,
				       struct ge_ctrl *ctrl,
				       enum mpp_pixel_format src_format,
				       enum mpp_pixel_format dst_format,
				       unsigned int src_buf_flags,
				       unsigned int dst_buf_flags,
				       int is_fill_color)
{
	if (ctrl->alpha_en)
		set_alpha_rules(data, ctrl->alpha_rules);

	set_premuliply(data, src_format, dst_format,
		       MPP_BUF_PREMULTIPLY_GET(src_buf_flags),
		       MPP_BUF_PREMULTIPLY_GET(dst_buf_flags),
		       is_fill_color, ctrl);
}

static void set_csc_flow(struct ge_data *data,
			 struct ge_ctrl *ctrl,
			 enum mpp_pixel_format src_format,
			 enum mpp_pixel_format dst_format)
{
	int src_is_rgb = is_rgb(src_format);
	int dst_is_rgb = is_rgb(dst_format);
	int is_blending = need_blend(ctrl);

	data->blend_is_rgb = 1;

	if (!src_is_rgb &&
	    (is_blending || dst_is_rgb)) {
		data->csc0_en = 1;
	} else if (!src_is_rgb) {
		data->blend_is_rgb = 0;
		data->csc0_en = 0;
	} else {
		data->csc0_en = 0;
	}

	if (is_blending && !dst_is_rgb)
		data->csc1_en = 1;
	else
		data->csc1_en = 0;

	if (data->blend_is_rgb && !dst_is_rgb)
		data->csc2_en = 1;
	else
		data->csc2_en = 0;
}

static void set_alpha_rules(struct ge_data *data,
			    enum ge_pd_rules rules)
{
	switch (rules) {
	case GE_PD_NONE:
		data->src_alpha_coef = 2;
		data->dst_alpha_coef = 3;
		break;
	case GE_PD_CLEAR:
		data->src_alpha_coef = 0;
		data->dst_alpha_coef = 0;
		break;
	case GE_PD_SRC:
		data->src_alpha_coef = 1;
		data->dst_alpha_coef = 0;
		break;
	case GE_PD_SRC_OVER:
		data->src_alpha_coef = 1;
		data->dst_alpha_coef = 3;
		break;
	case GE_PD_DST_OVER:
		data->src_alpha_coef = 5;
		data->dst_alpha_coef = 1;
		break;
	case GE_PD_SRC_IN:
		data->src_alpha_coef = 4;
		data->dst_alpha_coef = 0;
		break;
	case GE_PD_DST_IN:
		data->src_alpha_coef = 0;
		data->dst_alpha_coef = 2;
		break;
	case GE_PD_SRC_OUT:
		data->src_alpha_coef = 5;
		data->dst_alpha_coef = 0;
		break;
	case GE_PD_DST_OUT:
		data->src_alpha_coef = 0;
		data->dst_alpha_coef = 3;
		break;
	case GE_PD_SRC_ATOP:
		data->src_alpha_coef = 4;
		data->dst_alpha_coef = 3;
		break;
	case GE_PD_DST_ATOP:
		data->src_alpha_coef = 5;
		data->dst_alpha_coef = 2;
		break;
	case GE_PD_ADD:
		data->src_alpha_coef = 1;
		data->dst_alpha_coef = 1;
		break;
	case GE_PD_XOR:
		data->src_alpha_coef = 5;
		data->dst_alpha_coef = 3;
		break;
	case GE_PD_DST:
		data->src_alpha_coef = 0;
		data->dst_alpha_coef = 1;
		break;
	default:
		data->src_alpha_coef = 2;
		data->dst_alpha_coef = 3;
		break;
	}
}

/* must call set_premuliply after set_alpha_rules */
static int set_premuliply(struct ge_data *data,
			  enum mpp_pixel_format src_format,
			  enum mpp_pixel_format dst_format,
			  int src_premul,
			  int dst_premul,
			  int is_fill_color,
			  struct ge_ctrl *ctrl)
{
	if (src_format > MPP_FMT_BGRA_4444)
		src_premul = 0;

	if (dst_format > MPP_FMT_BGRA_4444)
		dst_premul = 0;

	if (src_premul == 0 && dst_premul == 0) {
		data->src_premul_en = 0;
		data->src_de_premul_en = 0;
		data->dst_de_premul_en = 0;
		data->out_premul_en = 0;

		if (is_fill_color == 0 &&
		    ctrl->src_alpha_mode == 0 &&
		    ctrl->alpha_en &&
		    ctrl->ck_en == 0 &&
		    data->src_alpha_coef == 2 &&
		    data->dst_alpha_coef == 3) {
			data->src_premul_en = 1;
			data->src_alpha_coef = 1;
		}
	} else if (src_premul == 1 && dst_premul == 0) {
		data->src_premul_en = 0;
		data->src_de_premul_en = 1;
		data->dst_de_premul_en = 0;
		data->out_premul_en = 0;

		if (ctrl->src_alpha_mode == 0 &&
		    ctrl->alpha_en &&
		    ctrl->ck_en == 0 &&
		    data->src_alpha_coef == 2 &&
		    data->dst_alpha_coef == 3) {
			data->src_de_premul_en = 0;
			data->src_alpha_coef = 1;
		}
	} else if (src_premul == 1 && dst_premul == 1) {
		data->src_premul_en = 0;
		data->src_de_premul_en = 1;
		data->dst_de_premul_en = 1;
		data->out_premul_en = 1;
	} else if (src_premul == 0 && dst_premul == 1) {
		data->src_premul_en = 0;
		data->src_de_premul_en = 0;
		data->dst_de_premul_en = 1;
		data->out_premul_en = 1;
	}

	return 0;
}

static inline unsigned int ge_set_src_ctrl(unsigned int global_alpha,
					   unsigned int alpha_mode,
					   unsigned int premul_en,
					   unsigned int scan_order,
					   unsigned int func_select,
					   unsigned int fmt,
					   unsigned int v_flip,
					   unsigned int h_flip,
					   unsigned int rot0_ctrl,
					   unsigned int source_mode,
					   unsigned int csc0_en)
{
	unsigned int value = SRC_SURFACE_G_ALPHA_MODE(global_alpha) |
		    SRC_SURFACE_ALPHA_MODE(alpha_mode) |
		    SRC_SURFACE_P_MUL(premul_en) |
		    SRC_SURFACE_SCAN_ORDER(scan_order) |
		    SRC_SURFACE_FUNC_SELECT(func_select) |
		    SRC_SURFACE_FORMAT(fmt) |
		    SRC_SURFACE_V_FLIP_EN(v_flip) |
		    SRC_SURFACE_H_FLIP_EN(h_flip) |
		    SRC_SURFACE_ROT0_CTRL(rot0_ctrl) |
		    SRC_SURFACE_SOURCE_MODE(source_mode) |
		    SRC_SURFACE_CSC0_EN(csc0_en) |
		    SRC_SURFACE_EN;

	return value;
}

static inline unsigned int ge_src_simple_ctrl(unsigned int global_alpha, unsigned int  alpha_mode,
			  unsigned int  premul_en, unsigned int  func_select,
			  unsigned int  fmt, unsigned int  source_mode)
{
	unsigned int value = SRC_SURFACE_G_ALPHA_MODE(global_alpha) |
		    SRC_SURFACE_ALPHA_MODE(alpha_mode) |
		    SRC_SURFACE_P_MUL(premul_en) |
		    SRC_SURFACE_FUNC_SELECT(func_select) |
		    SRC_SURFACE_FORMAT(fmt) |
		    SRC_SURFACE_SOURCE_MODE(source_mode) |
		    SRC_SURFACE_EN;

	return value;
}

static inline unsigned int ge_output_ctrl(unsigned int premul, int fmt,
			   unsigned int dither_en, unsigned int csc2_en)
{
	unsigned int value = OUTPUT_P_MUL(premul) |
		    OUTPUT_FORMAT(fmt) |
		    DITHER_EN(dither_en) |
		    OUTPUT_CSC2_EN(csc2_en);

	return value;
}

static inline unsigned int ge_dst_ctrl(unsigned int global_alpha,
		   unsigned int alpha_mode, int fmt, unsigned int csc1_en)
{
	unsigned int value = DST_SURFACE_G_ALPHA_MODE(global_alpha) |
		    DST_SURFACE_ALPHA_MODE(alpha_mode) |
		    DST_SURFACE_FORMAT(fmt) |
		    DST_SURFACE_CSC1_EN(csc1_en) |
		    DST_SURFACE_EN;

	return value;
}

void update_gradient_cmd(struct mpp_ge *ge, int width, int height,
			 unsigned start_color, unsigned end_color,
			 unsigned direction)
{
	int a_step, r_step, g_step, b_step;
	int length;
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	unsigned char start_a = (unsigned char)((start_color >> 24) & 0xff);
	unsigned char start_r = (unsigned char)((start_color >> 16) & 0xff);
	unsigned char start_g = (unsigned char)((start_color >> 8) & 0xff);
	unsigned char start_b = (unsigned char)(start_color & 0xff);

	unsigned char end_a = (unsigned char)((end_color >> 24) & 0xff);
	unsigned char end_r = (unsigned char)((end_color >> 16) & 0xff);
	unsigned char end_g = (unsigned char)((end_color >> 8) & 0xff);
	unsigned char end_b = (unsigned char)(end_color & 0xff);

	if (direction == 0)
		length = width;
	else
		length = height;

	a_step = length > 1 ?
		 ((end_a - start_a) << 16) / (length - 1) : 0;
	r_step = length > 1 ?
		 ((end_r - start_r) << 16) / (length - 1) : 0;
	g_step = length > 1 ?
		 ((end_g - start_g) << 16) / (length - 1) : 0;
	b_step = length > 1 ?
		 ((end_b - start_b) << 16) / (length - 1) : 0;

	cmd[0] = SRC_GRADIENT_STEP_SET(a_step);
	cmd[1] = SRC_GRADIENT_STEP_SET(r_step);
	cmd[3] = SRC_GRADIENT_STEP_SET(g_step);
	cmd[3] = SRC_GRADIENT_STEP_SET(b_step);

	update_cmd_group(cmdq, 0x30, 4, 0);
}

static void update_rot1_cmd(struct mpp_ge *ge,
			    int angle_sin, int angle_cos,
			    int src_center_x, int src_center_y,
			    int dst_center_x, int dst_center_y)
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);


	cmd[0] = SRC_ROT1_CENTER_SET(src_center_x, src_center_y);  //FIXME
	cmd[1] = SRC_ROT1_DEGREE_SET(angle_sin, angle_cos);
	cmd[2] = DST_ROT1_CENTER_SET(dst_center_x, dst_center_y);

	update_cmd_group(cmdq, 0x70, 3, 0);
}

/*
 * must call blend at the end, because update_cmd_group()
 * emit end task flag in this function
 */
static void update_blend_cmd(struct mpp_ge *ge,
			     unsigned int src_de_premul,
			     unsigned int dst_de_premul,
			     unsigned int alpha_ctrl,
			     unsigned int src_alpha_coef,
			     unsigned int dst_alpha_coef,
			     unsigned int ck_en,
			     unsigned int ck_value,
			     unsigned int alpha_en)
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = SRC_DE_P_MUL(src_de_premul) |
		DST_DE_P_MUL(dst_de_premul) |
		OUTPUT_ALPHA_CTRL(alpha_ctrl) |
		SRC_ALPHA_COEF(src_alpha_coef) |
		DST_ALPHA_COEF(dst_alpha_coef) |
		CK_EN(ck_en) |
		ALPHA_BLEND_EN(alpha_en);

	cmd[1] = ck_value;

	update_cmd_group(cmdq, 0x90, 2, 1);
}

static inline void update_scaler0_cmd(struct mpp_ge *ge,
				      unsigned int input_w,
				      unsigned int input_h,
				      unsigned int output_w,
				      unsigned int output_h,
				      int dx, int dy,
				      int h_phase, int v_phase,
				      unsigned int channel)
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = SCALER0_INPUT_SIZE_SET(input_w, input_h);
	cmd[1] = SCALER0_OUTPUT_SIZE_SET(output_w, output_h);
	cmd[2] = SCALER0_H_INIT_PHASE_SET(h_phase);
	cmd[3] = SCALER0_H_RATIO_SET(dx);
	cmd[4] = SCALER0_V_INIT_PHASE_SET(v_phase);
	cmd[5] = SCALER0_V_RATIO_SET(dy);

	if (channel == 0)
		update_cmd_group(cmdq, 0x210, 6, 0);
	else
		update_cmd_group(cmdq, 0x230, 6, 0);
}

static int update_fillrect_cmd(struct mpp_ge *ge,
			       struct ge_fillrect *fill)
{
	struct ge_data *data;
	struct cmd_queue *cmdq;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int dst_addr[3];
	unsigned int dst_stride[2];
	unsigned int src_ctrl;
	unsigned int dst_ctrl;
	unsigned int out_ctrl;

	cmdq = to_cmdq(ge);

	/* src cmd */
	data = &cmdq->data;
	dst_w = fill->dst_buf.crop.width;
	dst_h = fill->dst_buf.crop.height;

	src_ctrl = ge_src_simple_ctrl(fill->ctrl.src_global_alpha,
				      fill->ctrl.src_alpha_mode,
				      data->src_premul_en,
				      0, /* func_select */
				      MPP_FMT_ARGB_8888,
				      fill->type + 1); /* source_mode */

	update_src_fillrect_cmd(ge, src_ctrl,
				dst_w, dst_h,
				fill->start_color);

	/* out cmd */
	out_ctrl = ge_output_ctrl(data->out_premul_en,
				  fill->dst_buf.format,
				  fill->ctrl.dither_en,
				  data->csc2_en);

	dst_addr[0] = cmdq->data.dst_phy_addr[0];
	dst_addr[1] = cmdq->data.dst_phy_addr[1];
	dst_addr[2] = cmdq->data.dst_phy_addr[2];

	dst_stride[0] = fill->dst_buf.stride[0];
	dst_stride[1] = fill->dst_buf.stride[1];

	ge_buf_crop(dst_addr, dst_stride,
		    fill->dst_buf.format,
		    fill->dst_buf.crop.x,
		    fill->dst_buf.crop.y,
		    dst_w,
		    dst_h);

	update_output_cmd(ge, out_ctrl, dst_w, dst_h,
			  dst_stride[0], dst_stride[1],
			  dst_addr);

	/* dst cmd */
	if (need_blend(&fill->ctrl)) {
		dst_ctrl = ge_dst_ctrl(fill->ctrl.dst_global_alpha,
				       fill->ctrl.dst_alpha_mode,
				       fill->dst_buf.format,
				       data->csc1_en);

		update_dst_cmd(ge, dst_ctrl, dst_w, dst_h,
			       dst_stride[0], dst_stride[1],
			       dst_addr);
	} else {
		update_dst_disable_cmd(ge);
	}

	return 0;
}

static int update_basic_cmd(struct mpp_ge  *ge,
			    struct mpp_buf *src_buf,
			    struct mpp_buf *dst_buf,
			    struct ge_ctrl *ctrl)
{
	struct ge_data *data;
	struct cmd_queue *cmdq;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int src_addr[3];
	unsigned int dst_addr[3];
	unsigned int src_stride[2];
	unsigned int dst_stride[2];
	unsigned int src_ctrl;
	unsigned int dst_ctrl;
	unsigned int out_ctrl;
	struct mpp_rect *src_rect;
	struct mpp_rect *dst_rect;

	cmdq = to_cmdq(ge);
	data = &cmdq->data;
	src_rect = &src_buf->crop;
	dst_rect = &dst_buf->crop;
	src_w = src_rect->width;
	src_h = src_rect->height;
	dst_w = dst_rect->width;
	dst_h = dst_rect->height;

	src_addr[0] = data->src_phy_addr[0];
	src_addr[1] = data->src_phy_addr[1];
	src_addr[2] = data->src_phy_addr[2];

	dst_addr[0] = data->dst_phy_addr[0];
	dst_addr[1] = data->dst_phy_addr[1];
	dst_addr[2] = data->dst_phy_addr[2];

	src_stride[0] = src_buf->stride[0];
	src_stride[1] = src_buf->stride[1];
	dst_stride[0] = dst_buf->stride[0];
	dst_stride[1] = dst_buf->stride[1];

	src_ctrl = data->src_ctrl;
	out_ctrl = data->out_ctrl;

	printf("%s(), src_addr = %x\n", __func__, src_addr[0]);

	ge_buf_crop(src_addr, src_stride,
		    src_buf->format,
		    src_rect->x,
		    src_rect->y,
		    src_w,
		    src_h);

	ge_buf_crop(dst_addr, dst_stride,
		    dst_buf->format,
		    dst_rect->x,
		    dst_rect->y,
		    dst_w,
		    dst_h);

	/* src cmd */
	update_src_cmd(ge, src_ctrl, src_w, src_h,
		       src_stride[0], src_stride[1],
		       src_addr);

	/* out cmd */
	update_output_cmd(ge, out_ctrl, dst_w, dst_h,
			  dst_stride[0], dst_stride[1],
			  dst_addr);

	/* dst cmd */
	if (need_blend(ctrl)) {
		dst_ctrl = ge_dst_ctrl(ctrl->dst_global_alpha,
				       ctrl->dst_alpha_mode,
				       dst_buf->format,
				       data->csc1_en);

		update_dst_cmd(ge, dst_ctrl, dst_w, dst_h,
			       dst_stride[0], dst_stride[1],
			       dst_addr);
	} else {
		update_dst_disable_cmd(ge);
	}

	return 0;
}

static inline void ge_dst_disable(struct mpp_ge *ge)
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = 0;
	update_cmd_group(cmdq, 0x50, 1, 0);
}

static void scaler0_en_cmd(struct mpp_ge *ge, unsigned int enable)
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = enable;
	update_cmd_group(cmdq, 0x200, 1, 0);
}

/*
 * csc:
 * 0: csc0
 * 1: csc1
 */
static void update_csc_cmd(struct mpp_ge *ge, int color_space,
			   unsigned int csc)
{
	const unsigned int *coefs;
	int i;
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	switch (color_space) {
	case MPP_COLOR_SPACE_BT601:
		coefs = yuv2rgb_bt601;
		break;
	case MPP_COLOR_SPACE_BT709:
		coefs = yuv2rgb_bt709;
		break;
	case MPP_COLOR_SPACE_BT601_FULL_RANGE:
		coefs = yuv2rgb_bt601_full;
		break;
	case MPP_COLOR_SPACE_BT709_FULL_RANGE:
		coefs = yuv2rgb_bt709_full;
		break;
	default:
		coefs = yuv2rgb_bt601;
		break;
	}

	for (i = 0; i < CSC_COEFFS_NUM; i++)
		cmd[i] = coefs[i];

	if (csc == 0)
		update_cmd_group(cmdq, 0x140, 12, 0);
	else
		update_cmd_group(cmdq, 0x170, 12, 0);

}

static void update_csc2_cmd(struct mpp_ge *ge, int color_space)
{
	const int *coefs;
	int i;
	struct cmd_queue *cmdq = to_cmdq(ge);

	/* get cmd buf */
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	switch (color_space) {
	case MPP_COLOR_SPACE_BT601:
		coefs = rgb2yuv_bt601;
		break;
	case MPP_COLOR_SPACE_BT709:
		coefs = rgb2yuv_bt709;
		break;
	case MPP_COLOR_SPACE_BT601_FULL_RANGE:
		coefs = rgb2yuv_bt601_full;
		break;
	case MPP_COLOR_SPACE_BT709_FULL_RANGE:
		coefs = rgb2yuv_bt709_full;
		break;
	default:
		coefs = rgb2yuv_bt601;
		break;
	}

	for (i = 0; i < CSC_COEFFS_NUM; i++)
		cmd[i] = coefs[i];

	update_cmd_group(cmdq, 0x1a0, 12, 0);
}

static void update_src_cmd(struct mpp_ge *ge, unsigned int src_ctrl,
			   unsigned int w, unsigned int h,
			   unsigned int stride0, unsigned int stride1,
			   unsigned int addr[])
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = src_ctrl;
	cmd[1] =  SRC_INPUT_SIZE_SET(w, h);
	cmd[2] =  SRC_STRIDE_SET(stride0, stride1);
        cmd[3] = 0;
	cmd[4] = addr[0];
	cmd[5] = addr[1];
	cmd[6] = addr[2];

	update_cmd_group(cmdq, 0x10, 7, 0);
}

static void update_src_fillrect_cmd(struct mpp_ge *ge, unsigned int src_ctrl,
				    unsigned int  w, unsigned int  h,
				    unsigned int start_color)
{
	struct cmd_queue *cmdq = to_cmdq(ge);

	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = src_ctrl;
	cmd[1] = SRC_INPUT_SIZE_SET(w, h);
	cmd[2] = 0;
	cmd[3] = start_color;

	update_cmd_group(cmdq, 0x10, 4, 0);
}

static void update_dst_cmd(struct mpp_ge *ge, unsigned int dst_ctrl,
			   unsigned int w, unsigned int h,
			   unsigned int stride0, unsigned int stride1,
			   unsigned int addr[])
{
	struct cmd_queue *cmdq = to_cmdq(ge);
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = dst_ctrl;
	cmd[1] =  DST_INPUT_SIZE_SET(w, h);
	cmd[2] =  DST_STRIDE_SET(stride0, stride1);
	cmd[3] = 0;
	cmd[4] = addr[0];
	cmd[5] = addr[1];
	cmd[6] = addr[2];

	update_cmd_group(cmdq, 0x50, 7, 0);
}

static void update_dst_disable_cmd(struct mpp_ge *ge)
{
	struct cmd_queue *cmdq = to_cmdq(ge);
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = 0;
	update_cmd_group(cmdq, 0x50, 1, 0);
}

static void update_output_cmd(struct mpp_ge *ge, unsigned int out_ctrl,
			      unsigned int w, unsigned int h,
			      unsigned int stride0, unsigned int stride1,
			      unsigned int addr[])
{
	struct cmd_queue *cmdq = to_cmdq(ge);
	unsigned int *cmd = get_cmd_buf(cmdq, ge->dev_fd);

	cmd[0] = out_ctrl;
	cmd[1] = OUTPUT_SIZE_SET(w, h);
	cmd[2] = OUTPUT_STRIDE_SET(stride0, stride1);
	cmd[3] = 0;
	cmd[4] = addr[0];
	cmd[5] = addr[1];
	cmd[6] = addr[2];

	update_cmd_group(cmdq, 0x100, 7, 0);
}

static int ge_cmd_buf_size(struct mpp_ge *ge, unsigned int *buf_size)
{
	int ret;

	ret = ioctl(ge->dev_fd, IOC_GE_CMD_BUF_SIZE, buf_size);
	if (ret < 0) {
		printf("ge_cmd_buf_size failed %d\n", ret);
		return -1;
	}

	return 0;
}

static int ge_open(struct mpp_ge *ge)
{
	struct cmd_queue *cmdq;

	if (!ge)
		return -1;

	cmdq = (struct cmd_queue *)malloc(sizeof(struct cmd_queue));

	if (!cmdq) {
		printf("cmdq malloc failed!\n");
		return -1;
	}

	memset(cmdq, 0, sizeof(struct cmd_queue));
	ge->priv = cmdq;

	if (ge_cmd_buf_size(ge, &cmdq->total_size) < 0)
		cmdq->total_size = ALIGN_8B(DEFAULT_CMD_SIZE);

	cmdq->cmd_buf = (unsigned int *)malloc(cmdq->total_size);
	if (!cmdq->cmd_buf) {
		printf("cmd_buf malloc failed!\n");
		free(cmdq);
		return -1;
	}

	printf("info: cmd ring buf size:%u\n", cmdq->total_size);
	memset(cmdq->cmd_buf, 0, cmdq->total_size);

	/* init dma info lists */
	mpp_list_init(&cmdq->dma_list);
	return 0;
}

static int ge_close(struct mpp_ge *ge)
{
	struct cmd_queue *cmdq;

	if (!ge)
		return -1;

	cmdq = to_cmdq(ge);

	if (cmdq) {
		clean_dma_buf_list(ge);
		if (cmdq->cmd_buf)
			free(cmdq->cmd_buf);

		free(cmdq);
	}

	return 0;
}

static int ge_fillrect(struct mpp_ge *ge, struct ge_fillrect *fill)
{
	int ret = 0;
	int en_alpha_out_oxff;
	struct cmd_queue *cmdq;
	struct ge_data *data;
	enum mpp_pixel_format src_fmt;

	if (!ge)
		return -1;

	cmdq = to_cmdq(ge);
	data = &cmdq->data;
	src_fmt = MPP_FMT_ARGB_8888;

	if (check_fillrect(ge, fill) < 0)
		return -1;

	if (get_dst_phy_addr(ge, &fill->dst_buf) < 0)
		return -1;

	make_cmd_buf_valid(cmdq, ge->dev_fd);
	set_alpha_rules_and_premul(data, &fill->ctrl,
				   src_fmt, fill->dst_buf.format,
				   0, fill->dst_buf.flags,
				   1);
	set_csc_flow(data, &fill->ctrl,
		     src_fmt, fill->dst_buf.format);

	/* csc1 cmd */
	if (data->csc1_en)
		update_csc_cmd(ge,
			       MPP_BUF_COLOR_SPACE_GET(fill->dst_buf.flags),
			       1);

	/* csc2 cmd */
	if (data->csc2_en)
		update_csc2_cmd(ge,
				MPP_BUF_COLOR_SPACE_GET(fill->dst_buf.flags));

	switch (fill->type) {
	case GE_NO_GRADIENT:
		break;
	case GE_H_LINEAR_GRADIENT:
		update_gradient_cmd(ge,
				    fill->dst_buf.crop.width,
				    fill->dst_buf.crop.height,
				    fill->start_color,
				    fill->end_color,
				    0);
		break;
	case GE_V_LINEAR_GRADIENT:
		update_gradient_cmd(ge,
				    fill->dst_buf.crop.width,
				    fill->dst_buf.crop.height,
				    fill->start_color,
				    fill->end_color,
				    1);
		break;
	default:
		break;
	}

	update_fillrect_cmd(ge, fill);

	if (need_blend(&fill->ctrl))
		en_alpha_out_oxff = 1;
	else
		en_alpha_out_oxff = 0;

	scaler0_en_cmd(ge, 0);

	/* must call update_blend_cmd at the end */
	update_blend_cmd(ge,
			 data->src_de_premul_en,
			 data->dst_de_premul_en,
			 en_alpha_out_oxff,
			 data->src_alpha_coef,
			 data->dst_alpha_coef,
			 fill->ctrl.ck_en,
			 fill->ctrl.ck_value,
			 fill->ctrl.alpha_en);

	return ret;
}

static int ge_bitblt(struct mpp_ge *ge, struct ge_bitblt *blt)
{
	int en_alpha_out_oxff;
	struct cmd_queue *cmdq;
	struct ge_data *data;

	if (!ge)
		return -1;

	cmdq = to_cmdq(ge);
	data = &cmdq->data;

	if (check_bitblt(ge, blt) != 0)
		return -1;

	if (get_src_phy_addr(ge, &blt->src_buf) < 0 ||
	    get_dst_phy_addr(ge, &blt->dst_buf) < 0) {
		return -1;
	}

	make_cmd_buf_valid(cmdq, ge->dev_fd);
	set_alpha_rules_and_premul(data, &blt->ctrl,
				   blt->src_buf.format, blt->dst_buf.format,
				   blt->src_buf.flags, blt->dst_buf.flags,
				   0);
	set_csc_flow(data, &blt->ctrl,
		     blt->src_buf.format, blt->dst_buf.format);

	/* csc0 cmd */
	if (data->csc0_en)
		update_csc_cmd(ge,
			       MPP_BUF_COLOR_SPACE_GET(blt->src_buf.flags),
			       0);

	/* csc1 cmd */
	if (data->csc1_en)
		update_csc_cmd(ge,
			       MPP_BUF_COLOR_SPACE_GET(blt->dst_buf.flags),
			       1);

	/* csc2 cmd */
	if (data->csc2_en)
		update_csc2_cmd(ge,
				MPP_BUF_COLOR_SPACE_GET(blt->dst_buf.flags));

	data->src_ctrl = ge_set_src_ctrl(blt->ctrl.src_global_alpha,
					 blt->ctrl.src_alpha_mode,
					 data->src_premul_en,
					 MPP_SCAN_ORDER_GET(blt->ctrl.flags),
					 0, /* func_select */
					 blt->src_buf.format,
					 MPP_FLIP_V_GET(blt->ctrl.flags),
					 MPP_FLIP_H_GET(blt->ctrl.flags),
					 MPP_ROTATION_GET(blt->ctrl.flags),
					 0, /* fill buffer mode */
					 data->csc0_en);

	data->out_ctrl = ge_output_ctrl(data->out_premul_en,
					blt->dst_buf.format,
					blt->ctrl.dither_en,
					data->csc2_en);

	update_basic_cmd(ge, &blt->src_buf, &blt->dst_buf, &blt->ctrl);

	if (update_scaler_cmd(ge, blt) < 0)
		return -1;

	if (need_blend(&blt->ctrl))
		en_alpha_out_oxff = 1;
	else
		en_alpha_out_oxff = 0;

	/* must call update_blend_cmd at the end */
	update_blend_cmd(ge,
			 data->src_de_premul_en,
			 data->dst_de_premul_en,
			 en_alpha_out_oxff,
			 data->src_alpha_coef,
			 data->dst_alpha_coef,
			 blt->ctrl.ck_en,
			 blt->ctrl.ck_value,
			 blt->ctrl.alpha_en);
	return 0;
}

static int ge_rotate(struct mpp_ge *ge, struct ge_rotation *rot)
{
	int en_alpha_out_oxff;
	struct cmd_queue *cmdq;
	struct ge_data *data;

	if(!ge)
		return -1;

	cmdq = to_cmdq(ge);
	data = &cmdq->data;

	if (check_format_and_size(ge, &rot->src_buf, &rot->dst_buf)!= 0)
		return -1;

	if (get_src_phy_addr(ge, &rot->src_buf) < 0 ||
	    get_dst_phy_addr(ge, &rot->dst_buf) < 0) {
		return -1;
	}

	make_cmd_buf_valid(cmdq, ge->dev_fd);
	set_alpha_rules_and_premul(data, &rot->ctrl,
				   rot->src_buf.format, rot->dst_buf.format,
				   rot->src_buf.flags, rot->dst_buf.flags,
				   0);

	/* rot1 only support rgb format */
	data->csc0_en = 0;
	data->csc1_en = 0;
	data->csc2_en = 0;

	if(rot->ctrl.ck_en) {
		printf("warning: rot does't support color key\n");

		/* color must disable */
		rot->ctrl.ck_en = 0;
	}

	data->src_ctrl = ge_src_simple_ctrl(rot->ctrl.src_global_alpha,
					    rot->ctrl.src_alpha_mode,
					    data->src_premul_en,
					    1, /* rot1 */
					    rot->src_buf.format,
					    0); /* fill buffer mode */

	data->out_ctrl = ge_output_ctrl(data->out_premul_en,
					rot->dst_buf.format,
					0, /* rot1 does't support dither */
					data->csc2_en);

	update_basic_cmd(ge, &rot->src_buf, &rot->dst_buf, &rot->ctrl);

	update_rot1_cmd(ge, rot->angle_sin, rot->angle_cos,
			rot->src_rot_center.x, rot->src_rot_center.y,
			rot->dst_rot_center.x, rot->dst_rot_center.y);

	if (need_blend(&rot->ctrl))
		en_alpha_out_oxff = 1;
	else
		en_alpha_out_oxff = 0;

	scaler0_en_cmd(ge, 0);

	/* must call update_blend_cmd at the end */
	update_blend_cmd(ge,
			 data->src_de_premul_en,
			 data->dst_de_premul_en,
			 en_alpha_out_oxff,
			 data->src_alpha_coef,
			 data->dst_alpha_coef,
			 rot->ctrl.ck_en,
			 rot->ctrl.ck_value,
			 rot->ctrl.alpha_en);

	return 0;
}

static int ge_emit(struct mpp_ge *ge)
{
	int ret;

	if (!ge)
		return -1;

	ret = flush_cmd(to_cmdq(ge), ge->dev_fd);

	if (ret < 0)
		return ret;

	return ret;
}

static int ge_sync(struct mpp_ge *ge)
{
	int ret;

	if (!ge)
		return -1;

	ret = ioctl(ge->dev_fd, IOC_GE_SYNC);
	if (ret < 0) {
		printf("ge_sync failed %d\n", ret);
		return -1;
	}
	return 0;
}

struct ge_ops ge_cmdq_ops = {
	.name           = "cmd queue",
	.open           = ge_open,
	.close          = ge_close,
	.add_dma_buf    = ge_add_dma_buf,
	.rm_dma_buf     = ge_rm_dma_buf,
	.fillrect       = ge_fillrect,
	.bitblt         = ge_bitblt,
	.rotate         = ge_rotate,
	.emit           = ge_emit,
	.sync           = ge_sync,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
