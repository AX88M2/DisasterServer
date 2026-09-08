#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <Log.h>

typedef struct
{
	void** ptr;
	size_t noitems;
	size_t capacity;
} DyList;

bool		dylist_create(DyList* list, size_t max_capacity);
bool		dylist_push(DyList* list, void* item);
bool		dylist_remove(DyList* list, void* item);
void		dylist_free(DyList* list);

#endif