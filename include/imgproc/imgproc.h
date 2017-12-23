/*
 * File:   imgproc.h
 * Author: damone
 *
 * Created on 2017年11月11日, 上午11:21
 */

#ifndef IMGPROC_H
#define IMGPROC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <limits.h>
#include <nnpack.h>

typedef uint32_t bool_t;
typedef float float_t;

#define NULL 0
/*
#define M_PI 3.14159265358979323846
#define M_PI_4 (M_PI / 4)
#define M_PI_2 (M_PI / 2)
#define bit_is_set(x, b) ((x >> b) & 0x1)
#define _BV(bit) (1 << (bit))
#define set_bit(a, n) a |= (1 << n)
#define clear_bit(a, n) a &= ~(1 << n)
#define rad_of_deg(x) ((x) * (M_PI / 180.))
*/
#define Min(x, y) (x < y ? x : y)
#define Max(x, y) (x > y ? x : y)
#define ABS(val) ((val) < 0 ? -(val) : (val))

/* The different type of images we currently support */
enum image_type {
  PIXEL_CONVERT_SHIFT = 16,
  PIXEL_FORMAT_MASK = 0x0000ffff,
  PIXEL_CONVERT_MASK = 0xffff0000,

  /**
   * image type
   * RGB
   * BGR
   * YUV
   * WHCF   长宽顺序优先，通道依次排列， 像素类型为浮点类型
   */
  PIXEL_RGB = 1,
  PIXEL_BGR = (1 << 1),
  PIXEL_YUV = (1 << 3),
  PIXEL_WHCF = (1 << 4),

  PIXEL_RGB2WHCF = PIXEL_RGB | (PIXEL_WHCF << PIXEL_CONVERT_SHIFT),
  PIXEL_BGR2WHCF = PIXEL_BGR | (PIXEL_WHCF << PIXEL_CONVERT_SHIFT),
  PIXEL_YUV2WHCF = PIXEL_YUV | (PIXEL_WHCF << PIXEL_CONVERT_SHIFT),
};

/* Main image structure */
struct image_t {
  enum image_type type; ///< The image type
  uint16_t w;           ///< Image width
  uint16_t h;           ///< Image height
  uint16_t c;           ///< Image channls

  uint32_t buf_size; ///< The buffer size
  void *buf;         ///< Image buffer (depending on the image_type)
};

/* Image point structure */
struct point_t {
  uint32_t x; ///< The x coordinate of the point // CHANGED 16 -> 32
  uint32_t y; ///< The y coordinate of the point // CHANGED 16 -> 32
};

/**
 * 创建图片,拷贝图片， 释放图片
 */
void image_create(struct image_t *img, uint16_t w, uint16_t h, uint16_t c,
                  enum image_type type);
void image_copy(struct image_t *input, struct image_t *output);
void image_free(struct image_t *img);

/**
 * @brief  图像缩放参数，用于反复使用
 */
struct resize_params {

  /**
   * 缩放前图像大小
   */
  uint16_t src_w;
  uint16_t src_h;

  /**
   * 缩放后图像大小
   */
  uint16_t w;
  uint16_t h;

  /**
   * 缩放比例
   */
  float scale_x;
  float scale_y;

  /**
   * 参数缓冲区
   */
  int *buf;

  /**
   * 坐标偏移
   */
  int *xofs;
  int *yofs;

  /**
   * 变换参数
   */
  short *ialpha;
  short *ibeta;
};

/**
 * @brief  创建变换参数
 * @note
 * @param  resize:
 * @param  w:
 * @param  h:
 * @param  src_w:
 * @param  src_h:
 * @retval None
 */
void resize_params_create(struct resize_params *resize, uint16_t w, uint16_t h,
                          uint16_t src_w, uint16_t src_h);

/**
 * @brief  释放变换参数
 * @note
 * @param  resize:
 * @retval None
 */
void resize_params_free(struct resize_params *resize);

/**
 * 从RGB，BGR,YUV格式内容设置WHC图片的内容
 * 图片大小由img 指定
 */
bool_t image_data_from_rgb(struct image_t *img, const void *data);
bool_t image_data_from_bgr(struct image_t *img, const void *data);
bool_t image_data_from_yuv(struct image_t *img, const void *data);

/**
 * @brief 图像嵌入对象描述
 */
struct embed_box {
  /**
   * @brief  嵌入的位置x
   */
  uint16_t embed_x;

  /**
   * @brief  嵌入的位置 y
   **/
  uint16_t embed_y;

  /**
   * @brief  嵌入的目标图像宽度
   */
  uint16_t embed_w;

  /**
   * @brief 嵌入的目标图像宽度
   */
  uint16_t embed_h;
};

/**
 * @brief  缩放线程参数
 */
struct resize_thread_paramters {
  /**
   * @brief 目标图片
   */
  struct image_t *img;

  /**
   * @brief  变换参数
   */
  struct resize_params *resize;

  /**
   * @brief 原始图片数据
   */
  const void *data;

  /**
   * @brief 嵌入目标图像描述
   */
  struct embed_box box;
};

/**
 * @brief  先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为rgb
 * @note
 * @param  *img: 目标图像
 * @param  *resize: 改变参数
 * @param  *data:  原始图像数据
 * @retval 是否成功 0表示失败 1表示成功
 * */
bool_t image_data_from_rgb_resize(struct image_t *img,
                                  struct resize_params *resize,
                                  const void *data);

/**
* @brief  先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为rgb
* @note
* @param  *img: 目标图像
* @param  *resize: 改变参数
* @param  *data:  原始图像数据
* @retval 是否成功 0表示失败 1表示成功
* */
bool_t image_data_from_rgb_resize_neon(struct image_t *img,
                                       struct resize_params *resize,
                                       const void *data);
/**
* @brief  RGB缩线程函数，单个任务完成一行的计算
* @note
* @param  *params: 参数
* @param  index:  第几个任务，即计算行图片数据
* @retval None
*/
void rgb_resize_thread_neon(void *params, size_t index);

/**
 * @brief   先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为rgb,
 * 通过线程池完成
 * @note
 * @param  *img: 目标图像
 * @param  *resize:  变换参数
 * @param  *data:  原始数据
 * @param  threadpool: 线程池对象指针
 * @retval  是否成功 0 表示是否  1表示成功
 */
bool_t image_data_from_rgb_resize_thread(struct image_t *img,
                                         struct resize_params *resize,
                                         const void *data,
                                         pthreadpool_t threadpool);

/**
* @brief  RGB缩线程函数，单个任务完成一行的计算
* @note
* @param  *params: 参数
* @param  index:  第几个任务，即计算行图片数据
* @retval None
*/
void rgb_resize_embed_thread_neon(void *params, size_t index);


/**
 * @brief   先改变图像大小和目标图像一致，嵌入指定目标图像，缓冲区数据格式为rgb,
 * 通过线程池完成
 * @note
 * @param  *img: 目标图像
 * @param  *resize:  变换参数
 * @param  *data:  原始数据
 * @param  box: 
 * @param  threadpool: 线程池对象指针
 * @retval  是否成功 0 表示是否  1表示成功
 */
bool_t image_data_from_rgb_resize_embed_thread(struct image_t *img,
                                               struct resize_params *resize,
                                               const void *data,
                                               struct embed_box box,
                                               pthreadpool_t threadpool);

/**
 * @brief  先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为bgr
 * @note
 * @param  *img: 目标图像
 * @param  *resize: 改变参数
 * @param  *data:  原始图像数据
 * @retval 是否成功 0表示失败 1表示成功
 */
bool_t image_data_from_bgr_resize(struct image_t *img,
                                  struct resize_params *resize,
                                  const void *data);

/**
* @brief  先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为yuv
* @note
* @param  *img: 目标图像
* @param  *resize: 改变参数
* @param  *data:  原始图像数据
* @retval 是否成功 0表示失败 1表示成功
*/
bool_t image_data_from_yuv_resize(struct image_t *img,
                                  struct resize_params *resize,
                                  const void *data);

#ifdef __cplusplus
}
#endif

#endif /* IMGPROC_H */
