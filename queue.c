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
    bool duped = false;

    list_for_each_entry_safe (cur, next, head, list) {
        if (&next->list != head && duped &&
            strcmp(cur->value, next->value) != 0) {
            list_del(&cur->list);
            q_release_element(cur);
            duped = false;
            continue;
        }
        if (&next->list != head && strcmp(cur->value, next->value) == 0) {
            list_del(&cur->list);
            q_release_element(cur);
            removed = true;
            duped = true;
        }
    }
    if (duped) {
        q_remove_tail(head, NULL, 0);
    }
    return removed;
}

/* Swap every two adjacent nodes */  // https://leetcode.com/problems/swap-nodes-in-pairs/
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *cur = head->next;

    while (cur != head && cur->next != head) {
        struct list_head *next = cur->next;
        list_move(cur, next);
        cur = cur->next;
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

static void merge_two_sorted_lists(struct list_head *l1,
                                   struct list_head *l2,
                                   bool descend)
{
    if (!l1 || !l2 || list_empty(l1) || list_empty(l2))
        return;

    struct list_head tmp;
    INIT_LIST_HEAD(&tmp);

    while (!list_empty(l1) && !list_empty(l2)) {
        element_t *first = list_first_entry(l1, element_t, list);
        element_t *second = list_first_entry(l2, element_t, list);

        if ((descend && strcmp(first->value, second->value) > 0) ||
            (!descend && strcmp(first->value, second->value) < 0)) {
            list_move_tail(&first->list, &tmp);
        } else {
            list_move_tail(&second->list, &tmp);
        }
    }

    list_splice_tail_init(l1, &tmp);
    list_splice_tail_init(l2, &tmp);
    list_splice_init(&tmp, l1);
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
    merge_two_sorted_lists(head, &left_part, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */ // https://leetcode.com/problems/remove-nodes-from-linked-list/
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    element_t *cur, *safe = NULL;
    list_for_each_entry_safe (cur, safe, head, list) {
        if (&safe->list != head && strcmp(cur->value, safe->value) < 0) {
            list_del(&cur->list);
            q_release_element(cur);
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */ // https://leetcode.com/problems/remove-nodes-from-linked-list/
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *cur = head->prev;
    const char *max_value = list_entry(cur, element_t, list)->value;

    while (cur != head) {
        struct list_head *prev = cur->prev;
        element_t *entry = list_entry(cur, element_t, list);

        if (strcmp(entry->value, max_value) < 0) {
            list_del(&entry->list);
            q_release_element(entry);
        } else {
            max_value = entry->value;
        }
        cur = prev;
    }
    return q_size(head);
}


/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return q_size(head);

    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);

    queue_contex_t *cur, *safe = NULL;
    list_for_each_entry_safe (cur, safe, head, chain) {
        if (cur == first)
            continue;
        merge_two_sorted_lists(first->q, cur->q, descend);
        INIT_LIST_HEAD(cur->q);
    }

    return q_size(first->q);
}
