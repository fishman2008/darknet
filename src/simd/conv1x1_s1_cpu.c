
#include "conv_neon.h"

void conv1x1_s1_cpu(struct conv_params *params, size_t batch, size_t filters) {
    int w = params->w;
    int h = params->h;
    int inch = params->inch;
    int pad = params->pad;
    int pad_w = w + 2 * pad;
    int pad_h = h + 2 * pad;

    int outw = params->outw;
    int outh = params->outh;
    int outch = params->outch;
    const float *kernel = params->weights;

    const float * batchdata = params->input + batch * inch * pad_h * pad_w;
    float *out = params->output + batch * (outw * outh * outch) + filters * (outw * outh);
    memset(out, 0, sizeof (float) * outw * outh);

    int q = 0;
    for (; q + 3 < inch; q += 4) {
        float* outptr = out;

        const float* img0 = batchdata + q * pad_h * pad_w;
        const float* img1 = batchdata + (q + 1) * pad_h * pad_w;
        ;
        const float* img2 = batchdata + (q + 2) * pad_h * pad_w;
        const float* img3 = batchdata + (q + 3) * pad_h * pad_w;

        const float* kernel0 = kernel + filters * inch + q;
        const float k0 = kernel0[0];
        const float k1 = kernel0[1];
        const float k2 = kernel0[2];
        const float k3 = kernel0[3];

        const float* r0 = img0;
        const float* r1 = img1;
        const float* r2 = img2;
        const float* r3 = img3;

        int size = outw * outh;
        int remain = size;
        for (; remain > 0; remain--) {
            float sum = *r0 * k0;
            float sum1 = *r1 * k1;
            float sum2 = *r2 * k2;
            float sum3 = *r3 * k3;

            *outptr += sum + sum1 + sum2 + sum3;

            r0++;
            r1++;
            r2++;
            r3++;
            outptr++;
        }
    }

    for (; q < inch; q++) {
        float* outptr = out;

        const float* img0 = batchdata + q * pad_h * pad_w;

        const float* kernel0 = kernel + filters * inch + q;
        const float k0 = kernel0[0];

        const float* r0 = img0;
        int size = outw * outh;
        int remain = size;

        for (; remain > 0; remain--) {
            float sum = *r0 * k0;

            *outptr += sum;

            r0++;
            outptr++;
        }
    }
}