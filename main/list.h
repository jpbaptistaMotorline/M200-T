#ifndef _LIST_H
#define _LIST_H

typedef struct list_node {
	char *cron_expression;
	char *data;
	struct list_node *next;
} list_node;

/* linked list */
list_node* list_create(void *data,void *expression_data);
void list_destroy(list_node **list);
list_node* list_insert_after1(list_node *list, void *expression_data, void *data);
list_node* list_insert_beginning(list_node *list,void *expression_data, void *data);
list_node* list_insert_end(list_node *list, void *expression_data, void *data);
void list_remove_node(list_node **list, list_node *node);
void list_remove_by_data(list_node **list, void *data);
list_node* list_find_node(list_node *list, list_node *node);
list_node* list_find_by_data(list_node *list, void *data);
list_node* list_find(list_node *list, int(*func)(list_node*,void*), void *data);
list_node* list_print(list_node *list);

#endif

