/*
 * Copyright (C) 2020-2021 Artinchip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#ifndef _MPP_GE_H_
#define _MPP_GE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mpp_types.h>
#include <artinchip_ge.h>

struct mpp_ge;

struct mpp_ge *mpp_ge_open();

void mpp_ge_close(struct mpp_ge *ge);

enum ge_mode mpp_ge_get_mode(struct mpp_ge *ge);

int mpp_ge_add_dmabuf(struct mpp_ge *ge, int dma_fd);

int mpp_ge_rm_dmabuf(struct mpp_ge *ge, int dma_fd);

int mpp_ge_fillrect(struct mpp_ge *ge, struct ge_fillrect *fillrect);

int mpp_ge_bitblt(struct mpp_ge *ge, struct ge_bitblt *blt);

int mpp_ge_rotate(struct mpp_ge *ge, struct ge_rotation *rot);

int mpp_ge_emit(struct mpp_ge *ge);

int mpp_ge_sync(struct mpp_ge *ge);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MPP_GE_H_ */
