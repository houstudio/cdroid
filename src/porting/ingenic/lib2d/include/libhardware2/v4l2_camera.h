#ifndef V4L2_CAMERA_H_
#define V4L2_CAMERA_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <linux/media.h>
#include <linux/videodev2.h>

#ifndef V4L2_PIX_FMT_SGBRG16
#define V4L2_PIX_FMT_SGBRG16 v4l2_fourcc('G', 'B', '1', '6') /* 16  GBGB.. RGRG.. */
#define V4L2_PIX_FMT_SGRBG16 v4l2_fourcc('G', 'R', '1', '6') /* 16  GRGR.. BGBG.. */
#define V4L2_PIX_FMT_SRGGB16 v4l2_fourcc('R', 'G', '1', '6') /* 16  RGRG.. GBGB.. */
#endif

struct v4l2_camera_format {
    int type;            // V4L2_BUF_TYPE_VIDEO_CAPTURE, 
                         // V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, ...
    unsigned int format; // V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, ...
    int width;
    int height;
};

struct v4l2_camera_buffer {
    unsigned char *data[8]; // buffer 指针,
                            // V4L2_PIX_FMT_YUYV -> yuyv: data[0]
                            // V4L2_PIX_FMT_NV12 -> y: data[0] uv: data[1], ....
    int size[8];            // buffer 大小
    int index;              // buffer index,用户无需关心
};

struct v4l2_camera;

/**
 * @brief 打开v4l2 设备句柄
 * @param path v4l2 设备路径, 如 /dev/video4
 * @return v4l2 设备句柄, NULL 表示失败
 */
struct v4l2_camera *v4l2_camera_open(const char *path);

/**
 * @brief 枚举camera 所支持的格式
 * @param camera v4l2 设备句柄
 * @return 正数:表示获得的格式的个数 0:表示失败
 */
int v4l2_camera_enum_formats(
    struct v4l2_camera *camera, struct v4l2_camera_format *fmts, int count);

/**
 * @brief 探测camera 的格式,格式中值为0的成员会从设备中获取
 * @param camera v4l2 设备句柄
 * @param fmt 格式信息,为0的成员会从设备中获取赋值
 * @return 0:表示成功 负数:失败
 */
int v4l2_camera_detect_format(
    struct v4l2_camera *camera, struct v4l2_camera_format *fmt);

/**
 * @brief 根据提供的格式创建camera buffer
 * @param camera v4l2 设备句柄
 * @param fmt 格式信息
 * @param buf_cnt buffer 的个数
 * @return 0:表示成功 负数:失败
 */
int v4l2_camera_create_buffer(
    struct v4l2_camera *camera, struct v4l2_camera_format *fmt, int buf_cnt);

/**
 * @brief 使能camera 数据流,即开启录制
 * @param camera v4l2 设备句柄
 * @return 0:表示成功 负数:失败
 */
int v4l2_camera_stream_on(struct v4l2_camera *camera);

/**
 * @brief 失能camera 数据流,即停止录制
 * @param camera v4l2 设备句柄
 * @return 0:表示成功 负数:失败
 */
int v4l2_camera_stream_off(struct v4l2_camera *camera);

/**
 * @brief 获取一帧录制的buffer
 * @param camera v4l2 设备句柄
 * @param buf 用于接收buffer信息
 * @param timeout_ms 等待的时间,单位毫秒,-1表示无限制等待
 * @return 0:表示成功 负数:失败
 */
int v4l2_camera_dequeue_buffer(
    struct v4l2_camera *camera, struct v4l2_camera_buffer *buf, int timeout_ms);

/**
 * @brief 将获得的buffer,重新加入录制队列
 * @param camera v4l2 设备句柄
 * @param buf 调用 v4l2_camera_dequeue_buffer 函数获得的buffer
 * @return 0:表示成功 负数:失败
 */
int v4l2_camera_queue_buffer(
    struct v4l2_camera *camera, struct v4l2_camera_buffer *buf);

/**
 * @brief 关闭 v4l2 设备句柄,并释放所创建的buffer和其他资源
 * @param camera v4l2 设备句柄
 */
void v4l2_camera_close(struct v4l2_camera *camera);

/**
 * @brief 打印出格式的信息
 * @param camera v4l2 设备句柄
 */
void v4l2_camera_dump_format(struct v4l2_camera_format *fmt);

#ifdef  __cplusplus
}
#endif

#endif /* V4L2_CAMERA_H_ */
