/*
 * File:   container_linked_list.h
 * Author: damone
 *
 * Created on 2017年11月15日, 下午5:17
 */

#ifndef CONTAINER_LINKED_LIST_H
#define CONTAINER_LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

/**
 * 节点
 */
typedef struct linked_node_ {
  /**
   * 前一个节点
   */
  struct linked_node_ *pre;

  /**
   * 下一个节点
   */
  struct linked_node_ *next;
} linked_node;


/**
 * 节点比较函数
 */
typedef int (*linked_node_compare)(void *a, void *b);

/**
 * 节点处理函数
 */
typedef void (*deal_node)(linked_node *node, void *args);

/**
 * 节点查找函数, 返回 1 表示对应的节点是需要查找的节点
 */
typedef int (*find_node)(linked_node *node, void *args);


/**
 * @brief  初始化节点
 * @note
 * @retval
 */
linked_node create_linked_node();

/**
 * 链表结构
 */
typedef struct {
  /**
   * 节点数个数
   */
  int linked_size;

  /**
   * 头节点
   */
  linked_node *head;

  /**
   * 尾节点
   */
  linked_node *tail;

  /**
   * 比较函数
   */
  linked_node_compare compare;

  /**
   * 链表访问互斥体
   * 支持线程安全操作
   * */
  pthread_mutex_t mutex;
} linked_list;

/**
 * @brief  构建链表，初始化
 * @note
 * @param  compare:
 * @retval
 */
linked_list create_linked_list(linked_node_compare compare);

/**
 * @brief  从头部插入节点
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_front(linked_list *link_list, linked_node *node);

/**
 * @brief  从头部插入节点,安全的插入
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_front_safe(linked_list *link_list, linked_node *node);

/**
 * @brief  从尾部插入节点
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_tail(linked_list *link_list, linked_node *node);

/**
 * @brief  从尾部插入节点,线程安全
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_tail_safe(linked_list *link_list, linked_node *node);

/**
 * @brief  在节点nodeto 之前插入
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_before(linked_list *link_list, linked_node *nodeto,
                                    linked_node *nodebe);

/**
 * @brief  在节点nodeto 之前插入, 线程安全
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_before_safe(linked_list *link_list,
                                         linked_node *nodeto,
                                         linked_node *nodebe);

/**
 * @brief  在节点nodeto 之后插入
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_later(linked_list *link_list, linked_node *nodeto,
                                   linked_node *nodebe);

/**
 * @brief  在节点nodeto 之后插入,线程安全
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_later_safe(linked_list *link_list,
                                        linked_node *nodeto,
                                        linked_node *nodebe);

/**
 * @brief  从链表中删除指定的节点
 * @note
 * @param  *link_list:
 * @param  *todelete:
 * @retval None
 */
void linked_list_delete_node(linked_list *link_list, linked_node *todelete);

/**
 * @brief  从链表中删除指定的节点
 * @note
 * @param  *link_list:
 * @param  *todelete:
 * @retval None
 */
void linked_list_delete_node_safe(linked_list *link_list,
                                  linked_node *todelete);

/**
 * @brief  有序插入，插入的原始将会按照指定compare 进行排序
 * @note
 * @param  *link_list:
 * @param  *toinsert:
 * @retval None
 */
void linked_list_insert_sort(linked_list *link_list, linked_node *toinsert);

/**
 * @brief  有序插入，插入的原始将会按照指定compare 进行排序
 * @note
 * @param  *link_list:
 * @param  *toinsert:
 * @retval None
 */
void linked_list_insert_sort_safe(linked_list *link_list,
                                  linked_node *toinsert);

/**
 * @brief  取出链表的头部节点，并将其从链表中移除
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_head(linked_list *link_list);

/**
 * @brief  取出链表的头部节点，并将其从链表中移除
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_head_safe(linked_list *link_list);

/**
 * @brief  取出链表的尾节点并将其从链表中删除
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_tail(linked_list *link_list);

/**
 * @brief  取出链表的尾节点并将其从链表中删除,线程安全
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_tail_safe(linked_list *link_list);

/**
 * @brief  查找指定的节点
 * @note  find 返回1 表示节点是需要的节点,从前开始的第一个节点
 * @param  *link_list:
 * @param  find:
 * @param  *args:
 * @retval None
 */
linked_node *linked_list_find_node(linked_list *link_list, find_node find,
                                   void *args);

/**
* @brief  查找指定的节点, 线程安全
* @note   find 返回0 表示节点是需要的节点,从前开始的第一个节点
* @param  *link_list:
* @param  find:
* @param  *args:
* @retval None
*/
linked_node *linked_list_find_node_safe(linked_list *link_list, find_node find,
                                        void *args);

/**
 * @brief  对每一个节点执行指定操作,从前到后
 * @note 在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_forword(linked_list *link_list, deal_node deal,
                                  void *args);

/**
 * @brief  对每一个节点执行指定操作,从前到后,向后操作
 * @note  在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_forword_safe(linked_list *link_list, deal_node deal,
                                       void *args);

/**
 * @brief  对每一个节点执行指定操作,从后向前
 * @note 在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_back(linked_list *link_list, deal_node deal,
                               void *args);

/**
 * @brief  对每一个节点执行指定操作,从后向前
 * @note 在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_back_safe(linked_list *link_list, deal_node deal,
                                    void *args);

/**
 * @brief  释放所有节点，所有节点均是透过堆内存创建
 * @note
 * @param  *link_list:
 * @retval None
 */
void destroy_linked_list(linked_list *link_list);

/**
 * @brief  释放所有节点，所有节点均是透过堆内存创建,线程安全
 * @note
 * @param  *link_list:
 * @retval None
 */
void destroy_linked_list_safe(linked_list *link_list);

#ifdef __cplusplus
}
#endif

#endif /* CONTAINER_LINKED_LIST_H */
