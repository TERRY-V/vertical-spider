#include "qadlist.h"

Q_BEGIN_NAMESPACE

adlist *listCreate(void)
{
	struct adlist *list;

	if ((list = (adlist*)q_malloc(sizeof(*list))) == NULL)
		return NULL;
	list->head = list->tail = NULL;
	list->len = 0;
	list->dup = NULL;
	list->free = NULL;
	list->match = NULL;
	return list;
}

void listRelease(adlist *list)
{
	uint32_t len = 0;
	listNode *current = NULL, *next = NULL;

	current = list->head;
	len = list->len;
	while(len--) {
		next = current->next;
		if (list->free) list->free(current->value);
		q_free(current);
		current = next;
	}
	q_free(list);
}

adlist *listAddNodeHead(adlist *list, void *value)
{
	listNode *node = NULL;

	if ((node = (listNode*)q_malloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (list->len == 0) {
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = NULL;
		node->next = list->head;
		list->head->prev = node;
		list->head = node;
	}
	list->len++;
	return list;
}

adlist *listAddNodeTail(adlist *list, void *value)
{
	listNode *node = NULL;

	if ((node = (listNode*)q_malloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (list->len == 0) {
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = list->tail;
		node->next = NULL;
		list->tail->next = node;
		list->tail = node;
	}
	list->len++;
	return list;
}

adlist *listInsertNode(adlist *list, listNode *old_node, void *value, int32_t after)
{
	listNode *node = NULL;

	if ((node = (listNode*)q_malloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (after) {
		node->prev = old_node;
		node->next = old_node->next;
		if (list->tail == old_node) {
			list->tail = node;
		}
	} else {
		node->next = old_node;
		node->prev = old_node->prev;
		if (list->head == old_node) {
			list->head = node;
		}
	}
	if (node->prev != NULL) {
		node->prev->next = node;
	}
	if (node->next != NULL) {
		node->next->prev = node;
	}
	list->len++;
	return list;
}

void listDelNode(adlist *list, listNode *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else
		list->tail = node->prev;
	if (list->free) list->free(node->value);
	q_free(node);
	list->len--;
}

listIter *listGetIterator(adlist *list, int32_t direction)
{
	listIter *iter = NULL;

	if ((iter = (listIter*)q_malloc(sizeof(*iter))) == NULL)
		return NULL;
	if (direction == AL_START_HEAD)
		iter->next = list->head;
	else
		iter->next = list->tail;
	iter->direction = direction;
	return iter;
}

void listReleaseIterator(listIter *iter)
{
	q_free(iter);
}

void listRewind(adlist *list, listIter *li)
{
	li->next = list->head;
	li->direction = AL_START_HEAD;
}

void listRewindTail(adlist *list, listIter *li)
{
	li->next = list->tail;
	li->direction = AL_START_TAIL;
}

listNode *listNext(listIter *iter)
{
	listNode *current = iter->next;

	if (current != NULL) {
		if (iter->direction == AL_START_HEAD)
			iter->next = current->next;
		else
			iter->next = current->prev;
	}
	return current;
}

adlist *listDup(adlist *orig)
{
	adlist *copy = NULL;
	listIter *iter = NULL;
	listNode *node = NULL;

	if ((copy = listCreate()) == NULL)
		return NULL;
	copy->dup = orig->dup;
	copy->free = orig->free;
	copy->match = orig->match;
	iter = listGetIterator(orig, AL_START_HEAD);
	while((node = listNext(iter)) != NULL) {
		void *value;

		if (copy->dup) {
			value = copy->dup(node->value);
			if (value == NULL) {
				listRelease(copy);
				listReleaseIterator(iter);
				return NULL;
			}
		} else
			value = node->value;
		if (listAddNodeTail(copy, value) == NULL) {
			listRelease(copy);
			listReleaseIterator(iter);
			return NULL;
		}
	}
	listReleaseIterator(iter);
	return copy;
}

listNode *listSearchKey(adlist *list, void *key)
{
	listIter *iter = NULL;
	listNode *node = NULL;

	iter = listGetIterator(list, AL_START_HEAD);
	while((node = listNext(iter)) != NULL) {
		if (list->match) {
			if (list->match(node->value, key)) {
				listReleaseIterator(iter);
				return node;
			}
		} else {
			if (key == node->value) {
				listReleaseIterator(iter);
				return node;
			}
		}
	}
	listReleaseIterator(iter);
	return NULL;
}

listNode *listIndex(adlist *list, long index)
{
	listNode *n = NULL;

	if (index < 0) {
		index = (-index)-1;
		n = list->tail;
		while(index-- && n) n = n->prev;
	} else {
		n = list->head;
		while(index-- && n) n = n->next;
	}
	return n;
}

void listRotate(adlist *list)
{
	listNode *tail = list->tail;

	if (listLength(list) <= 1)
		return;

	/* Detach current tail */
	list->tail = tail->prev;
	list->tail->next = NULL;
	/* Move it as head */
	list->head->prev = tail;
	tail->prev = NULL;
	tail->next = list->head;
	list->head = tail;
}

Q_END_NAMESPACE
