/* 
 * File:   conv_test.h
 * Author: damone
 *
 * Created on 2017年9月8日, 下午1:48
 */

#ifndef CONV_TEST_H
#define CONV_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

    void test_1x1_convolutional_layer(pthreadpool_t threadpool);
    void test_depthwise_convolutional_layer(pthreadpool_t threadpool);
#ifdef __cplusplus
}
#endif

#endif /* CONV_TEST_H */

