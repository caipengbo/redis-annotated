// adlist.h - A generic doubly linked list implementation

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

// 双端链表节点
// 使用typedef就不需要总在struct名字前面加关键字struct了
typedef struct listNode {

    // 前向指针
    struct listNode *prev;

    // 后向指针
    struct listNode *next;

    // 节点的值
    void *value;

} listNode;

// 双端链表迭代器
typedef struct listIter {

    // 当前迭代到的节点
    listNode *next;

    // 迭代的方向
    int direction;

} listIter;

// 双端链表结构
// 使用函数指针，模仿成员函数，面向对象的思想
typedef struct list {

    // 表头节点
    listNode *head;

    // 表尾节点
    listNode *tail;

    // 节点值复制函数
    // dup 是个函数指针，指向void * (void *ptr)，设置它们的值，使用的是下方的宏 listSetDupMethod
    void *(*dup)(void *ptr);

    // 节点值释放函数
    void (*free)(void *ptr);

    // 节点值对比函数
    int (*match)(void *ptr, void *key);

    // 链表所包含的节点数量
    unsigned long len;

} list;


/* Functions implemented as macros */
// 需要学习各种宏的使用方法（推荐C参考手册）

// 返回给定链表所包含的节点数量
#define listLength(l) ((l)->len)

// 返回给定链表的表头节点
#define listFirst(l) ((l)->head)

// 返回给定链表的表尾节点
#define listLast(l) ((l)->tail)

// 返回给定节点的前置节点
#define listPrevNode(n) ((n)->prev)

// 返回给定节点的后置节点
#define listNextNode(n) ((n)->next)

// 返回给定节点的值
#define listNodeValue(n) ((n)->value)

// 将链表 l 的值复制函数设置为 m
// T = O(1)
#define listSetDupMethod(l,m) ((l)->dup = (m))
// 将链表 l 的值释放函数设置为 m
// T = O(1)
#define listSetFreeMethod(l,m) ((l)->free = (m))
// 将链表的对比函数设置为 m
// T = O(1)
#define listSetMatchMethod(l,m) ((l)->match = (m))

// 返回给定链表的值复制函数
#define listGetDupMethod(l) ((l)->dup)

// 返回给定链表的值释放函数
#define listGetFree(l) ((l)->free)

// 返回给定链表的值对比函数
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
list *listCreate(void);
void listRelease(list *list);
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
void listDelNode(list *list, listNode *node);
listIter *listGetIterator(list *list, int direction);
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);
list *listDup(list *orig);
listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, long index);
void listRewind(list *list, listIter *li);
void listRewindTail(list *list, listIter *li);
void listRotate(list *list);

//  Directions for iterators
// 从表头向表尾进行迭代
#define AL_START_HEAD 0
// 从表尾到表头进行迭代
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
