/*
 * File:   image_test.c
 * Author: damone
 *
 * Created on 2017年11月13日, 下午1:59
 */
#include "../src/image.h"
#include "../src/stb_image.h"
#include "../include/nnpack.h"
#include "imgproc/imgproc.h"
#include "../src/utils.h"

void test_resize_image_rgb(char *srcpath, char *distpath) {
  struct resize_params params;
  struct image_t img;
  int w, h, c;
  int channels = 3;

  unsigned char *data = stbi_load(srcpath, &w, &h, &c, channels);
  //data = (unsigned char*) malloc(w*h*c);
  if (NULL != data) {
    printf("%d", *data);
    resize_params_create(&params, 224, 224, w, h);
    image_create(&img, 224, 224, 3, PIXEL_WHCF);

    TIME_BEGIN(image_data_from_rgb_resize);
    image_data_from_rgb_resize(&img, &params, data);
    TIME_END(image_data_from_rgb_resize);

    TIME_BEGIN(image_data_from_rgb_resize_neon);
    image_data_from_rgb_resize_neon(&img, &params, data);
    TIME_END(image_data_from_rgb_resize_neon);

    pthreadpool_t pool = pthreadpool_create(4);
    TIME_BEGIN(image_data_from_rgb_resize_thread);
    image_data_from_rgb_resize_thread(&img, &params, data, pool);
    TIME_END(image_data_from_rgb_resize_thread);
    pthreadpool_destroy(pool);

    image orign = load_image_color(srcpath, w, h);
    TIME_BEGIN(resize_image);
    image retest = resize_image(orign, 288, 288);
    TIME_END(resize_image);

    save_image(retest, "verify");
    if (1) {

      image dstimage = make_image(img.w, img.h, img.c);
      memcpy(dstimage.data, img.buf, img.buf_size);
      printf("dstimage\n");
      // print_image(dstimage);
      save_image(dstimage, distpath);
      free_image(dstimage);

      image orign = load_image_color(srcpath, w, h);
      TIME_BEGIN(resize_image);
      image retest = resize_image(orign, 288, 288);
      TIME_END(resize_image);
      save_image(retest, "verify");
    } else {
      fprintf(stderr, "test_resize_image_rgb Cannot Resize image \"%s\n",
              srcpath);
    }
    image_free(&img);
    resize_params_free(&params);
  } else {
    fprintf(stderr,
            "test_resize_image_rgb Cannot load image \"%s\"\nSTB Reason: %s\n",
            srcpath, stbi_failure_reason());
  }
}

void test_resize_embed_image_rgb(char *srcpath, char *distpath) {
  struct resize_params params;
  struct image_t img;
  int w, h, c;
  int channels = 3;

  unsigned char *data = stbi_load(srcpath, &w, &h, &c, channels);
  if (NULL != data) {
    resize_params_create(&params, 320, 240, w, h);
    struct embed_box box = {0, 40, 320, 320};
    image_create(&img, box.embed_w, box.embed_h, 3, PIXEL_WHCF);

    pthreadpool_t pool = pthreadpool_create(4);
    TIME_BEGIN(image_data_from_rgb_resize_embed_thread);
    image_data_from_rgb_resize_embed_thread(&img, &params, data, box, 0);
    TIME_END(image_data_from_rgb_resize_embed_thread);
    pthreadpool_destroy(pool);

    image dstimage = make_image(img.w, img.h, img.c);
    memcpy(dstimage.data, img.buf, img.buf_size);
    printf("dstimage\n");
    save_image(dstimage, distpath);
    free_image(dstimage);

    image_free(&img);
    resize_params_free(&params);
  } else {
    fprintf(stderr,
            "test_resize_image_rgb Cannot load image \"%s\"\nSTB Reason: %s\n",
            srcpath, stbi_failure_reason());
  }
}

int main(int argc, char *argv[]) {
  if (argc >= 3) {
    test_resize_embed_image_rgb(argv[1], argv[2]);
  }
}
