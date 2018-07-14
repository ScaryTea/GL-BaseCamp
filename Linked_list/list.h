#include <stdlib.h>

struct list_node {
	struct list_node *prev, *next;
};

#define LIST_NODE_INIT(name) { &(name), &(name) }

#define LIST_NODE(name) \
	struct list_node name = LIST_NODE_INIT(name)

static inline void INIT_LIST_NODE(struct list_node *list)
{
	list->next = list;
	list->prev = list;
}

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * __list_add_valid - checks whether the @new can be added to list.
 * @new:        the pointer to the new &list_node struct to add.
 * @head:       any entry of the linked list.
 *
 * Return:
 * * 0		- @new can be added
 * * 1		- @new is already in a list
 */
static inline int __list_add_valid(struct list_node *new,
				struct list_node *head)
{
	struct list_node *node;
	if (new == head) return 0;
	list_for_each(node, head) {
		if (node == new) return 0;
	}
	return 1;
}

/**
 * __list_add - insert @new between @prev and @next. 
 * @new:        the pointer to the new &list_node struct to add.
 * @prev:       previous entry.
 * @next:	next entry.
 *
 * This function is used as auxilary one for other library functions.
 */
static inline void __list_add(struct list_node *new,
				struct list_node *prev,
				struct list_node *next)
{
	if (!__list_add_valid(new, prev)) {
		return;
	}
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add - add @new after @head. 
 * @new:        the pointer to the new &list_node struct to add.
 * @head:       the entry to add @new after.
 *
 * This function is for user.
 */
static inline void list_add(struct list_node *new, struct list_node *head)
{
	__list_add(new, head, head->next);
}

/**
 * list_add_tail - add @new before @head. 
 * @new:        the pointer to the new &list_node struct to add.
 * @head:       the entry to add @new before.
 *
 * This function is for user.
 */
static inline void list_add_tail(struct list_node *new, struct list_node *head)
{
	__list_add(new, head->prev, head);
}

/**
 * list_insert - insert @new to a certain position. 
 * @new:        the pointer to the new &list_node struct to add.
 * @head:       the head of the linked list.
 * @pos:	position at which @new should be.
 *
 */
static inline int list_insert(struct list_node *new,
			struct list_node *head,
			int pos)
{
	struct list_node *node;
	list_for_each(node, head) {
		if (pos-- <= 0) break;
	}
	if (pos > 0) return 1; /* index out of range */
	__list_add(new, node->prev, node);
	return 0;
}

/**
 * list_clear - clears the list. 
 * @head:        the pointer to head of the list.
 *
 * The function makes all nodes of the list(including @head)
 * point to themselves. 
 */
static inline void list_clear(struct list_node *head)
{
	struct list_node *prev, *node = head;
	do {
		prev = node;
		node = node->next;
		INIT_LIST_NODE(prev);
	} while (node != head);
}

/**
 * list_reverse - reverse the order of the list. 
 * @head:        the pointer to the head of the list.
 *
 * The function swaps &prev and &next pointers of every entry.
 */
static inline void list_reverse(struct list_node *head)
{
	struct list_node *tmp, *node = head;
	do {
		tmp = node->prev;
		node->prev = node->next;
		node->next = tmp;
		node = node->next;
	} while (node != head);
}

/**
 * list_remove - removes entry from the list.
 * @target:        the pointer to the entry to remove.
 *
 */
static inline void list_remove(struct list_node *target)
{
	target->prev->next = target->next;
	target->next->prev = target->prev;
	INIT_LIST_NODE(target);
}

/**
 * list_count - counts entries in a list, which are equal to the given one.
 * @target:        the pointer to the new &list_node struct to add.
 * @head:       the pointer to head of the list.
 * @cmp:	compare function.
 *
 * The compare functions compare the data, which is held in the user structure.
 * The compare function must have const void * as its arguments, then
 * cast them to &list_node struct. Compare function must return 0 if two 
 * given structs are equal.
 */
static int list_count(struct list_node *target,
			struct list_node *head,
			int (*cmp)(const void *, const void *))
{
	struct list_node *node;
	int counter = 0;
	list_for_each(node, head) {
		if (!cmp(&node, &target))
			counter++;
	}
	return counter;
}

/**
 * list_sort - sorts the list.
 * @head:       the pointer to head of the list.
 * @cmp:	compare function.
 *
 * The comparison function must return an integer less than, equal to, or  greater
 * than  zero  if  the  first argument is considered to be respectively less than,
 * equal to, or greater than the second. 
 */
static void list_sort(struct list_node *head,
			int (*cmp)(const void *, const void *))
{
	int length = 0;
	struct list_node *node;
	list_for_each(node, head)
		length++;

	struct list_node **nodes = malloc(length * sizeof(nodes));
	int i = 0;
	list_for_each(node, head)
		nodes[i++] = node;

	qsort(nodes, length, sizeof(nodes), cmp);

	list_clear(head);
	while(length > 0)
		list_add(nodes[--length], head);
}
