/*
 * File:   detect_object.h
 * Author: damone
 *
 * Created on 2017年11月16日, 下午3:08
 */

#include <stdlib.h>
#include "utils.h"
#include "../region_layer.h"
#include "object_detect/detect_object.h"

/**
 * @brief  初始化 object_box
 * @note
 * @retval
 */
object_box create_object_box() {
  object_box box;
  box.w = 0;
  box.h = 0;
  box.x = 0;
  box.y = 0;
  box.prob = 0;
  box.classid = -1;
  box.anchor_x = -1;
  box.anchor_y = -1;
  box.anchor_n = -1;
  return box;
}

float box_overlap(float x1, float w1, float x2, float w2) {
  float left = x1 > x2 ? x1 : x2;
  float r1 = x1 + w1;
  float r2 = x2 + w2;
  float right = r1 < r2 ? r1 : r2;
  return right - left;
}

/**
 * @brief  两个对象box 的交集
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
float object_box_intersection(object_box a, object_box b) {
  float w = box_overlap(a.x, a.w, b.x, b.w);
  float h = box_overlap(a.y, a.h, b.y, b.h);
  if (w < 0 || h < 0)
    return 0;
  float area = w * h;
  return area;
}

/**
 * @brief  两个对象box的并集
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
float object_box_union(object_box a, object_box b) {
  float i = object_box_intersection(a, b);
  float u = a.w * a.h + b.w * b.h - i;
  return u;
}

/**
 * @brief  两个目标box的IOU
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
float object_box_iou(object_box a, object_box b) {
  return object_box_intersection(a, b) / object_box_union(a, b);
}

/**
 * @brief
 * @note   节点排序比较函数
 * @param  *a:
 * @param  *b:
 * @retval
 */
int object_box_node_compare(object_box_node *a, object_box_node *b) {
  if (NULL == a || NULL == b)
    return 0;

  float diff = a->box.prob - b->box.prob;
  if (diff > 0)
    return 1;
  else if (diff < 0)
    return -1;
  return 0;
}

/**
 * @brief  创建目标对象节点
 * @note
 * @retval
 */
object_box_node *create_object_box_node(object_box value) {
  object_box_node *node = (object_box_node *)malloc(sizeof(object_box_node));
  node->pointer = create_linked_node();
  node->box = value;
  return node;
}

void print_object_box(linked_node *node, void *args) {
  object_box_node *box = (object_box_node *)node;
  if (NULL != box)
    printf("object_box: p_pos: %d %d %d pos: x %f y %f w %f h %f prob: %f "
           "classid ：%d \n",
           box->box.anchor_y, box->box.anchor_x, box->box.anchor_n, box->box.x,
           box->box.y, box->box.w, box->box.h, box->box.prob, box->box.classid);
}

/**
 * @brief  对按概率降序排列的目标对象进行极大值抑制处理
 * @note   送入的链表必须是已经按目标概率降序排列好的
 * @param  *link_list:
 * @param  nms:
 * @retval None
 */
void object_boxs_nms(linked_list *link_list, float nms) {
  if (NULL == link_list || nms <= 0)
    return;

  object_box_node *current = (object_box_node *)link_list->head;
  while (NULL != current) {

    /**
     * @brief  删除满足抑制条件的目标框
     */
    object_box_node *next_node = (object_box_node *)(current->pointer.next);
    while (NULL != next_node) {

      /**
       * @brief  如果目标类型相同且IOU在抑制条件下，删除该节点
       */
      object_box_node *free_node = NULL;
      if (current->box.classid == next_node->box.classid &&
          object_box_iou(current->box, next_node->box) > nms) {
        free_node = (object_box_node *)next_node;
      }

      next_node = (object_box_node *)(next_node->pointer.next);
      if (NULL != free_node) {
        linked_list_delete_node(link_list, free_node);
        free(free_node);
        free_node = NULL;
      }
    }
    current = (object_box_node *)(current->pointer.next);
  }
}

/**
 * @brief  解析目标BOX信息
 * @note
 * @param  *x:
 * @param  *biases:
 * @param  n:
 * @param  index:
 * @param  i:
 * @param  j:
 * @param  w:
 * @param  h:
 * @param  stride:
 * @retval
 */
object_box get_object_box(float *x, float *biases, int n, int index, int i,
                          int j, int w, int h, int stride) {
  object_box b = create_object_box();
  b.x = (i + x[index + 0 * stride]) / w;
  b.y = (j + x[index + 1 * stride]) / h;
  b.w = exp(x[index + 2 * stride]) * biases[2 * n] / w;
  b.h = exp(x[index + 3 * stride]) * biases[2 * n + 1] / h;
  return b;
}

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
correct_param create_correct_param(int w, int h, int netw, int neth) {
  correct_param param;
  int new_w = 0;
  int new_h = 0;

  /**
   * 图片以嵌入的方式放入和网络大小一致的图片中
   * 有一定的偏移
   * 得到图片缩放的大小
   */
  if (((float)netw / w) < ((float)neth / h)) {
    new_w = netw;
    new_h = (h * netw) / w;
  } else {
    new_h = neth;
    new_w = (w * neth) / h;
  }

  /**
   *  x方向参数
   */
  param.xt = (float)(w * netw) / (float)new_w;
  param.wt = (float)(w * netw) / (float)(2 * new_w);
  param.xd = -(float)(w * (netw - new_w)) / (float)(2 * new_w);

  /**
   * y方向参数
   */
  param.yt = (float)(h * neth) / (float)new_h;
  param.ht = (float)(h * neth) / (float)(2 * new_h);
  param.yd = -(float)(h * (neth - new_h)) / (float)(2 * new_h);
  return param;
}

/**
 * @brief  矫正目标位置信息
 * @note
 * @param  param:
 * @param  box:
 * @retval
 */
object_box object_box_correct(correct_param param, object_box box) {
  object_box ret_box = box;
  ret_box.x = param.xt * box.x - param.wt * box.w + param.xd;
  ret_box.y = param.yt * box.y - param.ht * box.h + param.yd;
  ret_box.w = 2 * param.wt * box.w;
  ret_box.h = 2 * param.ht * box.h;
  return ret_box;
}

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
                       linked_list *link_list) {
  TIME_BEGIN(parse_object_boxs);
  int i, j, n, z;
  float *predictions = l.output;
  for (i = 0; i < l.w * l.h; ++i) {
    int row = i / l.w;
    int col = i % l.w;
    for (n = 0; n < l.n; ++n) {
      /**
       * 得到BOX及有无目标的概率
       * */
      int obj_index = entry_index(l, 0, n * l.w * l.h + i, l.coords);
      int box_index = entry_index(l, 0, n * l.w * l.h + i, 0);
      float scale = l.background ? 1 : predictions[obj_index];
      object_box box = get_object_box(predictions, l.biases, n, box_index, col,
                                      row, l.w, l.h, l.w * l.h);
      box.prob = 0;
      box.classid = -1;
      box.anchor_x = col;
      box.anchor_y = row;
      box.anchor_n = n;
      /**
       * 得到最大概率及类型
       */
      for (j = 0; j < l.classes; ++j) {
        int class_index =
            entry_index(l, 0, n * l.w * l.h + i, l.coords + 1 + j);
        float prob = scale * predictions[class_index];
        if (prob > box.prob) {
          box.prob = prob;
          box.classid = j;
        }
      }

      /**
       * @brief  有序插入链表中
       */
      if (box.prob > thresh) {
        // printf("\nbox pos: %d %d n:%d box %f %f %f %f prob: %f\n", row, col, n,
        //        box.x, box.y, box.w, box.h, box.prob);
        box = object_box_correct(param, box);
        object_box_node *node = create_object_box_node(box);
        linked_list_insert_sort(link_list, (linked_node *)node);
      }
    }
  }

  TIME_END(parse_object_boxs);
  /**
   * @brief   进行极大值抑制处理
   */
  // printf("nms before:\n");
  // linked_list_for_each_forword(link_list, print_object_box, NULL);
  // TIME_BEGIN(object_boxs_nms);
  object_boxs_nms(link_list, nms);
  // TIME_END(object_boxs_nms);
  // printf("nms back:\n");
  // linked_list_for_each_forword(link_list, print_object_box, NULL);
}
