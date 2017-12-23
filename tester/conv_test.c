
#include <nnpack.h>
#include "../src/convolutional_layer.h"
#include "conv_test.h"
#include "cuda.h"

void test_1x1_convolutional_layer() {
  convolutional_layer l =
      make_convolutional_layer(1, 5, 5, 3, 1, 1, 1, 0, 1, LEAKY, 0, 0, 0, 0);

  float data[] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };

  network net = make_network(1);
#ifdef NNPACK
  nnp_initialize();
  net.threadpool = pthreadpool_create(4);
#endif
  net.h = 5;
  net.w = 5;
  net.c = 3;
  net.workspace = calloc(1, l.workspace_size);
  net.batch = 1;
  net.input = data;

  {
    memset(l.output, 0, sizeof(float) * l.out_h * l.out_w * l.out_c);
    l.forward(l, net);
    int h = l.out_h;
    int w = l.out_w;
    int c = l.n;

    image iw = float_to_image(l.size, l.size, l.n, l.weights);
    printf("weihts:\n");
    print_image(iw);

    image im = float_to_image(w, h, c, l.output);
    printf("dw_3x3 filter:\n");
    print_image(im);
    fflush(stderr);

    memset(l.output, 0, sizeof(float) * l.out_h * l.out_w * l.out_c);
    forward_convolutional_layer(l, net);

    im = float_to_image(w, h, c, l.output);
    printf("dw_3x3 filter:\n");
    print_image(im);
    fflush(stderr);
  }
}

void test_depthwise_convolutional_layer() {
  convolutional_layer l =
      make_convolutional_layer(1, 5, 5, 3, 3, 3, 1, 1, 3, LEAKY, 1, 0, 0, 0);

  float data[] = {1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 4,
                  1, 1, 1, 1, 1, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 8, 2,
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 3, 3, 3, 3, 3, 5, 3,
                  3, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3};

  network net = make_network(1);
#ifdef NNPACK
  nnp_initialize();
  net.threadpool = pthreadpool_create(4);
  expand_rolling_mean_variance(&l);
#endif
  net.h = 5;
  net.w = 5;
  net.c = 3;
  net.workspace = calloc(1, l.workspace_size);
  net.batch = 1;

  net.input = data;

  {

    memset(l.output, 0, sizeof(float) * l.out_h * l.out_w * l.out_c);
    l.forward(l, net);
    int h = l.out_h;
    int w = l.out_w;
    int c = l.n;

    image iw = float_to_image(l.size, l.size, l.n, l.weights);
    printf("weihts:\n");
    print_image(iw);

    image im = float_to_image(w, h, c, l.output);
    printf(" filter:\n");
    print_image(im);
    fflush(stderr);

    memset(l.output, 0, sizeof(float) * l.out_h * l.out_w * l.out_c);
    forward_convolutional_layer(l, net);

    im = float_to_image(w, h, c, l.output);
    printf(" filter:\n");
    print_image(im);
    fflush(stderr);
  }
}

int main() { test_depthwise_convolutional_layer(); }
