
#ifndef CONV_NEON_H_
#define CONV_NEON_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "darknet.h"
#include <nnpack.h>
#include "../utils.h"

    struct normalize_params {
        float *x;
        float *adata;
        float *bdata;
        int spatial;
        ACTIVATION a;
    };

    void make_border_row(float *workspace, const float *input, int row, int pad, int w, int h);
    void make_border_data(float *workspace, const float *input, int batch, int pad, int w, int h, int c);
    void normalize_active_cpu_thread(struct normalize_params *params, size_t batch, size_t filters);

    struct conv_params {
        float *input;
        float *output;
        float *weights;
        float *workspace;
        int workspace_size;
        int size;
        int pad;
        int stride;
        int groups;
        int w;
        int h;
        int inch;
        int outw;
        int outh;
        int outch;
    };

    void prefetch_range(void *addr, size_t len);
    void conv_cpu_inference(pthreadpool_t threadpool, struct conv_params *params, size_t batch, size_t filters);
    void conv1x1_s1_cpu(struct conv_params *params, size_t batch, size_t filters);
    void conv3x3_s1_cpu(struct conv_params *params, size_t batch, size_t filters);
    void dwconv3x3_s1_workspace(struct conv_params *params, size_t batch, size_t filters);
    void dwconv3x3_s1_cpu(struct conv_params *params, size_t batch, size_t filters);

#ifdef __cplusplus
}
#endif

#endif