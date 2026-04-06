#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct _Node Node;

struct _Node {
    i64 value;
    Node *next;
    Node *prev;
};

typedef struct {
    Node *head;
    Node *tail;
} Linked;

Linked *ll_new() {
    Linked *ll = malloc(sizeof(Linked));
    if (!ll)
        return NULL;
    ll->head = NULL;
    ll->tail = NULL;
    return ll;
}

void ll_push_front(Linked *ll, i64 value) {
    Node *chead = ll->head;
    // create the new node
    Node *new = malloc(sizeof(Node));
    new->value = value;
    new->prev = NULL;
    new->next = chead;

    // set the new node as the head
    ll->head = new;

    // update previous head's prev
    if (chead != NULL) {
        chead->prev = new;
    }

    // set the tail to head if the list is empty
    if (ll->tail == NULL) {
        ll->tail = ll->head;
    }
}

// same as push_front but head and tail are swapped
void ll_push_back(Linked *ll, i64 value) {
    Node *ctail = ll->tail;
    Node *new = malloc(sizeof(Node));
    new->value = value;
    new->prev = ctail;
    new->next = NULL;
    ll->tail = new;

    if (ctail != NULL) {
        ctail->next = new;
    }

    if (ll->head == NULL) {
        ll->head = ll->tail;
    }
}

i64 ll_pop_front(Linked *ll) {
    assert(ll->head != NULL);
    Node *old_head = ll->head;
    i64 value = old_head->value;
    ll->head = old_head->next;
    if (ll->head != NULL)
        ll->head->prev = NULL;
    else
        ll->tail = NULL;

    free(old_head);
    return value;
}

i64 ll_pop_back(Linked *ll) {
    assert(ll->tail != NULL);
    Node *old_tail = ll->tail;
    i64 value = old_tail->value;
    ll->tail = old_tail->prev;
    if (ll->tail != NULL)
        ll->tail->next = NULL;
    else
        ll->head = NULL;

    free(old_tail);
    return value;
}

void ll_free(Linked *ll) {
    if (!ll)
        return;
    Node *current = ll->head;
    while (current) {
        Node *tmp = current->next;
        free(current);
        current = tmp;
    }
    free(ll);
}

// zero-indexed
i64 *ll_peek_at(Linked *ll, u64 n) {
    Node *current = ll->head;
    u64 steps = 0;
    while (current && steps < n) {
        current = current->next;
        steps++;
    }
    return current ? &current->value : NULL;
}

int main(void) {
    Linked *list = ll_new();
    ll_push_front(list, 10);
    ll_push_front(list, 20);
    ll_push_front(list, 30);

    ll_push_back(list, 11);
    ll_push_back(list, 21);
    ll_push_back(list, 31);

    for (Node *current = list->head; current != NULL; current = current->next) {
        printf("%ld ", current->value);
    }
    printf("\n");

    i64 *val = ll_peek_at(list, 4);
    assert(val != NULL);
    printf("%ld\n", *val);
    ll_free(list);

    return 0;
}
