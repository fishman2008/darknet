#include "conv_neon.h"

void dwconv3x3_s1_cpu(struct conv_params *params, size_t batch, size_t filters)
{
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

    float *out = params->output + batch * (outw * outh * outch) + filters * (outw * outh);
    memset(out, 0, sizeof(float) * outw * outh);

    const float *img0 = params->input + (inch * batch + filters) * pad_w * pad_h;
    float *outptr = out;
    float *outptr2 = outptr + outw;
    const float *kernel0 = kernel + filters * 9;

    const float *r0 = img0;
    const float *r1 = img0 + pad_w;
    const float *r2 = img0 + pad_w * 2;
    const float *r3 = img0 + pad_w * 3;

    const float *k0 = kernel0;
    const float *k1 = kernel0 + 3;
    const float *k2 = kernel0 + 6;

    int i = 0;
    for (; i + 1 < outh; i += 2)
    {
        int remain = outw;
        for (; remain > 0; remain--)
        {

            float sum = 0;
            float sum2 = 0;

            sum += r0[0] * k0[0];
            sum += r0[1] * k0[1];
            sum += r0[2] * k0[2];
            sum += r1[0] * k1[0];
            sum += r1[1] * k1[1];
            sum += r1[2] * k1[2];
            sum += r2[0] * k2[0];
            sum += r2[1] * k2[1];
            sum += r2[2] * k2[2];

            sum2 += r1[0] * k0[0];
            sum2 += r1[1] * k0[1];
            sum2 += r1[2] * k0[2];
            sum2 += r2[0] * k1[0];
            sum2 += r2[1] * k1[1];
            sum2 += r2[2] * k1[2];
            sum2 += r3[0] * k2[0];
            sum2 += r3[1] * k2[1];
            sum2 += r3[2] * k2[2];

            *outptr = sum;
            *outptr2 = sum2;
            r0++;
            r1++;
            r2++;
            r3++;
            outptr++;
            outptr2++;
        }

        r0 += 2 + pad_w;
        r1 += 2 + pad_w;
        r2 += 2 + pad_w;
        r3 += 2 + pad_w;

        outptr += outw;
        outptr2 += outw;
    }

    for (; i < outh; i++)
    {
        int remain = outw;
        for (; remain > 0; remain--)
        {
            float sum = 0;
            sum += r0[0] * k0[0];
            sum += r0[1] * k0[1];
            sum += r0[2] * k0[2];
            sum += r1[0] * k1[0];
            sum += r1[1] * k1[1];
            sum += r1[2] * k1[2];
            sum += r2[0] * k2[0];
            sum += r2[1] * k2[1];
            sum += r2[2] * k2[2];

            *outptr += sum;
            r0++;
            r1++;
            r2++;
            outptr++;
        }

        r0 += 2;
        r1 += 2;
        r2 += 2;
    }
}