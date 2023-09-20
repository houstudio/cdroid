/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2020-2021 ArtinChip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#ifndef _GE_REG_H_
#define _GE_REG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __KERNEL__
#define GENMASK(h,l) (((~0U) << (l)) & (~0U >>(32 - 1 - (h))))
#define BIT(nr)  (1U << (nr))
#else
#include <linux/bits.h>
#include <linux/io.h>
#include <linux/types.h>
#endif /* __KERNEL__ */

#define GE_CTRL_HW_ERR_IRQ_EN                 BIT(1)
#define GE_CTRL_FINISH_IRQ_EN                 BIT(0)

#define GE_STATUS_HW_ERR_IRQ                  BIT(1)
#define GE_STATUS_FINISH_IRQ                  BIT(0)

#define TIMEOUT_EN                            BIT(3)
#define GE_SW_RESET                           BIT(2)
#define GE_CMD_EN                             BIT(1)
#define GE_START_EN                           BIT(0)

#define SRC_SURFACE_G_ALPHA_MASK              GENMASK(31, 24)
#define SRC_SURFACE_G_ALPHA_MODE(x)           (((x) & 0xff) << 24)

#define SRC_SURFACE_ALPHA_MODE_MASK           GENMASK(23, 22)
#define SRC_SURFACE_ALPHA_MODE(x)             (((x) & 0x3) << 22)

#define SRC_SURFACE_P_MUL(x)                  (((x) & 0x1) << 21)

#define SRC_SURFACE_SCAN_ORDER_MASK          GENMASK(19, 18)
#define SRC_SURFACE_SCAN_ORDER(x)            (((x) & 0x3) << 18)

#define SRC_SURFACE_FUNC_SELECT_MASK          GENMASK(17, 16)
#define SRC_SURFACE_FUNC_SELECT(x)            (((x) & 0x3) << 16)

#define SRC_SURFACE_FORMAT_MASK               GENMASK(14, 8)
#define SRC_SURFACE_FORMAT(x)                 (((x) & 0x07f) << 8)

#define SRC_SURFACE_V_FLIP_EN(x)              (((x) & 0x1) << 7)
#define SRC_SURFACE_H_FLIP_EN(x)              (((x) & 0x1) << 6)

#define SRC_SURFACE_ROT0_CTRL_MASK            GENMASK(5, 4)
#define SRC_SURFACE_ROT0_CTRL(x)              (((x) & 0x3) << 4)

#define SRC_SURFACE_SCAN_ORDER_MODE(x)        (((x) & 0x3) << 18)
#define SRC_SURFACE_SOURCE_MODE(x)            (((x) & 0x3) << 2)
#define SRC_SURFACE_CSC0_EN(x)                (((x) & 0x1) << 1)
#define SRC_SURFACE_EN                        BIT(0)

#define SRC_GRADIENT_STEP_SET(x)              (((x) & 0x1ffffff) << 0)

#define SRC_INPUT_SIZE_SET(w, h)              ((((h) & 0x1fff) << 16) \
					      | (((w) & 0x1fff) << 0))

#define SRC_STRIDE_SET(p0, p1)                ((((p1) & 0xffff) << 16) \
					       | (((p0) & 0xffff) << 0))

#define SRC_ROT1_CENTER_SET(x, y)             ((((y) & 0x3fff) << 16) \
					       | (((x) & 0x3fff) << 0))

#define SRC_ROT1_DEGREE_SET(sin, cos)         ((((cos) & 0x3fff) << 16) \
					       | (((sin) & 0x3fff) << 0))

#define SRC_SHEAR_SET(x)                      ((x) & 0xfffff)

#define DST_SURFACE_G_ALPHA_MASK              GENMASK(31, 24)
#define DST_SURFACE_G_ALPHA_MODE(x)           (((x) & 0xff) << 24)

#define DST_SURFACE_ALPHA_MODE_MASK           GENMASK(23, 22)
#define DST_SURFACE_ALPHA_MODE(x)             (((x) & 0x3) << 22)

#define DST_SURFACE_P_MUL                     BIT(21)

#define DST_SURFACE_FORMAT_MASK               GENMASK(14, 8)
#define DST_SURFACE_FORMAT(x)                 (((x) & 0x07f) << 8)

#define DST_SURFACE_CSC1_EN(x)                (((x) & 1) << 1)
#define DST_SURFACE_EN                        BIT(0)

#define DST_INPUT_SIZE_SET(w, h)              ((((h) & 0x1fff) << 16) \
					       | (((w) & 0x1fff) << 0))

#define DST_STRIDE_SET(p0, p1)                ((((p1) & 0xffff) << 16) \
					       | (((p0) & 0xffff) << 0))

#define DST_TILE_OFFSET_SET(x, y)             ((((y) & 0x1f) << 16) \
					       | (((x) & 0x1f) << 0))

#define DST_ROT1_CENTER_SET(x, y)             ((((y) & 0x3fff) << 16) \
					       | (((x) & 0x3fff) << 0))

#define DST_SHEAR_OFFSET_SET(x, y)            ((((y) & 0x3fff) << 16) \
					       | (((x) & 0x3fff) << 0))

#define SRC_DE_P_MUL(x)                         (((x) & 0x1) << 17)
#define DST_DE_P_MUL(x)                         (((x) & 0x1) << 16)
#define OUTPUT_ALPHA_CTRL(x)                    (((x) & 0x1) << 15)

#define SRC_ALPHA_COEF_MASK                   GENMASK(13, 11)
#define SRC_ALPHA_COEF(x)                     (((x) & 0x7) << 11)

#define DST_ALPHA_COEF_MASK                   GENMASK(10, 8)
#define DST_ALPHA_COEF(x)                     (((x) & 0x7) << 8)

#define CK_EN(x)                              (((x) & 0x1) << 1)
#define ALPHA_BLEND_EN(x)                     (((x) & 0x1) << 0)

#define CK_MATCH_COLOR(x)                     ((x) & 0xffffff)

#define OUTPUT_P_MUL(x)                       (((x) & 1) << 16)

#define OUTPUT_FORMAT_MASK                    GENMASK(14, 8)
#define OUTPUT_FORMAT(x)                      (((x) & 0x07f) << 8)

#define DITHER_EN(x)                           (((x) & 1) << 4)
#define OUTPUT_CSC2_EN(x)                      (((x) & 1) << 1)

#define OUTPUT_SIZE_SET(w, h)                 ((((h) & 0x1fff) << 16) \
					       | (((w) & 0x1fff) << 0))

#define OUTPUT_STRIDE_SET(p0, p1)                ((((p1) & 0xffff) << 16) \
					       | (((p0) & 0xffff) << 0))

#define OUTPUT_TILE_OFFSET_SET(x, y)             ((((y) & 0x1f) << 16) \
					       | (((x) & 0x1f) << 0))

#define	RAND_DITHER_EN                        BIT(31)
#define	RAND_DITHER_SEED(x)                   ((x) & 0xffffff)
#define	DITHER_MASK_BITS(x)                   ((x) & 0xffffff)

#define SCALER0_CTRL_CH1_V_COEF_LUT_EN        BIT(7)
#define SCALER0_CTRL_CH1_H_COEF_LUT_EN        BIT(6)
#define SCALER0_CTRL_CH0_V_COEF_LUT_EN        BIT(5)
#define SCALER0_CTRL_CH0_H_COEF_LUT_EN        BIT(4)
#define SCALER0_CTRL_BILINEAR_SELECT          BIT(2)
#define SCALER0_CTRL_EN                       BIT(0)

#define SCALER0_INPUT_SIZE_SET(w, h)          ((((h) & 0x1fff) << 16) \
					       | (((w) & 0x1fff) << 0))

#define SCALER0_OUTPUT_SIZE_SET(w, h)         ((((h) & 0x1fff) << 16) \
					       | (((w) & 0x1fff) << 0))

#define SCALER0_H_INIT_PHASE_SET(x)           ((x) & 0xfffff)
#define SCALER0_H_RATIO_SET(x)                ((x) & 0x1fffff)
#define SCALER0_V_INIT_PHASE_SET(x)           ((x) & 0xfffff)
#define SCALER0_V_RATIO_SET(x)                ((x) & 0x1fffff)

/* base offset */
#define G_BASE                       0x000
#define SRC_BASE                     0x010
#define DST_BASE                     0x050
#define ROT1_BASE                    0x070
#define BLENDING_BASE                0x090
#define OUTPUT_BASE                  0x100
#define CSC0_BASE                    0x140
#define CSC1_BASE                    0x170
#define CSC2_BASE                    0x1a0
#define SCALER0_BASE                 0x200

/* global control */
#define GE_CTRL                      (G_BASE + 0x000)
#define GE_STATUS                    (G_BASE + 0x004)
#define GE_START                     (G_BASE + 0x008)
#define GE_VERSION_ID                (G_BASE + 0x00C)

/* src surface control */
#define SRC_SURFACE_CTRL             (SRC_BASE + 0x000)
#define SRC_SURFACE_INPUT_SIZE       (SRC_BASE + 0x004)
#define SRC_SURFACE_STRIDE           (SRC_BASE + 0x008)
#define SRC_FILL_COLOER              (SRC_BASE + 0x00C)
#define SRC_SURFACE_ADDR0            (SRC_BASE + 0x010)
#define SRC_SURFACE_ADDR1            (SRC_BASE + 0x014)
#define SRC_SURFACE_ADDR2            (SRC_BASE + 0x018)
#define SRC_GRADIENT_A_STEP          (SRC_BASE + 0x020)
#define SRC_GRADIENT_R_STEP          (SRC_BASE + 0x024)
#define SRC_GRADIENT_G_STEP          (SRC_BASE + 0x028)
#define SRC_GRADIENT_B_STEP          (SRC_BASE + 0x02C)

/* dst surface control */
#define DST_SURFACE_CTRL             (DST_BASE + 0x000)
#define DST_SURFACE_INPUT_SIZE       (DST_BASE + 0x004)
#define DST_SURFACE_STRIDE           (DST_BASE + 0x008)
#define DST_SURFACE_ADDR0            (DST_BASE + 0x010)
#define DST_SURFACE_ADDR1            (DST_BASE + 0x014)
#define DST_SURFACE_ADDR2            (DST_BASE + 0x018)

#define SRC_ROT1_CENTER              (ROT1_BASE + 0x000)
#define SRC_ROT1_DEGREE              (ROT1_BASE + 0x004)
#define DST_ROT1_CENTER              (ROT1_BASE + 0x008)

// ignore shear , not support
#define SRC_SHEAR_DEGREE             (SRC_BASE + 0x038)
#define DST_SHEAR_OFFSET             (DST_BASE + 0x034)

/* blending control */
#define BLENDING_CTRL                (BLENDING_BASE + 0x000)
#define COLORKEY_MATCH_COLOR         (BLENDING_BASE + 0x004)

/* output control */
#define OUTPUT_CTRL                  (OUTPUT_BASE + 0x000)
#define OUTPUT_SIZE                  (OUTPUT_BASE + 0x004)
#define OUTPUT_STRIDE                (OUTPUT_BASE + 0x008)

#define OUTPUT_ADDR0                 (OUTPUT_BASE + 0x010)
#define OUTPUT_ADDR1                 (OUTPUT_BASE + 0x014)
#define OUTPUT_ADDR2                 (OUTPUT_BASE + 0x018)

#define CSC2_COEF_SET(x)             ((x) & 0x7ff)
#define DITHER_BGN_ADDR              (0x120)

#define CMD_BUF_START_ADDR           (0x130)
#define CMD_BUF_END_ADDR             (0x134)
#define CMD_BUF_ADDR_OFFSET          (0x138)
#define CMD_BUF_VALID_LENGTH         (0x13c)

/* n = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 */
#define CSC0_COEF(n)                 (CSC0_BASE + 0x4 * (n))

/* n = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 */
#define CSC1_COEF(n)                 (CSC1_BASE + 0x4 * (n))

/* n = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 */
#define CSC2_COEF(n)                 (CSC2_BASE + 0x4 * (n))

/* scaler0 control */
#define SCALER0_CTRL                 (SCALER0_BASE + 0x000)

/* ch = 0, 1 */
#define SCALER0_INPUT_SIZE(ch)       (SCALER0_BASE + 0x010 + 0x20 * (ch))
#define SCALER0_OUTPUT_SIZE(ch)      (SCALER0_BASE + 0x014 + 0x20 * (ch))
#define SCALER0_H_INIT_PHASE(ch)     (SCALER0_BASE + 0x018 + 0x20 * (ch))
#define SCALER0_H_RATIO(ch)          (SCALER0_BASE + 0x01c + 0x20 * (ch))
#define SCALER0_V_INIT_PHASE(ch)     (SCALER0_BASE + 0x020 + 0x20 * (ch))
#define SCALER0_V_RATIO(ch)          (SCALER0_BASE + 0x024 + 0x20 * (ch))

/* n = 0 ~ 47 */
#define SCALER0_CH0_H_COEF(n)        (0x400 + 4 * (n))
#define SCALER0_CH0_V_COEF(n)        (0x500 + 4 * (n))
#define SCALER0_CH1_H_COEF(n)        (0x600 + 4 * (n))
#define SCALER0_CH1_V_COEF(n)        (0x700 + 4 * (n))

#endif /*_GE_REG_H_ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

