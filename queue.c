#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q_head = malloc(sizeof(*q_head));
    if (q_head)
        INIT_LIST_HEAD(q_head);
    return q_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_element = malloc(sizeof(*new_element));
    if (!new_element)  // If allocate failed
        return false;
    new_element->value = strdup(s);
    if (!new_element->value) {  // If allocate failed
        q_release_element(new_element);
        return false;
    }
    list_add(&new_element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *rm_element = list_first_entry(head, element_t, list);
    list_del_init(&rm_element->list);
    if (sp) {
        size_t len = strlen(rm_element->value);
        if (bufsize > len) {
            strncpy(sp, rm_element->value, len);
            *(sp + len) = '\0';
        } else {
            strncpy(sp, rm_element->value, bufsize - 1);
            *(sp + bufsize - 1) = '\0';
        }
    }
    return rm_element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int count = 0;
    struct list_head *node;
    list_for_each (node, head)
        count++;
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *fast = head->next, *slow = head->next;
    while (fast->next != head && fast->next->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    element_t *rm_element = list_entry(slow, element_t, list);
    list_del_init(&rm_element->list);
    q_release_element(rm_element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    bool dup = false;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list != head && !strcmp(entry->value, safe->value)) {
            list_del_init(&entry->list);
            q_release_element(entry);
            dup = true;
        } else if (dup) {
            list_del_init(&entry->list);
            q_release_element(entry);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
    return;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    int count = 0;
    LIST_HEAD(new_head);
    struct list_head *node, *safe, *temp = head;
    list_for_each_safe (node, safe, head) {
        if (++count == k) {
            list_cut_position(&new_head, temp, node);
            q_reverse(&new_head);
            list_splice_init(&new_head, temp);
            temp = safe->prev;
            count = 0;
        }
    }
}

void merge_two_list(struct list_head *list1, struct list_head *list2)
{
    if (!list1 || !list2)
        return;
    LIST_HEAD(tmp_head);
    while (!list_empty(list1) && !list_empty(list2)) {
        element_t *list1_element = list_first_entry(list1, element_t, list);
        element_t *list2_element = list_first_entry(list2, element_t, list);
        list_move_tail((strcmp(list1_element->value, list2_element->value) < 0)
                           ? &list1_element->list
                           : &list2_element->list,
                       &tmp_head);
    }
    list_splice_tail_init(list_empty(list1) ? list2 : list1, &tmp_head);
    list_splice(&tmp_head, list1);
}

void merge_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *fast = head->next, *slow = head->next;
    while (fast->next != head && fast->next->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    LIST_HEAD(new_head);
    list_cut_position(&new_head, head, slow);
    merge_sort(head);
    merge_sort(&new_head);
    merge_two_list(head, &new_head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    merge_sort(head);
    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    q_reverse(head);
    element_t *entry, *safe, *limit = list_entry(head->next, element_t, list);
    list_for_each_entry_safe (entry, safe, head, list) {
        if (strcmp(entry->value, limit->value) > 0) {
            list_del_init(&entry->list);
            q_release_element(entry);
        } else {
            limit = entry;
        }
    }
    q_reverse(head);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    q_reverse(head);
    element_t *entry, *safe, *limit = list_entry(head->next, element_t, list);
    list_for_each_entry_safe (entry, safe, head, list) {
        if (strcmp(entry->value, limit->value) < 0) {
            list_del_init(&entry->list);
            q_release_element(entry);
        } else {
            limit = entry;
        }
    }
    q_reverse(head);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head) || list_is_singular(head))
        return q_size(head);
    queue_contex_t *first, *entry;
    first = list_entry(head->next, queue_contex_t, chain);
    list_for_each_entry (entry, head, chain)
        if (first != entry)
            merge_two_list(first->q, entry->q);
    if (descend)
        q_reverse(head);
    return q_size(head);
}
