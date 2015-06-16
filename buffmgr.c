#include "buffmgr.h"
#include <assert.h>
#include <stdbool.h>

static unsigned long largest_pow_2(unsigned long n) {
  unsigned long v = 1;

  if(!n) return 0;

  for(n >>= 1; n; n >>= 1)
    v = (v << 1);

  return v;
}

static buffmgr *buffmgr_alloc(unsigned long n_slots,
                              short n_consumers) {

  buffmgr *bm = malloc(sizeof(struct buffmgr));
  assert(bm != NULL);

  bm->consumer_slots = calloc(1, n_consumers * sizeof(unsigned long));
  assert(bm->consumer_slots);

  bm->threads = calloc(1, n_consumers * sizeof(pthread_t));
  assert(bm->threads);

  return bm;
}

buffmgr *buffmgr_make(unsigned long requested_slots,
                      short n_consumers,
                      consumer *consumers,
                      bool consumer_deps[n_consumers][n_consumers]) {

  unsigned long n_slots = largest_pow_2(requested_slots);

  buffmgr *bm = buffmgr_alloc(n_slots, n_consumers);
  assert(bm != NULL);

  bm->consumers = consumers;
  // bm->threads
  // bm->slots
  // bm->consumer_slots
  bm->consumer_deps = consumer_deps;
  bm->head = 0;
  bm->tail = 0;
  bm->n_slots = n_slots;
  bm->n_slots_mask = n_slots - 1;
  bm->n_consumers = n_consumers;
  bm->state = PAUSED;

  return bm;
}

void buffmgr_start(buffmgr *bm) {
  for (int i = 0; i < bm->n_consumers; i++) {
    consumer_args *args = malloc(sizeof(consumer_args));
    assert(args != NULL);
    // Ownership of args is passed to consumer function.

    args->bm = bm;
    args->consumer = i;

    int result = pthread_create(&bm->threads[i], NULL, bm->consumers[i],
                                (void *)args);
    assert(result == 0);
  }
  bm->state = ACTIVE;
}

void buffmgr_pause(buffmgr *bm) {
  bm->state = PAUSED;
}

void buffmgr_resume(buffmgr *bm) {
  bm->state = ACTIVE;
}

static void stop_threads(int n, pthread_t *threads) {
  for(int i = 0; i < n; i++)
    assert(pthread_join(threads[i], NULL) == 0);
}

void buffmgr_destroy(buffmgr *bm) {
  // bm->consumers belongs to someone else
  // bm->consumer_deps belong to someone else
  bm->state = STOPPED;
  stop_threads(bm->n_consumers, bm->threads);
  free(bm->threads);
  free((void *)bm->consumer_slots);
  free(bm);
}

static long long new_head(buffmgr *bm) {
  unsigned long h = (bm->head + 1) & bm->n_slots_mask;
  return (h == bm->tail) ? -1 : h;
}

static void advance_tail(buffmgr *bm) {

  // If buffer is empty do nothing.
  if (bm->tail == bm->head)
    return;

  size_t is[bm->n_consumers];

  {
    size_t *isp = is;
    volatile size_t *csp = bm->consumer_slots;
    for (int i = 0; i < bm->n_consumers; i++, isp++, csp++)
      *isp = (*csp + bm->n_slots - bm->tail) & bm->n_slots_mask;
  }

  unsigned long advance = bm->n_slots_mask;

  {
    unsigned long *isp = is;
    for (int i = 0; i < bm->n_consumers; i++, isp++)
      if (*isp < advance)
        advance = *isp;
  }

  bm->tail = (bm->tail + advance) & bm->n_slots_mask;
}

size_t buffmgr_getslot(buffmgr *bm) {

  advance_tail(bm);

  if (bm->state != ACTIVE) return -1;

  long long h = new_head(bm);
  if (h == -1) return -1;

  return bm->head;
}

size_t buffmgr_getslot_spin(buffmgr *bm) {
  long long slot;
  while((slot = buffmgr_getslot(bm)) == -1);
  return (size_t)slot;
}

void buffmgr_advanceslot(buffmgr *bm) {
  bm->head = new_head(bm);
}

long long buffmgr_slotavailable_priv(buffmgr *bm, unsigned short consumer) {
  unsigned long slot = bm->consumer_slots[consumer];
  int n_consumers = bm->n_consumers;

  if (bm->head == slot)
    return -1;

  {
    volatile unsigned long *csp = bm->consumer_slots;
    bool *cdp = ((bool(*)[bm->n_consumers])bm->consumer_deps)[consumer];

    for (int i = 0; i < n_consumers; i++, csp++, cdp++)
      if (*cdp && *csp == slot)
        return -1;
  }

  return slot;
}

void buffmgr_finishslot_priv(buffmgr *bm, unsigned short consumer) {
  bm->consumer_slots[consumer] =
    bm->consumer_slots[consumer] + 1 & bm->n_slots_mask;
}

bool buffmgr_is_empty(buffmgr *bm) {
  return bm->head == bm->tail;
}

void buffmgr_join_spin(buffmgr *bm) {
  for (; !buffmgr_is_empty(bm);)
    advance_tail(bm);
}

