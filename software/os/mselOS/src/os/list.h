/** @file list.h

    This is a header-only implementation of a typed doubly-linked
    list. Those familiar with lists in the Linux kernel will pick
    right up on this API.

*/
/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef _MSEL_LIST_H_
#define _MSEL_LIST_H_

#include <stdlib.h>

/* This directive can be defined in order to enable safe unlinking */
#ifdef SAFE_UNLINK

#define MSEL_LIST_ASSERT(expr)			\
{						\
    if((expr) == 0)				\
    {						\
    __asm__ volatile("bkpt 0");			\
    }						\
}(void)0

#else
#define MSEL_LIST_ASSERT(expr) (void)0
#endif

/* NOTE: all arguments in pre-proc macros need to be enclosed in
 * parens when referenced to avoid unexpected precedence issues */

/** @brief used to define a list inside a custom struct definition */
#define MSEL_LIST(type) \
    type *next;		\
    type *prev

/** @brief defines a structure to manage a list of structures with MSEL_LIST items inside it */
#define MSEL_LIST_HEAD(type) \
    struct type##_listhead { \
        type *first;	     \
        type *last;	     \
        size_t len;	     \
    }

/** @brief Initializes a listhead as defined by MSEL_LIST_HEAD */
#define MSEL_LIST_INIT(listhead)		\
    (listhead).first = NULL;			\
    (listhead).last  = NULL;			\
    (listhead).len   = 0

/** @brief Append to end of list */
#define MSEL_LIST_PUSH(head,val)					\
    if(!(head).first)							\
    {									\
	MSEL_LIST_ASSERT(!((head).last));				\
	(head).first = (head).last = &(val);				\
	(head).len = 1;							\
	(val).next = (val).prev = NULL;					\
    } else {								\
	MSEL_LIST_ASSERT((head).first);					\
	MSEL_LIST_ASSERT((head).last);					\
	MSEL_LIST_ASSERT((head).last->next == NULL);			\
	(head).last->next = &(val);					\
        (val).prev = (head).last;					\
	(val).next = NULL;						\
        (head).last = &(val);						\
	(head).len++;							\
    }

/** @brief delete from end of list */
#define MSEL_LIST_POP(head)						\
    if((head).last) {							\
	MSEL_LIST_ASSERT((head).first);					\
	if((head).first == (head).last) {				\
	    MSEL_LIST_ASSERT((head).first->prev == NULL);		\
	    MSEL_LIST_ASSERT((head).first->next == NULL);		\
	    (head).first = (head).last = NULL;				\
	    (head).len = 0;						\
	} else {							\
	    MSEL_LIST_ASSERT((head).last->next == NULL);		\
	    MSEL_LIST_ASSERT((head).last->prev);			\
	    MSEL_LIST_ASSERT((head).last->prev->next == (head).last);	\
	    (head).last = (head).last->prev;				\
	    (head).last->next = NULL;					\
	    (head).len--;						\
	}								\
    }

/** @brief delete from beginning of list */
#define MSEL_LIST_SHIFT(head)						\
    if((head).first) {							\
	MSEL_LIST_ASSERT((head).last);					\
	if((head).first == (head).last) {				\
	    MSEL_LIST_ASSERT((head).first->prev == NULL);		\
	    MSEL_LIST_ASSERT((head).first->next == NULL);		\
	    (head).first = (head).last = NULL;				\
	    (head).len = 0;						\
	} else {							\
	    MSEL_LIST_ASSERT((head).first->prev == NULL);		\
	    MSEL_LIST_ASSERT((head).first->next);			\
	    MSEL_LIST_ASSERT((head).first->next->prev == (head).first);	\
	    (head).first = (head).first->next;				\
	    (head).first->prev = NULL;					\
	    (head).len--;						\
	}								\
    }

/** @brief append to beginning of list */
#define MSEL_LIST_UNSHIFT(head,val)					\
    if(!(head).last)							\
    {									\
	MSEL_LIST_ASSERT(!(head).first);				\
	(head).first = (head).last = &(val);				\
	(head).len = 1;							\
	(val).next = (val).prev = NULL;					\
    } else {								\
        MSEL_LIST_ASSERT((head).first);					\
        MSEL_LIST_ASSERT((head).last);					\
	MSEL_LIST_ASSERT((head).first->prev == NULL);			\
	(head).first->prev = &(val);					\
        (val).next = (head).first;					\
	(val).prev = NULL;						\
        (head).first = &(val);						\
	(head).len++;							\
    }

/** @brief iterate over the entire list */
#define MSEL_LIST_FOREACH(head,ptr)				\
    for(typeof ((head).first) ptr=(head).first;ptr;ptr=ptr->next)

/** @brief delete an item via pointer */
#define MSEL_LIST_DELETE(head,tgt_ptr)		        \
    MSEL_LIST_FOREACH((head),(item)) {		        \
        if((item) == (tgt_ptr)) {			\
            if((item)->next) {				\
	        (item)->next->prev = (item)->prev;	\
	    } else {		        		\
		(head).last = (item)->prev;		\
	    }					        \
	    if((item)->prev) {		        	\
		(item)->prev->next = (item)->next;	\
	    } else {				        \
		(head).first = (item)->next;		\
	    }						\
	    (head).len--;                               \
            break;				        \
        }                                               \
    } 


#endif

/* 

Usage example / Unit tests 
=============================================================

#include <stdio.h>

typedef struct _mytype {
    int value;
    MSEL_LIST(struct _mytype);
} mytype;

int main() 
{
    MSEL_LIST_HEAD(mytype) listhead;
    MSEL_LIST_INIT(listhead);

    mytype a,b,c,d;
    a.value = 1;
    b.value = 2;
    c.value = 3;
    d.value = 4;

    MSEL_LIST_PUSH(listhead,a);
    MSEL_LIST_PUSH(listhead,b);

    MSEL_LIST_FOREACH(listhead,item) {x
	printf("%u ",item->value);
    };
    printf("\n");

    MSEL_LIST_POP(listhead);
    MSEL_LIST_UNSHIFT(listhead,c);

    MSEL_LIST_FOREACH(listhead,item) {
	printf("%d ",item->value);
    };
    printf("\n");

    MSEL_LIST_SHIFT(listhead);
    MSEL_LIST_UNSHIFT(listhead,d);

    MSEL_LIST_FOREACH(listhead,item) {
	printf("%d ",item->value);
    };
    printf("\n");

    MSEL_LIST_POP(listhead);
    MSEL_LIST_SHIFT(listhead);

    MSEL_LIST_FOREACH(listhead,item) {
	printf("%d ",item->value);
    };
    printf("\n");

}
*/
