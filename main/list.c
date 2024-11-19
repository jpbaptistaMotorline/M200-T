#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "stdio.h"

/* Creates a list (node) and returns it
 * Arguments: The data the list will contain or NULL to create an empty
 * list/node
 */
list_node *list_create(void *data, void *expression_data)
{
	list_node *l = malloc(sizeof(list_node));
	if (l != NULL)
	{
		l->next = NULL;
		l->data = data;
		l->cron_expression = expression_data;
	}

	return l;
}

/* Completely destroys a list
 * Arguments: A pointer to a pointer to a list
 */
void list_destroy(list_node **list)
{
	//*list = (*list)->next;
	if (*list == NULL)
		return;
	while (*list != NULL)
	{
		list_remove_node(list, *list);
	}
}

/* Creates a list node and inserts it after the specified node
 * Arguments: A node to insert after and the data the new node will contain
 */
list_node *list_insert_after1(list_node *node, void *expression_data, void *data)
{
	list_node *new_node = list_create(data, expression_data);
	if (new_node)
	{
		new_node->next = node->next;
		node->next = new_node;
	}
	return new_node;
}

/* Creates a new list node and inserts it in the beginning of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node *list_insert_beginning(list_node *list, void *expression_data, void *data)
{
	list_node *new_node = list_create(data, expression_data);
	if (new_node != NULL)
	{
		new_node->next = list;
	}
	return new_node;
}

/* Creates a new list node and inserts it at the end of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node *list_insert_end(list_node *list, void *expression_data, void *data)
{
	list_node *new_node = list_create(data, expression_data);
	if (new_node != NULL)
	{
		for (list_node *it = list; it != NULL; it = it->next)
		{
			if (it->next == NULL)
			{
				it->next = new_node;
				break;
			}
		}
	}
	return new_node;
}

/* Removes a node from the list
 * Arguments: The list and the node that will be removed
 */
void list_remove_node(list_node **list, list_node *node)
{
	list_node *tmp = NULL;
	if (list == NULL || *list == NULL || node == NULL)
		return;


	if (*list == node)
	{
		*list = (*list)->next;
		//////printf("\nlist destroy 11\n");
		/* if (node->cron_expression != NULL)
		{
			//////printf("\nlist erasee cron - %s\n",node->cron_expression);
			free(node->cron_expression);
		}

		if (node->data != NULL)
		{
			//////printf("\nlist erasee node->data - %s\n",node->data);
			free(node->data);
		} */
		
		/* free(node->cron_expression);
		//////printf("\nlist destroy 12\n");
		free(node->data);
		//////printf("\nlist destroy 13\n");	 */	
		free(node);
		//////printf("\nlist destroy 14\n");
		node = NULL;
	}
	else
	{
		tmp = *list;
		while (tmp->next && tmp->next != node)
			tmp = tmp->next;
		if (tmp->next)
		{
			tmp->next = node->next;
			//////printf("\nlist destroy 21\n");
			free(node->cron_expression);
			//////printf("\nlist destroy 22\n");
			free(node->data);
			//////printf("\nlist destroy 23\n");			
			free(node);
			//////printf("\nlist destroy 24\n");
			node = NULL;
		}
	}
}

/* Removes an element from a list by comparing the data pointers
 * Arguments: A pointer to a pointer to a list and the pointer to the data
 */
void list_remove_by_data(list_node **list, void *data)
{
	if (list == NULL || *list == NULL || data == NULL)
		return;
	list_remove_node(list, list_find_by_data(*list, data));
}

/* Find an element in a list by the pointer to the element
 * Arguments: A pointer to a list and a pointer to the node/element
 */
list_node *list_find_node(list_node *list, list_node *node)
{
	while (list)
	{
		if (list == node)
			break;
		list = list->next;
	}
	return list;
}

/* Finds an elemt in a list by the data pointer
 * Arguments: A pointer to a list and a pointer to the data
 */
list_node *list_find_by_data(list_node *list, void *data)
{
	while (list)
	{
		if (list->data == data)
			break;
		list = list->next;
	}
	return list;
}

/* Finds an element in the list by using the comparison function
 * Arguments: A pointer to a list, the comparison function and a pointer to the
 * data
 */
list_node *list_find(list_node *list, int (*func)(list_node *, void *), void *data)
{
	if (!func)
		return NULL;
	while (list)
	{
		if (func(list, data))
			break;
		list = list->next;
	}
	return list;
}

list_node *list_print(list_node *list)
{
	while (list)
	{
		//////printf("\n list - %s - %s\n", list->data, list->cron_expression);
		list = list->next;
	}
	return list;
}
