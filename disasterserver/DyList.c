#include <DyList.h>
#include <stdlib.h>

bool dylist_create(DyList* list, size_t max_capacity)
{
	if (!list)
		return false;

	memset(list, 0, sizeof(DyList));
	list->capacity = max_capacity;
	list->noitems = 0;
	list->ptr = (void**)malloc(list->capacity * sizeof(void*));

	if (!list->ptr)
		return false;

	memset(list->ptr, 0, list->capacity * sizeof(void*));
	Debug("%p DyList allocated (size %d)", list->ptr, list->capacity * sizeof(void*));
	return true;
}

bool dylist_push(DyList* list, void* item)
{
	if (!list)
		return false;

	if (!item)
		return false;

	bool found = false;
	for (size_t i = 0; i < list->capacity; i++)
	{
		if (!list->ptr[i])
		{
			Debug("%p DyList added item %p (index %d)", list->ptr, item, i);
			list->ptr[i] = item;
			found = true;
			break;
		}
	}

	if (!found)
	{
		Warn("%p DyList reached limit (%d items)!", list->ptr, list->capacity);
		return false;
	}

	list->noitems++;
	return true;
}

bool dylist_remove(DyList* list, void* item)
{
	if (!item)
		return false;

	for (size_t i = 0; i < list->capacity; i++)
	{
		if (list->ptr[i] == item)
		{
			Debug("%p DyList removed item %p (index %d)", list->ptr, item, i);
			list->ptr[i] = NULL;
			list->noitems--;
			return true;
		}
	}

	return false;
}

void dylist_free(DyList* list)
{
	if (!list)
		return;

	Debug("%p DyList freed.", list->ptr);
	free(list->ptr);
	memset(list, 0, sizeof(DyList));
}
