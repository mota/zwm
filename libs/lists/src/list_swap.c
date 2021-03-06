#include <stdlib.h>

#include "lists.h"

static void	swap2(t_list *list)
{
  t_elem	*tmp;

  tmp = list->head;
  list->head = list->tail;
  list->tail = tmp;
  list->head->prev = NULL;
  list->head->next = list->tail;
  list->tail->next = NULL;
  list->tail->prev = list->head;
}

void		list_swap_next(t_list *list, t_elem *elem)
{
  t_elem	*next;
  t_elem	*tmp;

  if (list->size > 2)
    {
      if (elem != list->tail)
	{
	  next = elem->next;
	  tmp = elem->prev;
	  elem->next = next->next;
	  elem->prev = next;
	  next->next = elem;
	  next->prev = tmp;
	  if (elem == list->head)
	    list->head = next;
	  if (next == list->tail)
	    list->tail = elem;
	}
      else
	{
	  next = list->head;
	  tmp = elem->prev;
	  elem->next = next->next;
	  elem->prev = NULL;
	  next->next = NULL;
	  next->prev = tmp;
	  list->head = elem;
	  list->tail = next;
	}
      if (elem->next != NULL)
	elem->next->prev = elem;
      if (next->prev != NULL)
	next->prev->next = next;
    }
  else if (list->size == 2)
    swap2(list);
}

void		list_swap_prev(t_list *list, t_elem *elem)
{
  t_elem	*prev;
  t_elem	*tmp;

  if (list->size > 2)
    {
      if (elem != list->head)
	{
	  prev = elem->prev;
	  tmp = elem->next;
	  elem->prev = prev->prev;
	  elem->next = prev;
	  prev->prev = elem;
	  prev->next = tmp;
	  if (elem == list->tail)
	    list->tail = prev;
	  if (prev == list->head)
	    list->head = elem;
	}
      else
	{
	  prev = list->tail;
	  tmp = elem->next;
	  elem->prev = prev->prev;
	  elem->next = NULL;
	  prev->prev = NULL;
	  prev->next = tmp;
	  list->head = prev;
	  list->tail = elem;
	}
      if (elem->prev != NULL)
	elem->prev->next = elem;
      if (prev->next != NULL)
	prev->next->prev = prev;
    }
  else if (list->size == 2)
    swap2(list);
}
