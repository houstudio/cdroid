#ifndef __LIBHARDWARE2_IPU_H__
#define __LIBHARDWARE2_IPU_H__

#define IPU_CMD_OSD_CH0                 (0)
#define IPU_CMD_OSD_CH1                 (1)
#define IPU_CMD_OSD_CH2                 (2)
#define IPU_CMD_OSD_CH3                 (3)

#define IPU_OSD_CH_INDEX_MIN            (0)
#define IPU_OSD_CH_INDEX_MAX            (3)

#define IPU_CMD_CSC_FLAG                (1<<4)
#define IPU_CMD_OSD_FLAG                (0xf)

#define IPU_OSD_CHX_PARA_CSCCTL_SET(p, x)     ((p)=(((p)&(~(0x3<<24)))|((x&0x3)<<24)))
#define IPU_OSD_CHX_PARA_ALPHA_SET(p, x)      ((p)=(((p)&(~(0xff<<3)))|((x&0xff)<<3)))
#define IPU_OSD_CHX_PARA_PICTYPE_SET(p, x)    ((p)=(((p)&(~(0x7<<11)))|((x&0x7)<<11)))
#define IPU_OSD_CHX_PARA_ARGBTYPE_SET(p, x)   ((p)=(((p)&(~(0xf<<14)))|((x&0xf)<<14)))

typedef struct {
    unsigned int osd_chx_fmt;           /* osd chx input image format */
    unsigned int osd_chx_para;          /* OSD_CHX_PARA reg value*/
    unsigned int osd_chx_bak_argb;      /* OSD_CHX_BAK_ARGB reg value */
    unsigned int osd_chx_pos_x;         /* osd chx input image start_point_x */
    unsigned int osd_chx_pos_y;         /* osd chx input image start_point_y */
    unsigned int osd_chx_src_w;         /* osd chx input image's width */
    unsigned int osd_chx_src_h;         /* osd chx input image's height */
    unsigned int osd_chx_buf_phy;       /* osd chx input image's physical address must be continuity*/
}ipu_osdx_param_t;

typedef struct {
    unsigned int cmd;                   /* IPU command */

    unsigned int bg_w;                  /* background weight */
    unsigned int bg_h;                  /* background hight */
    unsigned int bg_fmt;                /* background format */
    unsigned int bg_buf_phy;            /* background buffer physical addr */

    unsigned int out_fmt;               /* out format */

    ipu_osdx_param_t ipu_osdx_param[IPU_OSD_CH_INDEX_MAX+1];
}ipu_param_t;

/**
 * @brief 获得ipu设备句柄
 * @return 成功返回设备句柄,失败返回-1
 */
int ipu_open(void);

/**
 * @brief 关闭ipu设备句柄
 * @param fd ipu设备句柄,由ipu_open()函数获得
 * @return 成功返回0,失败返回-1
 */
int ipu_close(int fd);

/**
 * @brief 启动ipu绘制
 * @param ipu_fd ipu设备句柄,由ipu_open()函数获得
 * @param ipu_param ipu的信息指针, 指向用户设置的绘制信息
 * @return 成功返回0,失败返回非0
 */
int ipu_start_draw(const int fd, const ipu_param_t *ipu_param);

#endif /* __LIBHARDWARE2_IPU_H__ */