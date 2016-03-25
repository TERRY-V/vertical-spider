/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qadlist.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/10/15
**
*********************************************************************************************/

#ifndef __QADLIST_H_
#define __QADLIST_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

/* Node, List, and Iterator are the only data structures used currently. */
typedef struct listNode {
	struct listNode *prev;
	struct listNode *next;
	void *value;
} listNode;

typedef struct listIter {
	listNode *next;
	int32_t direction;
} listIter;

typedef struct adlist {
	listNode *head;
	listNode *tail;
	void *(*dup)(void *ptr);
	void (*free)(void *ptr);
	int32_t (*match)(void *ptr, void *key);
	uint32_t len;
} adlist;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
adlist *listCreate(void);
void listRelease(adlist *list);
adlist *listAddNodeHead(adlist *list, void *value);
adlist *listAddNodeTail(adlist *list, void *value);
adlist *listInsertNode(adlist *list, listNode *old_node, void *value, int32_t after);
void listDelNode(adlist *list, listNode *node);
listIter *listGetIterator(adlist *list, int32_t direction);
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);
adlist *listDup(adlist *orig);
listNode *listSearchKey(adlist *list, void *key);
listNode *listIndex(adlist *list, int32_t index);
void listRewind(adlist *adlist, listIter *li);
void listRewindTail(adlist *list, listIter *li);
void listRotate(adlist *list);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

Q_END_NAMESPACE

#endif // __QADLIST_H_
