
#include <float.h>
#include "conv_neon.h"
#include "maxpool_neon.h"

void maxpool_cpu_thread(struct maxpool_params *params, size_t batch, size_t filters) {
    typedef void (*maxpool_func)(struct maxpool_params *, size_t, size_t);
    maxpool_func maxpool = maxpool_cpu;
    if (2 == params->size && 2 == params->stride && 0 == params->pad)
        maxpool = maxpool2x2_s2_cpu;
    maxpool(params, batch, filters);
}

void maxpool_cpu(struct maxpool_params *params, size_t batch, size_t filters) {
    int w = params->w;
    int h = params->h;
    int inch = params->inch;
    int pad = params->pad;
    int pad_w = w + 2 * pad;
    int pad_h = h + 2 * pad;
    int size = params->size;

    int outw = params->outw;
    int outh = params->outh;
    int outch = params->outch;
    int stride = params->stride;


    const float *img = params->input + (inch * batch + filters) * w * h;
    float *outptr = params->output + batch * (outw * outh * outch) + filters * (outw * outh);
    prefetch_range(img, w * h * 4);
    prefetch_range(outptr, outw * outh * 4);
    int i = 0, j = 0;
    for (i = 0; i < outh; i++) {
        for (j = 0; j < outw; j++) {
            *outptr = -FLT_MAX;
            int m = 0, n = 0;
            for (m = 0; m < size; m++) {
                for (n = 0; n < size; n++) {
                    int cur_h = i * stride + m - pad;
                    int cur_w = j * stride + n - pad;
                    int valid = (cur_h >= 0 && cur_h < h && cur_w >= 0 && cur_w < w);
                    float *indata = img + cur_h * w + cur_w;
                    float val = (valid != 0) ? (*indata) : -FLT_MAX;
                    *outptr = MAX(*outptr, val);
                }
            }
            outptr++;
        }
    }
}

void maxpool2x2_s2_cpu(struct maxpool_params *params, size_t batch, size_t filters) {
    int w = params->w;
    int h = params->h;
    int inch = params->inch;
    int pad = params->pad;
    int pad_w = w + 2 * pad;
    int pad_h = h + 2 * pad;
    int size = params->size;

    int outw = params->outw;
    int outh = params->outh;
    int outch = params->outch;
    int stride = params->stride;

    const float *img = params->input + (inch * batch + filters) * w * h;
    float *outptr = params->output + batch * (outw * outh * outch) + filters * (outw * outh);

    const float* r0 = img;
    const float* r1 = img + pad_w;

    int i = 0;
    for (i = 0; i < outh; i++) {
        int remain = outw;
        prefetch_range(r0, w * 4);
        prefetch_range(r1, w * 4);
        prefetch_range(outptr, outw * 4);
        for (; remain > 0; remain--) {
            float max0 = MAX(r0[0], r0[1]);
            float max1 = MAX(r1[0], r1[1]);
            *outptr = MAX(max0, max1);

            r0 += 2;
            r1 += 2;
            outptr++;
        }

        r0 += pad_w;
        r1 += pad_w;
    }
}

