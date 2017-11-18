
#include <nnpack.h>
#include "../src/convolutional_layer.h"
#include "conv_test.h"

void test_1x1_convolutional_layer(pthreadpool_t threadpool) {
    convolutional_layer l = make_convolutional_layer(1, 5, 5, 3, 1, 1, 1, 0, 1, LEAKY, 0, 0, 0, 0);

    float data[] = {
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
    };

    network net = make_network(1);
    net.h = 5;
    net.w = 5;
    net.c = 3;
    net.workspace = calloc(1, l.workspace_size);
    net.batch = 1;
    net.input = data;

    {
        make_border_data(net.workspace, net.input, l.batch, l.pad, l.w, l.h, l.c);
        struct conv_params params = {net.workspace, l.output, &l};
        pthreadpool_compute_2d(0,
                (pthreadpool_function_2d_t) conv1x1_s1_cpu,
                &params, l.batch, l.out_c);

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
    }

    {
        memset(l.output, 0, sizeof (float) * l.out_h * l.out_w * l.out_c);
        struct nnp_size input_size = {l.w, l.h};
        struct nnp_padding input_padding = {l.pad, l.pad, l.pad, l.pad};
        struct nnp_size kernel_size = {l.size, l.size};
        struct nnp_size stride = {l.stride, l.stride};
        int j;

        int group_in_ch = l.c / l.groups;
        int group_out_ch = l.n / l.groups;
        int group_ksize = group_in_ch * l.size * l.size;
        int group_in_step = l.h * l.w * group_in_ch;
        int group_out_step = l.out_w * l.out_h * group_out_ch;

        for (j = 0; j < l.groups; j++) {
            nnp_convolution_inference(
                    nnp_convolution_algorithm_implicit_gemm,
                    nnp_convolution_transform_strategy_tuple_based,
                    group_in_ch,
                    group_out_ch,
                    input_size,
                    input_padding,
                    kernel_size,
                    stride,
                    net.input + j * group_in_step,
                    l.weights + j * group_ksize,
                    NULL,
                    l.output + j * group_out_step,
                    NULL,
                    NULL,
                    nnp_activation_identity,
                    NULL,
                    threadpool,
                    NULL);
        }

        int h = l.out_h;
        int w = l.out_w;
        int c = l.n;

        image iw = float_to_image(l.size, l.size, l.n, l.weights);
        printf("weihts:\n");
        print_image(iw);

        image im = float_to_image(w, h, c, l.output);
        printf("nnpack filter:\n");
        print_image(im);
        fflush(stderr);
    }
}


void test_depthwise_convolutional_layer(pthreadpool_t threadpool) {
    convolutional_layer l = make_convolutional_layer(1, 5, 5, 3, 3, 3, 1, 1, 3, LEAKY, 0, 0, 0, 0);

    float data[] = {1, 1, 1, 1, 1,
        1, 2, 1, 1, 1,
        1, 1, 3, 1, 1,
        1, 1, 1, 4, 1,
        1, 1, 1, 1, 5,
        2, 2, 2, 2, 2,
        2, 2, 2, 2, 2,
        4, 8, 2, 2, 2,
        2, 2, 2, 2, 2,
        2, 2, 2, 2, 9,
        3, 3, 3, 3, 3,
        5, 3, 3, 7, 3,
        3, 3, 3, 3, 3,
        3, 3, 3, 3, 3,
        4, 3, 3, 3, 3};

    network net = make_network(1);
    net.h = 5;
    net.w = 5;
    net.c = 3;
    net.workspace = calloc(1, l.workspace_size);
    net.batch = 1;

    net.input = data;

    {
        memset(l.output, 0, sizeof (float) * l.out_h * l.out_w * l.out_c);
        make_border_data(net.workspace, net.input, l.batch, l.pad, l.w, l.h, l.c);
        struct conv_params params = {net.workspace, l.output, &l};
        pthreadpool_compute_2d(0,
                (pthreadpool_function_2d_t) dwconv3x3_s1_neon,
                &params, l.batch, l.out_c);

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
    }

    {

        memset(l.output, 0, sizeof (float) * l.out_h * l.out_w * l.out_c);
        forward_convolutional_layer(l, net);
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
    }

    {
        memset(l.output, 0, sizeof (float) * l.out_h * l.out_w * l.out_c);
        struct nnp_size input_size = {l.w, l.h};
        struct nnp_padding input_padding = {l.pad, l.pad, l.pad, l.pad};
        struct nnp_size kernel_size = {l.size, l.size};
        struct nnp_size stride = {l.stride, l.stride};
        int j;

        int group_in_ch = l.c / l.groups;
        int group_out_ch = l.n / l.groups;
        int group_ksize = group_in_ch * l.size * l.size;
        int group_in_step = l.h * l.w * group_in_ch;
        int group_out_step = l.out_w * l.out_h * group_out_ch;

        for (j = 0; j < l.groups; j++) {
            nnp_convolution_inference(
                    nnp_convolution_algorithm_implicit_gemm,
                    nnp_convolution_transform_strategy_tuple_based,
                    group_in_ch,
                    group_out_ch,
                    input_size,
                    input_padding,
                    kernel_size,
                    stride,
                    net.input + j * group_in_step,
                    l.weights + j * group_ksize,
                    NULL,
                    l.output + j * group_out_step,
                    NULL,
                    NULL,
                    nnp_activation_identity,
                    NULL,
                    threadpool,
                    NULL);
        }

        int h = l.out_h;
        int w = l.out_w;
        int c = l.n;

        image iw = float_to_image(l.size, l.size, l.n, l.weights);
        printf("weihts:\n");
        print_image(iw);

        image im = float_to_image(w, h, c, l.output);
        printf("nnpack filter:\n");
        print_image(im);
        fflush(stderr);
    }
}