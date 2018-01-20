/*
 * File:   imgproc.c
 * Author: damone
 *
 * Created on 2017年11月11日, 上午11:21
 */

#include "imgproc/imgproc.h"
#include <nnpack.h>
#include "../utils.h"
#include <stdlib.h>

/**
 * @brief  创建image_t 对象并分配内容缓存区
 * @note   图片缓存区从堆申请,需要调用image_free释放
 *         不建议在外部给图片分配缓冲区,尽量避免内存泄露
 * @param  *img:
 * @param  w:
 * @param  h:
 * @param  c:
 * @param  type:
 * @retval None
 */
void image_create(struct image_t *img, uint16_t w, uint16_t h, uint16_t c,
                  enum image_type type) {
  // Set the variables
  img->type = type;
  img->w = w;
  img->h = h;
  img->c = c;

  // Depending on the type the size differs
  if (PIXEL_YUV == type) {
    img->buf_size = sizeof(uint8_t) * 2 * img->w * img->h;
  } else if (PIXEL_WHCF == type) {
    img->buf_size = sizeof(float_t) * c * img->w * img->h;
  } else {
    img->buf_size = sizeof(uint8_t) * c * img->w * img->h;
  }

  img->buf = malloc(img->buf_size);
}

/**
 * @brief  释放图片缓冲
 * @note
 * @param  *img:
 * @retval None
 */
void image_free(struct image_t *img) {
  if (NULL != img->buf) {
    free(img->buf);
    img->buf = NULL;
  }
}

/**
 * @brief  拷贝图片
 * @note   需要先创建目标图片image_t output,相关参数和 input一致
 *         copy 不会进行output 缓冲的分配,需要传入前分配完成
 * @param  *input:
 * @param  *output:
 * @retval None
 */
void image_copy(struct image_t *input, struct image_t *output) {
  if (input->type != output->type) {
    return;
  }

  output->w = input->w;
  output->h = input->h;
  output->c = input->c;
  output->buf_size = input->buf_size;
  memcpy(output->buf, input->buf, input->buf_size);
}

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
                          uint16_t src_w, uint16_t src_h) {
  const int INTER_RESIZE_COEF_BITS = 11;
  const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;

  if (NULL == resize)
    return;

  resize->w = w;
  resize->h = h;
  resize->src_w = src_w;
  resize->src_h = src_h;
  resize->scale_x = (float)src_w / w;
  resize->scale_y = (float)src_h / h;
  resize->buf = (int *)malloc(sizeof(int) * (w + h + w + h));

  /**
   * int w
   */
  resize->xofs = resize->buf;

  /**
   * int h;
   */
  resize->yofs = resize->buf + w;

  /**
   * short w*2
   */
  resize->ialpha = (short *)(resize->buf + w + h);
  /**
   * short h*2
   */
  resize->ibeta = (short *)(resize->buf + w + h + w);

  float fx;
  float fy;
  int sx;
  int sy;

#define SATURATE_CAST_SHORT(X)                                                 \
  (short) Min(Max((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

  int dx;
  for (dx = 0; dx < w; dx++) {
    fx = (float)((dx + 0.5) * resize->scale_x - 0.5);
    sx = fx; // cvFloor(fx);
    fx -= sx;

    if (sx >= src_w - 1) {
      sx = src_w - 2;
      fx = 1.f;
    }

    resize->xofs[dx] = sx * 3;

    float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
    float a1 = fx * INTER_RESIZE_COEF_SCALE;

    resize->ialpha[dx * 2] = SATURATE_CAST_SHORT(a0);
    resize->ialpha[dx * 2 + 1] = SATURATE_CAST_SHORT(a1);
  }

  int dy;
  for (dy = 0; dy < h; dy++) {
    fy = (float)((dy + 0.5) * resize->scale_y - 0.5);
    sy = fy; // Floor(fy);
    fy -= sy;

    if (sy >= src_h - 1) {
      sy = src_h - 2;
      fy = 1.f;
    }

    resize->yofs[dy] = sy * 3;

    float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
    float b1 = fy * INTER_RESIZE_COEF_SCALE;

    resize->ibeta[dy * 2] = SATURATE_CAST_SHORT(b0);
    resize->ibeta[dy * 2 + 1] = SATURATE_CAST_SHORT(b1);
  }

#undef SATURATE_CAST_SHORT
}

/**
 * @brief  释放变换参数
 * @note
 * @param  resize:
 * @retval None
 */
void resize_params_free(struct resize_params *resize) {
  if (NULL != resize && NULL != resize->buf) {
    free(resize->buf);
    resize->buf = NULL;
    resize->w = 0;
    resize->h = 0;
    resize->src_w = 0;
    resize->src_h = 0;
    resize->scale_x = 1;
    resize->scale_y = 1;
    resize->ialpha = NULL;
    resize->ibeta = NULL;
    resize->xofs = NULL;
    resize->yofs = NULL;
  }
}

/**
 * @brief 设置图片数据从rgb 数据缓冲
 * @note
 * @param  *img:
 * @param  data:
 * @retval
 */
bool_t image_data_from_rgb(struct image_t *img, const void *data) {
  if (NULL == data || NULL == img || NULL == img->buf || img->w <= 0 ||
      img->h <= 0 || img->c <= 0 || PIXEL_WHCF != img->type)
    return 0;

  uint8_t *orign_data = (uint8_t *)data;
  float_t *newdata = (float_t *)img->buf;
  int h, w, c;
  int mapsize = img->w * img->h;
  for (h = 0; h < img->h; h++) {
    for (w = 0; w < img->w; w++) {
      for (c = 0; c < img->c; c++) {
        int new_index = c * (img->w * img->h) + h * img->w + w;
        int orign_index = (h * img->w + w) * img->c + c;
        newdata[new_index] = (float_t)orign_data[orign_index] / (float_t)(255);
      }
    }
  }
  return 1;
}

/**
 * @brief 设置图像数据从bgr 数据缓冲
 * @note
 * @param  *img:
 * @param  data:
 * @retval
 */
bool_t image_data_from_bgr(struct image_t *img, const void *data) {
  if (NULL == data || NULL == img || NULL == img->buf || img->w <= 0 ||
      img->h <= 0 || img->c <= 0 || PIXEL_WHCF != img->type)
    return 0;

  uint8_t *orign_data = (uint8_t *)data;
  float_t *newdata = (float_t *)img->buf;
  int mapsize = img->w * img->h;
  int h, w, c;
  for (h = 0; h < img->h; h++) {
    for (w = 0; w < img->w; w++) {
      for (c = 0; c < img->c; c++) {
        int new_index = c * (img->w * img->h) + h * img->w + w;
        int orign_index = (h * img->w + w) * img->c + c;
        switch (c) {
        case 0: {
          break;
        }
        case 1: {
          break;
        }
        case 2: {
          break;
        }
        }
        newdata[new_index] = (float_t)orign_data[orign_index] / (float_t)(255);
      }
    }
  }
  return 1;
}

/**
 * @brief 设置图片内容从YUV缓冲区
 * @note
 * @param  *img:
 * @param  data:
 * @retval
 */
bool_t image_data_from_yuv(struct image_t *img, const void *data) {}

/**
 * @brief  先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为rgb
 * @note
 * @param  *img: 目标图像
 * @param  *resize: 改变参数
 * @param  *data:  原始图像数据
 * @retval 是否成功 0表示失败 1表示成功
 */
bool_t image_data_from_rgb_resize(struct image_t *img,
                                  struct resize_params *resize,
                                  const void *data) {
  if (NULL == data || NULL == img || NULL == img->buf ||
      PIXEL_WHCF != img->type || NULL == resize || NULL == resize->buf) {
    return 0;
  }

  // loop body
  int size = sizeof(short) * ((resize->w * 3 >> 1) + 3);
  short rows0_[size];
  short rows1_[size];
  short *rows0 = rows0_;
  short *rows1 = rows1_;

  int prev_sy1 = -1;
  float_t *dst = (float_t *)img->buf;

  float_t *dst0 = (float_t *)dst;
  float_t *dst1 = (float_t *)(dst + resize->w * resize->h);
  float_t *dst2 = (float_t *)(dst + 2 * resize->w * resize->h);
  short *ibeta = resize->ibeta;
  const unsigned char *orign_data = (const unsigned char *)data;
  printf("%d", *(int*)data);
  int dy;
  for (dy = 0; dy < resize->h; dy++) {
    int sy = resize->yofs[dy];

    if (sy == prev_sy1) {
      // hresize one row
      short *rows0_old = rows0;
      rows0 = rows1;
      rows1 = rows0_old;
      const unsigned char *S1 = orign_data + resize->src_w * (sy + 3);

      const short *ialphap = resize->ialpha;
      short *rows1p = rows1;
      int dx;
      for (dx = 0; dx < resize->w; dx++) {
        int sx = resize->xofs[dx];
        short a0 = ialphap[0];
        short a1 = ialphap[1];

        const unsigned char *S1p = S1 + sx;

        rows1p[0] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
        rows1p[1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
        rows1p[2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;


        ialphap += 2;
        rows1p += 3;
      }
    } else {
      // hresize two rows
      const unsigned char *S0 = orign_data + resize->src_w * (sy);
      const unsigned char *S1 = orign_data + resize->src_w * (sy + 3);

      const short *ialphap = resize->ialpha;
      short *rows0p = rows0;
      short *rows1p = rows1;
      int dx;
      for (dx = 0; dx < resize->w; dx++) {
        int sx = resize->xofs[dx];
        short a0 = ialphap[0];
        short a1 = ialphap[1];

        const unsigned char *S0p = S0 + sx;
        const unsigned char *S1p = S1 + sx;

        rows0p[0] = (S0p[0] * a0 + S0p[3] * a1) >> 4;
        rows0p[1] = (S0p[1] * a0 + S0p[4] * a1) >> 4;
        rows0p[2] = (S0p[2] * a0 + S0p[5] * a1) >> 4;
        rows1p[0] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
        rows1p[1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
        rows1p[2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;

        ialphap += 2;
        rows0p += 3;
        rows1p += 3;
      }
    }

    prev_sy1 = sy + 1;

    // vresize
    short b0 = ibeta[0];
    short b1 = ibeta[1];

    short *rows0p = rows0;
    short *rows1p = rows1;

    int nn = 0;
    int remain = resize->w;

    for (; remain; --remain) {
      *dst0++ = (((short)((b0 * (short)(*rows0p++)) >> 16) +
                  (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >>
                 2) /
                255.0;
      *dst1++ = (((short)((b0 * (short)(*rows0p++)) >> 16) +
                  (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >>
                 2) /
                255.0;
      *dst2++ = (((short)((b0 * (short)(*rows0p++)) >> 16) +
                  (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >>
                 2) /
                255.0;
    }

    ibeta += 2;
  }

  return 1;
}

/**
 * @brief  先改变图像大小和目标图像一致，然后再设置缓冲区，缓冲区数据格式为rgb
 * @note
 * @param  *img: 目标图像
 * @param  *resize: 改变参数
 * @param  *data:  原始图像数据
 * @retval 是否成功 0表示失败 1表示成功
 */
bool_t image_data_from_rgb_resize_neon(struct image_t *img,
                                       struct resize_params *resize,
                                       const void *data) {
  if (NULL == data || NULL == img || NULL == img->buf ||
      PIXEL_WHCF != img->type || NULL == resize || NULL == resize->buf) {
    return 0;
  }

  return 1;
}

void resize_one_row_neon(struct resize_params *resize, short *ibeta, int sy,
                         int dy, const unsigned char *orign_data, short *rows0,
                         short *rows1, float_t *dst0, float_t *dst1,
                         float_t *dst2) {
  if (sy == -1) {
    // hresize one row
    short *rows0_old = rows0;
    rows0 = rows1;
    rows1 = rows0_old;
    const unsigned char *S1 = orign_data + resize->src_w * (sy + 3);

    const short *ialphap = resize->ialpha;
    short *rows1p = rows1;
    int dx;
    for (dx = 0; dx < resize->w; dx++) {
      int sx = resize->xofs[dx];
      short a0 = ialphap[0];
      short a1 = ialphap[1];

      const unsigned char *S1p = S1 + sx;

      rows1p[0] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
      rows1p[1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
      rows1p[2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;

      ialphap += 2;
      rows1p += 3;
    }
  } else {
    // hresize two rows
    const unsigned char *S0 = orign_data + resize->src_w * (sy);
    const unsigned char *S1 = orign_data + resize->src_w * (sy + 3);

    const short *ialphap = resize->ialpha;
    short *rows0p = rows0;
    short *rows1p = rows1;
    int dx;
    for (dx = 0; dx < resize->w; dx++) {
      int sx = resize->xofs[dx];
      short a0 = ialphap[0];
      short a1 = ialphap[1];

      const unsigned char *S0p = S0 + sx;
      const unsigned char *S1p = S1 + sx;

      rows0p[0] = (S0p[0] * a0 + S0p[3] * a1) >> 4;
      rows0p[1] = (S0p[1] * a0 + S0p[4] * a1) >> 4;
      rows0p[2] = (S0p[2] * a0 + S0p[5] * a1) >> 4;
      rows1p[0] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
      rows1p[1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
      rows1p[2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;

      ialphap += 2;
      rows0p += 3;
      rows1p += 3;
    }
  }

  // vresize
  short b0 = ibeta[2 * dy];
  short b1 = ibeta[2 * dy + 1];

  short *rows0p = rows0;
  short *rows1p = rows1;
  int remain = resize->w;

  for (; remain; --remain) {

    *dst2++ = (((short)((b0 * (short)(*rows0p++)) >> 16) +
                (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >>
               2) /
              255.0;
    *dst1++ = (((short)((b0 * (short)(*rows0p++)) >> 16) +
                (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >>
               2) /
              255.0;
    *dst0++ = (((short)((b0 * (short)(*rows0p++)) >> 16) +
                (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >>
               2) /
              255.0;
  }
}

/**
 * @brief  RGB缩线程函数，单个任务完成一行的计算
 * @note
 * @param  *params:
 * @param  index:
 * @retval None
 */
void rgb_resize_thread_neon(void *params, size_t index) {
  struct resize_thread_paramters *p = (struct resize_thread_paramters *)params;
  struct image_t *img = p->img;
  struct resize_params *resize = p->resize;

  // loop body
  int size = sizeof(short) * ((resize->w * 3 >> 1) + 3);
  short rows0_[size];
  short rows1_[size];
  short *rows0 = rows0_;
  short *rows1 = rows1_;

  short *ibeta = resize->ibeta;
  const unsigned char *orign_data = (const unsigned char *)p->data;
  int dy = index;
  int sy = resize->yofs[dy];

  int map_index = resize->w * dy;
  int map_size = resize->w * resize->h;
  float_t *dst = (float_t *)img->buf;
  float_t *dst0 = dst + map_index;
  float_t *dst1 = dst0 + map_size;
  float_t *dst2 = dst1 + map_size;
  resize_one_row_neon(resize, ibeta, sy, dy, orign_data, rows0, rows1, dst0,
                      dst1, dst2);
}

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
                                         pthreadpool_t threadpool) {
  if (NULL == data || NULL == img || NULL == img->buf ||
      PIXEL_WHCF != img->type || NULL == resize || NULL == resize->buf) {
    return 0;
  }

  struct resize_thread_paramters params = {img, resize, data};
  pthreadpool_compute_1d(threadpool,
                         (pthreadpool_function_1d_t)rgb_resize_thread_neon,
                         &params, resize->h);
  return 1;
}

void fill_data(float_t *data, int length, float_t value) {
  int i = 0;
  for (; i < length; i++)
    *data++ = value;
}

/**
* @brief  RGB缩线程函数，单个任务完成一行的计算
* @note
* @param  *params: 参数
* @param  index:  第几个任务，即计算行图片数据
* @retval None
*/
void rgb_resize_embed_thread_neon(void *params, size_t index) {

  struct resize_thread_paramters *p = (struct resize_thread_paramters *)params;
  struct image_t *img = p->img;
  struct resize_params *resize = p->resize;
  struct embed_box box = p->box;

  // loop body
  int size = sizeof(short) * ((resize->w * 3 >> 1) + 3);
  short rows0_[size];
  short rows1_[size];
  short *rows0 = rows0_;
  short *rows1 = rows1_;

  short *ibeta = resize->ibeta;
  const unsigned char *orign_data = (const unsigned char *)p->data;

  int map_index = box.embed_w * index;
  int map_size = box.embed_h * box.embed_w;
  float_t *dst = (float_t *)img->buf;
  float_t *dst0 = dst + map_index;
  float_t *dst1 = dst0 + map_size;
  float_t *dst2 = dst1 + map_size;

  if (index < box.embed_y || index >= box.embed_y + resize->h) {
    fill_data(dst0, box.embed_w, 0.5);
    fill_data(dst1, box.embed_w, 0.5);
    fill_data(dst2, box.embed_w, 0.5);
  } else {

    if (box.embed_x > 0) {
      fill_data(dst0, box.embed_x, 0.5);
      fill_data(dst1, box.embed_x, 0.5);
      fill_data(dst2, box.embed_x, 0.5);

      dst0 += box.embed_x;
      dst1 += box.embed_x;
      dst2 += box.embed_x;
    }

    int dy = index - box.embed_y;
    int sy = resize->yofs[dy];
    resize_one_row_neon(resize, ibeta, sy, dy, orign_data, rows0, rows1, dst0,
                        dst1, dst2);

    dst0 += resize->w;
    dst1 += resize->w;
    dst2 += resize->w;

    int havelen = box.embed_w - resize->w - box.embed_x;
    if (havelen > 0) {
      fill_data(dst0, havelen, 0.5);
      fill_data(dst1, havelen, 0.5);
      fill_data(dst2, havelen, 0.5);
    }
  }
}

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
                                               pthreadpool_t threadpool) {
  if (NULL == data || NULL == img || NULL == img->buf ||
      PIXEL_WHCF != img->type || NULL == resize || NULL == resize->buf) {
    return 0;
  }

  if (box.embed_h < resize->h || box.embed_w < resize->w ||
      box.embed_x + resize->w > box.embed_w ||
      box.embed_y + resize->h > box.embed_h) {
    return 0;
  }

  printf("resize %d %d\n", box.embed_w, box.embed_h);
  TIME_BEGIN(rgb_resize_embed_thread_neon);
  struct resize_thread_paramters params = {img, resize, data, box};
  pthreadpool_compute_1d(
      threadpool, (pthreadpool_function_1d_t)rgb_resize_embed_thread_neon,
      &params, box.embed_h);
  TIME_END(rgb_resize_embed_thread_neon);

  return 1;
}

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
                                  const void *data) {
  if (NULL == data || NULL == img || NULL == img->buf ||
      PIXEL_WHCF != img->type || NULL == resize || NULL == resize->buf) {
    return 0;
  }
  return 1;
}

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
                                  const void *data) {
  if (NULL == data || NULL == img || NULL == img->buf ||
      PIXEL_WHCF != img->type || NULL == resize || NULL == resize->buf) {
    return 0;
  }
  return 1;
}