/******************************************************************************
Copyright(c) 2016-2018 Digital Power Inc.
File name: voapi.h
Author: LiuZhengzhong
Version: 1.0.0
Date: 2018/1/30
Description: Platform of DP X5 display C api
History:
Bug report: liuzhengzhong@d-power.com.cn
******************************************************************************/

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "VDecApi.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagSYNC_INFO_S
{

} SYNC_INFO_S;

typedef struct _PUB_ATTR_S
{
    // 背景色，从低位起，ARGB排序
    unsigned int u32BgColor;
    // VO接口时序配置，空结构，相关参数已在系统配置中提供
    SYNC_INFO_S stSyncInfo;

} PUB_ATTR_S;

typedef struct _LAYER_RECT
{
    // 图层的X坐标
    int X;
    // 图层的Y坐标
    int Y;
    // 图层的宽度
    unsigned int W;
    // 图层的高度
    unsigned int H;

} LAYER_RECT;

typedef enum _LAYER_ROTATE_E
{
    ROTATE_NONE = 0,
    ROTATE_90   = 1,
    ROTATE_180  = 2,
    ROTATE_270  = 3,
} LAYER_ROTATE_E;

typedef struct _LAYER_INFO
{
    // 位置信息
    LAYER_RECT Rect;
    // 图层模式，0或1，0:带buffer的图层，1:无buffer，只用一个颜色值表示图像内容，通常使用带buffer的图层
    unsigned int LayerMode;
    // 仅当图层模式为1时此成员有效
    unsigned int Color;
    // 全局Alpha值，默认0xFF
    unsigned char AlphaValue;
    // 默认填0即可
    unsigned char Specified_Layer;
    // 是否在显示时旋转
    LAYER_ROTATE_E Rotate;
} LAYER_INFO;

/******************************************************************************
Function: Enable
Description: 打开显示设备
Param:
Return: 成功返回1，失败返回0
Others:
******************************************************************************/
int Enable(void);

/******************************************************************************
Function: Disable
Description: 关闭显示设备
Param:
Return: 成功返回1，失败返回0
Others:
******************************************************************************/
int Disable(void);

/******************************************************************************
Function: SetPubAttr
Description: 设置VO公共属性
Param:
    Attr    in      属性
Return: 成功返回1，失败返回0
Others: 暂时只支持设置背景色
******************************************************************************/
int SetPubAttr(PUB_ATTR_S *Attr);

/******************************************************************************
Function: GetPubAttr
Description: 获取VO公共属性
Param:
    Attr    out      接收数据结构体指针
Return: 成功返回1，失败返回0
Others: 暂时只支持获取背景色信息
******************************************************************************/
int GetPubAttr(PUB_ATTR_S *Attr);

/******************************************************************************
Function: EnableChn
Description: 使能视频通道
Param:
    Channel in      指定通道
    Info    in      图层参数结构
Return: 成功返回1，失败返回0
Others: Channel取值范围是0和1，两个视频通道供使用
        使能通道的同时，会使能该通道的第0个图层
******************************************************************************/
int EnableChn(unsigned int Channel, LAYER_INFO *Info);

/******************************************************************************
Function: DisableChn
Description: 关闭视频通道
Param:
    Channel in      指定通道
Return: 成功返回1，失败返回0
Others: Channel取值范围是0和1，两个视频通道供使用
        关闭通道的同时，会关闭该通道的第0个图层
        如果有其他图层存在，并且0图层关闭，其他图层也无法显示
******************************************************************************/
int DisableChn(unsigned int Channel);

/******************************************************************************
Function: EnableVideoLayer
Description: 使能视频层
Param:
    Channel in      指定通道
    Layer   in      指定图层
    Info    in      图层参数结构
Return: 成功返回1，失败返回0
Others: Channel取值范围是0和1，两个视频通道供使用
        Layer取值范围是0到3，每个通道4层供使用
******************************************************************************/
int EnableVideoLayer(unsigned int Channel, unsigned int Layer, LAYER_INFO *Info);

/******************************************************************************
Function: DisableVideoLayer
Description: 关闭视频层
Param:
    Channel in      指定通道
    Layer   in      指定图层
Return: 成功返回1，失败返回0
Others: Channel取值范围是0和1，两个视频通道供使用
        Layer取值范围是0到3，每个通道4层供使用
******************************************************************************/
int DisableVideoLayer(unsigned int Channel, unsigned int Layer);

/******************************************************************************
Function: SetVideoLayerAttr
Description: 设置视频层属性
Param:
    Channel in      指定通道
    Layer   in      指定图层
    Info    in      图层参数结构
Return: 成功返回1，失败返回0
Others: 空函数
******************************************************************************/
int SetVideoLayerAttr(unsigned int Channel, unsigned int Layer, LAYER_INFO *Info);

/******************************************************************************
Function: GetVideoLayerAttr
Description: 获取视频层属性
Param:
    Channel in      指定通道
    Layer   in      指定图层
    Info    out     接收图层参数结构
Return: 成功返回1，失败返回0
Others: 空函数
******************************************************************************/
int GetVideoLayerAttr(unsigned int Channel, unsigned int Layer, LAYER_INFO *Info);

/******************************************************************************
Function: SetZoomInWindow
Description: 设置缩放窗口
Param:
    Channel in      指定通道
    Layer   in      指定图层
    SrcInfo in      图像信息结构
Return: 成功返回1，失败返回0
Others:
******************************************************************************/
int SetZoomInWindow(unsigned int Channel, unsigned int Layer, VDEC_SRC_INFO *SrcInfo);

/******************************************************************************
Function: ChnShow
Description: 显示指定窗口的图像
Param:
    Channel in      指定通道
    Layer   in      指定图层
    Frame   in      解码后的Frame结构
Return: 成功返回1，失败返回0
Others:
******************************************************************************/
int ChnShow(unsigned int Channel, unsigned int Layer, VDEC_FRAME_S *Frame);

/******************************************************************************
Function: Resize
Description: 重新指定图层的位置和大小
Param:
    Channel in      指定通道
    Layer   in      指定图层
    Rect    in      位置和大小
Return: 成功返回1，失败返回0
Others:
******************************************************************************/
int Resize(unsigned int Channel, unsigned int Layer, LAYER_RECT *Rect);

#ifdef __cplusplus
}
#endif

#endif // !__DISPLAY_H__
