/*
 * Copyright (C) 2022-2023 Artinchip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#ifndef _MPP_LIST_H_
#define _MPP_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

struct mpp_list;

struct mpp_list {
	struct mpp_list *next;
	struct mpp_list *prev;
};

static inline void mpp_list_init(struct mpp_list *list)
{
	list->next = list;
	list->prev = list;
}

static inline void mpp_list_add_head(struct mpp_list *elem,
				     struct mpp_list *head)
{
	struct mpp_list *prev = head;
	struct mpp_list *next = head->next;

	assert(elem != NULL);
	assert(head != NULL);

	next->prev = elem;
	elem->next = next;
	elem->prev = prev;
	prev->next = elem;
}

static inline void mpp_list_add_tail(struct mpp_list *elem,
				     struct mpp_list *head)
{
	struct mpp_list *prev = head->prev;
	struct mpp_list *next = head;

	assert(elem != NULL);
	assert(head != NULL);

	next->prev = elem;
	elem->next = next;
	elem->prev = prev;
	prev->next = elem;
}

static inline void mpp_list_del(struct mpp_list *elem)
{
	struct mpp_list *prev = elem->prev;
	struct mpp_list *next = elem->next;

	next->prev = prev;
	prev->next = next;
	elem->next = NULL;
	elem->prev = NULL;
}

static inline void mpp_list_del_init(struct mpp_list *entry)
{
	struct mpp_list *prev = entry->prev;
	struct mpp_list *next = entry->next;

	next->prev = prev;
	prev->next = next;
	entry->next = entry;
	entry->prev = entry;
}

static inline int mpp_list_empty(struct mpp_list *head)
{
	return head->next == head;
}

#define mpp_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ( { \
const typeof( ((type *)0)->member ) *__mptr = (ptr); \
(type *)( (char *)__mptr - mpp_offsetof(type,member) ); } )

#define mpp_list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define mpp_list_first_entry(ptr, type, member) \
	mpp_list_entry((ptr)->next, type, member)

#define mpp_list_first_entry_or_null(ptr, type, member) ({ \
		struct mpp_list *head__ = (ptr); \
		struct mpp_list *pos__ = head__->next; \
		pos__ != head__ ? mpp_list_entry(pos__, type, member) : NULL; \
	})

#define mpp_list_next_entry(pos, member) \
	mpp_list_entry((pos)->member.next, typeof(*(pos)), member)

#define mpp_list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

#define mpp_list_for_each_entry(pos, head, member)				\
	for (pos = mpp_list_first_entry(head, typeof(*pos), member);	\
	     !mpp_list_entry_is_head(pos, head, member);			\
	     pos = mpp_list_next_entry(pos, member))

#define mpp_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = mpp_list_first_entry(head, typeof(*pos), member),	\
		n = mpp_list_next_entry(pos, member);			\
	     !mpp_list_entry_is_head(pos, head, member); 			\
	     pos = n, n = mpp_list_next_entry(n, member))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MPP_LIST_H_ */
