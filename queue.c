#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe = NULL;
    list_for_each_entry_safe (entry, safe, head, list) {
        list_del(&entry->list);
        free(entry->value);
        free(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        return false;

    new_elem->value = strdup(s);
    if (!new_elem->value) {
        free(new_elem);
        return false;
    }

    list_add(&new_elem->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        return false;

    new_elem->value = strdup(s);
    if (!new_elem->value) {
        free(new_elem);
        return false;
    }

    list_add_tail(&new_elem->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *old_head = list_first_entry(head, element_t, list);
    list_del(&old_head->list);

    if (sp) {
        strncpy(sp, old_head->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return old_head;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *old_tail = list_last_entry(head, element_t, list);
    list_del(&old_tail->list);

    if (sp) {
        strncpy(sp, old_tail->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return old_tail;
}


/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int count = 0;
    element_t *entry;
    list_for_each_entry (entry, head, list)
        count++;

    return count;
}

/* Delete the middle node in queue */  // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    int mid_index = q_size(head) / 2;
    int i = 0;
    element_t *entry;

    list_for_each_entry (entry, head, list) {
        if (i == mid_index) {
            list_del(&entry->list);
            q_release_element(entry);
            return true;
        }
        i++;
    }

    return false;
}

/* Delete all nodes that have duplicate string */  // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    element_t *cur, *next = NULL;
    bool removed = false;

    list_for_each_entry_safe (cur, next, head, list) {
        if (&next->list != head && strcmp(cur->value, next->value) == 0) {
            list_del(&cur->list);
            q_release_element(cur);
            removed = true;
        }
    }
    return removed;
}

/* Swap every two adjacent nodes */  // https://leetcode.com/problems/swap-nodes-in-pairs/
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *cur = head->next, *next;

    while (cur != head && cur->next != head) {
        next = cur->next;
        list_move(cur, next->next);
        cur = next->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    element_t *cur, *next = NULL;
    list_for_each_entry_safe (cur, next, head, list)
        list_move(&cur->list, head);
}

/* Reverse the nodes of the list k at a time */  // https://leetcode.com/problems/reverse-nodes-in-k-group/
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head) || k <= 1)
        return;

    struct list_head *cur = head->next, *group_head = head;
    int count = 0;

    while (cur != head) {
        count++;
        struct list_head *next = cur->next;

        if (count % k == 0) {
            struct list_head *tmp = group_head->next;
            while (tmp != next) {
                struct list_head *move = tmp;
                tmp = tmp->next;
                list_move_tail(move, group_head);
            }
            group_head = cur;
        }
        cur = next;
    }
}

static void merge_sorted_lists(struct list_head *left,
                               struct list_head *right,
                               struct list_head *head,
                               bool descend)
{
    LIST_HEAD(sorted);

    while (!list_empty(left) && !list_empty(right)) {
        element_t *left_node = list_first_entry(left, element_t, list);
        element_t *right_node = list_first_entry(right, element_t, list);

        if ((descend && strcmp(left_node->value, right_node->value) > 0) ||
            (!descend && strcmp(left_node->value, right_node->value) <= 0)) {
            list_move_tail(&left_node->list, &sorted);
        } else {
            list_move_tail(&right_node->list, &sorted);
        }
    }

    list_splice_tail_init(left, &sorted);
    list_splice_tail_init(right, &sorted);
    list_splice(&sorted, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    int mid_size = q_size(head) / 2;
    struct list_head *mid = head->next;
    for (int i = 0; i < mid_size - 1; i++)
        mid = mid->next;

    LIST_HEAD(left_part);
    list_cut_position(&left_part, head, mid);

    q_sort(&left_part, descend);
    q_sort(head, descend);
    merge_sorted_lists(&left_part, head, head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
