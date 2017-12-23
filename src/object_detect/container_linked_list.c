/*
 * File:   container_linked_list.h
 * Author: damone
 *
 * Created on 2017年11月15日, 下午5:17
 */
#include <stdlib.h>
#include "object_detect/container_linked_list.h"

/**
 * @brief  初始化节点
 * @note
 * @retval
 */
linked_node create_linked_node() {
  linked_node node;
  node.pre = NULL;
  node.next = NULL;
  return node;
}

/**
 * @brief  构建链表，初始化
 * @note
 * @param  compare:
 * @retval
 */
linked_list create_linked_list(linked_node_compare compare) {
  linked_list temp_list;
  temp_list.linked_size = 0;
  temp_list.head = NULL;
  temp_list.tail = NULL;
  temp_list.compare = compare;
  pthread_mutex_init(&temp_list.mutex, NULL);
  return temp_list;
}

/**
 * @brief  从头部插入节点
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_front(linked_list *link_list, linked_node *node) {
  if (link_list == NULL || NULL == node)
    return;

  node->next = link_list->head;
  node->pre = NULL;
  link_list->head = node;

  if (NULL != node->next) {
    node->next->pre = node;
  }

  if (NULL == link_list->tail)
    link_list->tail = node;

  link_list->linked_size++;
}

/**
 * @brief  从头部插入节点,安全的插入
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_front_safe(linked_list *link_list, linked_node *node) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_insert_front(link_list, node);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  从尾部插入节点
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_tail(linked_list *link_list, linked_node *node) {
  if (NULL == link_list || NULL == node)
    return;

  node->pre = link_list->tail;
  node->next = NULL;
  link_list->tail = node;

  if (NULL != node->pre) {
    node->pre->next = node;
  }

  if (NULL == link_list->head) {
    link_list->head = node;
  }

  link_list->linked_size++;
}

/**
 * @brief  从尾部插入节点,线程安全
 * @note
 * @param  *link_list:
 * @param  node:
 * @retval None
 */
void linked_list_insert_tail_safe(linked_list *link_list, linked_node *node) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_insert_tail(link_list, node);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  在节点nodeto 之前插入
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_before(linked_list *link_list, linked_node *nodeto,
                                    linked_node *nodebe) {
  if (NULL == link_list || NULL == nodeto || NULL == nodebe)
    return;

  if (link_list->head == nodeto)
    link_list->head = nodebe;

  nodebe->pre = nodeto->pre;
  if (NULL != nodebe->pre)
    nodebe->pre->next = nodebe;
  nodebe->next = nodeto;

  nodeto->pre = nodebe;
  link_list->linked_size++;
}

/**
 * @brief  在节点nodeto 之前插入, 线程安全
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_before_safe(linked_list *link_list,
                                         linked_node *nodeto,
                                         linked_node *nodebe) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_insert_node_before(link_list, nodeto, nodebe);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  在节点nodeto 之后插入
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_later(linked_list *link_list, linked_node *nodeto,
                                   linked_node *nodebe) {
  if (NULL == link_list || NULL == nodeto || NULL == nodebe)
    return;

  if (link_list->tail == nodeto)
    link_list->tail = nodebe;

  nodebe->next = nodeto->next;
  if (NULL != nodebe->next)
    nodebe->next->pre = nodebe;
  nodebe->pre = nodeto;

  nodeto->next = nodebe;
  link_list->linked_size++;
}

/**
 * @brief  在节点nodeto 之后插入,线程安全
 * @note
 * @param  *nodeto:
 * @param  nodebe:
 * @retval None
 */
void linked_list_insert_node_later_safe(linked_list *link_list,
                                        linked_node *nodeto,
                                        linked_node *nodebe) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_insert_node_later(link_list, nodeto, nodebe);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  从链表中删除指定的节点
 * @note
 * @param  *link_list:
 * @param  *todelete:
 * @retval None
 */
void linked_list_delete_node(linked_list *link_list, linked_node *todelete) {
  if (NULL == link_list || NULL == todelete)
    return;

  if (link_list->head == todelete)
    link_list->head = todelete->next;

  if (link_list->tail == todelete)
    link_list->tail = todelete->pre;

  if (NULL != todelete->next)
    todelete->next->pre = todelete->pre;

  if (NULL != todelete->pre)
    todelete->pre->next = todelete->next;

  todelete->next = NULL;
  todelete->pre = NULL;
  link_list->linked_size--;
}

/**
 * @brief  从链表中删除指定的节点
 * @note
 * @param  *link_list:
 * @param  *todelete:
 * @retval None
 */
void linked_list_delete_node_safe(linked_list *link_list,
                                  linked_node *todelete) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_delete_node(link_list, todelete);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  有序插入，插入的原始将会按照指定compare 进行排序
 * @note
 * @param  *link_list:
 * @param  *toinsert:
 * @retval None
 */
void linked_list_insert_sort(linked_list *link_list, linked_node *toinsert) {
  if (NULL == link_list || NULL == toinsert)
    return;

  /**
   * @brief 找到第一个比插入节点小的节点
   *        如果没有指定比较函数，则直接插入到末尾
   */
  linked_node *current = link_list->head;
  while (NULL != current) {
    if (NULL == link_list->compare ||
        link_list->compare(toinsert, current) > 0) {
      break;
    }
    current = current->next;
  }

  /**
   * @brief  没有找到，在末尾插入
   *  找到了则在找到的节点节点之前插入
   */
  if (NULL == current) {
    linked_list_insert_tail(link_list, toinsert);
  } else {
    linked_list_insert_node_before(link_list, current, toinsert);
  }
}

/**
 * @brief  有序插入，插入的原始将会按照指定compare 进行排序
 * @note
 * @param  *link_list:
 * @param  *toinsert:
 * @retval None
 */
void linked_list_insert_sort_safe(linked_list *link_list,
                                  linked_node *toinsert) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_insert_sort(link_list, toinsert);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  取出链表的头部节点，并将其从链表中移除
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_head(linked_list *link_list) {
  linked_node *node = NULL;
  if (NULL != link_list) {
    node = link_list->head;
    linked_list_delete_node(link_list, node);
  }
  return node;
}

/**
 * @brief  取出链表的头部节点，并将其从链表中移除
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_head_safe(linked_list *link_list) {
  linked_node *node = NULL;
  pthread_mutex_lock(&link_list->mutex);
  node = pop_linked_list_head(link_list);
  pthread_mutex_unlock(&link_list->mutex);
  return node;
}

/**
 * @brief  取出链表的尾节点并将其从链表中删除
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_tail(linked_list *link_list) {
  linked_node *node = NULL;
  if (NULL != link_list) {
    node = link_list->tail;
    linked_list_delete_node(link_list, node);
  }
  return node;
}

/**
 * @brief  取出链表的尾节点并将其从链表中删除,线程安全
 * @note
 * @param  link_list:
 * @retval
 */
linked_node *pop_linked_list_tail_safe(linked_list *link_list) {
  linked_node *node = NULL;
  pthread_mutex_lock(&link_list->mutex);
  node = pop_linked_list_tail(link_list);
  pthread_mutex_unlock(&link_list->mutex);
  return node;
}

/**
 * @brief  查找指定的节点
 * @note  find 返回1 表示节点是需要的节点,从前开始的第一个节点
 * @param  *link_list:
 * @param  find:
 * @param  *args:
 * @retval None
 */
linked_node *linked_list_find_node(linked_list *link_list, find_node find,
                                   void *args) {
  if (NULL == link_list || NULL == find)
    return NULL;

  linked_node *current = link_list->head;
  while (NULL != current) {
    if (1 == find(current, args)) {
      break;
    }
    current = current->next;
  }

  return current;
}

/**
* @brief  查找指定的节点, 线程安全
* @note   find 返回0 表示节点是需要的节点,从前开始的第一个节点
* @param  *link_list:
* @param  find:
* @param  *args:
* @retval None
*/
linked_node *linked_list_find_node_safe(linked_list *link_list, find_node find,
                                        void *args) {
  if (NULL == link_list || NULL == find)
    return NULL;

  pthread_mutex_lock(&link_list->mutex);
  linked_node *node = linked_list_find_node(link_list, find, args);
  pthread_mutex_unlock(&link_list->mutex);
  return node;
}

/**
 * @brief  对每一个节点执行指定操作,从前到后
 * @note 在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_forword(linked_list *link_list, deal_node deal,
                                  void *args) {
  if (NULL == link_list || NULL == deal)
    return;

  linked_node *current = link_list->head;
  while (NULL != current) {
    deal(current, args);
    current = current->next;
  }
}

/**
 * @brief  对每一个节点执行指定操作,从前到后,向后操作
 * @note  在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_forword_safe(linked_list *link_list, deal_node deal,
                                       void *args) {
  if (NULL == link_list || NULL == deal)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_for_each_forword(link_list, deal, args);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  对每一个节点执行指定操作,从后向前
 * @note 在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_back(linked_list *link_list, deal_node deal,
                               void *args) {
  if (NULL == link_list || NULL == deal)
    return;

  linked_node *current = link_list->tail;
  while (NULL != current) {
    deal(current, args);
    current = current->pre;
  }
}

/**
 * @brief  对每一个节点执行指定操作,从后向前
 * @note 在处理函数中不应该对链表进行操作，不能删除增加，只应访问节点内容
 * @param  *link_list:
 * @param  deal:
 * @param *args 传入参数
 * @retval None
 */
void linked_list_for_each_back_safe(linked_list *link_list, deal_node deal,
                                    void *args) {
  if (NULL == link_list || NULL == deal)
    return;

  pthread_mutex_lock(&link_list->mutex);
  linked_list_for_each_back(link_list, deal, args);
  pthread_mutex_unlock(&link_list->mutex);
}

/**
 * @brief  释放所有节点，所有节点均是透过堆内存创建
 * @note
 * @param  *link_list:
 * @retval None
 */
void destroy_linked_list(linked_list *link_list) {
  if (NULL == link_list)
    return;

  linked_node *current = link_list->head;
  linked_node *tofree = link_list->head;
  while (NULL != current) {
    current = current->next;
    if (NULL != tofree)
      free(tofree);
    tofree = current;
  }

  link_list->linked_size = 0;
  link_list->head = NULL;
  link_list->tail = NULL;
}

/**
 * @brief  释放所有节点，所有节点均是透过堆内存创建,线程安全
 * @note
 * @param  *link_list:
 * @retval None
 */
void destroy_linked_list_safe(linked_list *link_list) {
  if (NULL == link_list)
    return;

  pthread_mutex_lock(&link_list->mutex);
  destroy_linked_list(link_list);
  pthread_mutex_unlock(&link_list->mutex);
}
