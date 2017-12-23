
/* 
 * File:   tools.h
 * Author: xiangwei
 *
 * Created on 2017年8月3日, 下午2:57
 */

#ifndef TOOLS_H
#define TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <darknet.h>
#include "utils.h"

    /**
     * 初始化网络
     * @param cfgfile
     * @param weightfile
     * @param threadsize
     * @return 
     */
    network network_init(const char *cfgfile, const char *weightfile, int threadsize);
    
    /**
     * 释放网络
     * @param net
     */
    void network_release(network *net);
    
    /**
     * 加载图片
     * @param filename
     * @param net
     * @return 
     */
    image loadimage(char* filename, network *net);
    
    /**
     * 图片转化
     * @param data
     * @param w
     * @param h
     * @param c
     * @param step
     * @return 
     */
    image convert2image(unsigned char *data, int w, int h, int c, int step);
    
    
    /**
     * 检测目标
     * @param net
     * @param im
     * @param thresh
     * @param hier_thresh
     * @param nms
     * @param boxes
     * @param probs
     * @return 
     */
    int detect_object(network * net, image im, float thresh, float hier_thresh, float nms, box *boxes, float **probs);

    /**
     * 验证检测器
     * @param datacfg
     * @param cfgfile
     * @param weightfile
     * @param outpath
     * @param thresh
     * @param hier_thresh
     */
    void validate_detector(char *datacfg, char *cfgfile, char *weightfile, char *outpath, float thresh, float hier_thresh);
    
    /**
     * 测试检测器
     * @param datacfg
     * @param cfgfile
     * @param weightfile
     * @param filename
     * @param thresh
     * @param hier_thresh
     * @param outfile
     */
    void test_detector(char *datacfg, char *cfgfile, char *weightfile, char *filename, float thresh, float hier_thresh, char *outfile);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_H */

