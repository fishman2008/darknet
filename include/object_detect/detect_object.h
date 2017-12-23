/*
 * File:   detect_object.h
 * Author: damone
 *
 * Created on 2017年11月16日, 下午3:08
 */

#ifndef DETECT_OBJECT_H
#define DETECT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "container_linked_list.h"
#include "darknet.h"

/**
 * 目标框
 */
typedef struct {
  /**
   * x坐标
   */
  float x;

  /**
   * y坐标
   */
  float y;

  /**
   * 宽度w
   */
  float w;

  /**
   * 高度h
   */
  float h;

  /**
   * 目标概率
   */
  float prob;

  /**
   * 目标类别
   */
  int classid;
} object_box;

/**
 * @brief  初始化 object_box
 * @note
 * @retval
 */
object_box create_object_box();


/**
 * @brief  两个对象box 的交集
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
float object_box_intersection(object_box a, object_box b);

/**
 * @brief  两个对象box的并集
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
float object_box_union(object_box a, object_box b);

/**
 * @brief  两个目标box的IOU
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
float object_box_iou(object_box a, object_box b);

/**
 * @brief  目标对象节点
 * @note
 * @retval None
 */
typedef struct {
  /**
   * @brief 头部指针
   */
  linked_node pointer;

  /**
   * @brief  目标对象
   */
  object_box box;
} object_box_node;

/**
 * @brief
 * @note   节点排序比较函数
 * @param  *a:
 * @param  *b:
 * @retval
 */
int object_box_node_compare(object_box_node *a, object_box_node *b);

/**
 * @brief  创建目标对象节点
 * @note
 * @retval
 */
object_box_node *create_object_box_node(object_box value);

/** 
 * @brief  对按概率降序排列的目标对象进行极大值抑制处理
 * @note   送入的链表必须是已经按目标概率降序排列好的
 * @param  *link_list: 
 * @param  nms: 
 * @retval None
 */
void object_boxs_nms(linked_list *link_list, float nms);


/**
 * @brief  目标转换矫正一次性参数，一次性完成从网络输出坐标到目标提取坐标的转换
 * @note
 * @retval None
 */
typedef struct {
  /**
   *  x 及w 的选项
   */
  float xt;
  float wt;
  float xd;

  /**
   * y 及h 的选项
   */
  float yt;
  float ht;
  float yd;
} correct_param;

/**
 * @brief  得到目标框提取矫正参数
 * @note  （1） 将网络预测的宽度转换为在图像上的图片位置框，预测的X Y 是框中心，
 * 需要的是X Y左左上角的坐标
 *        （2） 预测的坐标是归一化到 0 - 1之间进行的， 需要进行相应的转换
 * @param  w: 原始图像宽度
 * @param  h: 原始图像高度
 * @param  netw:  网络输入宽度
 * @param  neth:  网络输入高度
 * @retval
 */
correct_param create_correct_param(int w, int h, int netw, int neth);

/**
 * @brief  矫正目标位置信息
 * @note
 * @param  param:
 * @param  box:
 * @retval
 */
object_box object_box_correct(correct_param param, object_box box);

/**
 * @brief  解析目标对象
 * @note   过程 :
 *             (1）把目标从网络输出中解析出来,包括类别及概率及位置信息
 *             (2）根据阈值进行过滤
 *             (3) 将通过筛选的目标进行位置信息矫正，得到在原图像中的位置信息
 *             (4)将目标按概率进行从大到小的顺序进行排列，这里使用链表有序插
 *                  入实现了有序
 *             (5) 对筛选的目标进行极大值抑制剔除
 * @param  l:  region网络输出层
 * @param  param  位置变换参数
 * @param  thresh:
 * @param  nms:
 * @param  *list:
 * @retval None
 */
void parse_object_boxs(layer l, correct_param param, float thresh, float nms,
                       linked_list *link_list);

#ifdef __cplusplus
}
#endif

#endif /* DETECT_OBJECT_H */
