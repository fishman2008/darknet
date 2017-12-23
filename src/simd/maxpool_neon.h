/* 
 * File:   max_neon.h
 * Author: damone
 *
 * Created on 2017年9月7日, 上午11:53
 */

#ifndef MAXPOLL_NEON_H
#define MAXPOLL_NEON_H

#ifdef __cplusplus
extern "C" {
#endif

    struct maxpool_params {
        float *input;
        float *output;
        int size;
        int pad;
        int w;
        int h;
        int inch;
        int outw;
        int outh;
        int outch;
        int stride;
    };

    #define MAX(a, b)  (a) > (b) ? (a) : (b);
    #define MIN(a, b)  (a) < (b) ? (a) : (b);
    void maxpool_cpu_thread(struct maxpool_params *params, size_t batch, size_t filters);
    void maxpool_cpu(struct maxpool_params *params, size_t batch, size_t filters);
    void maxpool2x2_s2_cpu(struct maxpool_params *params, size_t batch, size_t filters);

#ifdef __cplusplus
}
#endif

#endif /* MAX_NEON_H */

