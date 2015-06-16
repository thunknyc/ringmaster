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

static ringmgr_t *ringmgr_alloc(unsigned long n_slots,
                                short n_consumers) {

  ringmgr_t *rm = malloc(sizeof(ringmgr_t));
  assert(rm != NULL);

  rm->consumer_slots = calloc(1, n_consumers * sizeof(unsigned long));
  assert(rm->consumer_slots);

  rm->threads = calloc(1, n_consumers * sizeof(pthread_t));
  assert(rm->threads);

  return rm;
}

ringmgr_t *ringmgr_make(unsigned long requested_slots,
                      short n_consumers,
                      consumer *consumers,
                      bool consumer_deps[n_consumers][n_consumers]) {

  unsigned long n_slots = largest_pow_2(requested_slots);

  ringmgr_t *rm = ringmgr_alloc(n_slots, n_consumers);
  assert(rm != NULL);

  rm->consumers = consumers;
  // rm->threads
  // rm->slots
  // rm->consumer_slots
  rm->consumer_deps = consumer_deps;
  rm->head = 0;
  rm->tail = 0;
  rm->n_slots = n_slots;
  rm->n_slots_mask = n_slots - 1;
  rm->n_consumers = n_consumers;
  rm->state = PAUSED;

  return rm;
}

void ringmgr_start(ringmgr_t *rm) {
  for (int i = 0; i < rm->n_consumers; i++) {
    consumer_args *args = malloc(sizeof(consumer_args));
    assert(args != NULL);
    // Ownership of args is passed to consumer function.

    args->rm = rm;
    args->consumer = i;

    int result = pthread_create(&rm->threads[i], NULL, rm->consumers[i],
                                (void *)args);
    assert(result == 0);
  }
  rm->state = ACTIVE;
}

void ringmgr_pause(ringmgr_t *rm) {
  rm->state = PAUSED;
}

void ringmgr_resume(ringmgr_t *rm) {
  rm->state = ACTIVE;
}

static void stop_threads(int n, pthread_t *threads) {
  for(int i = 0; i < n; i++)
    assert(pthread_join(threads[i], NULL) == 0);
}

void ringmgr_destroy(ringmgr_t *rm) {
  // rm->consumers belongs to someone else
  // rm->consumer_deps belong to someone else
  rm->state = STOPPED;
  stop_threads(rm->n_consumers, rm->threads);
  free(rm->threads);
  free((void *)rm->consumer_slots);
  free(rm);
}

static long long new_head(ringmgr_t *rm) {
  unsigned long h = (rm->head + 1) & rm->n_slots_mask;
  return (h == rm->tail) ? -1 : h;
}

static void advance_tail(ringmgr_t *rm) {

  // If buffer is empty do nothing.
  if (rm->tail == rm->head)
    return;

  size_t is[rm->n_consumers];

  {
    size_t *isp = is;
    volatile size_t *csp = rm->consumer_slots;
    for (int i = 0; i < rm->n_consumers; i++, isp++, csp++)
      *isp = (*csp + rm->n_slots - rm->tail) & rm->n_slots_mask;
  }

  unsigned long advance = rm->n_slots_mask;

  {
    unsigned long *isp = is;
    for (int i = 0; i < rm->n_consumers; i++, isp++)
      if (*isp < advance)
        advance = *isp;
  }

  rm->tail = (rm->tail + advance) & rm->n_slots_mask;
}

size_t ringmgr_getslot(ringmgr_t *rm) {

  advance_tail(rm);

  if (rm->state != ACTIVE) return -1;

  long long h = new_head(rm);
  if (h == -1) return -1;

  return rm->head;
}

size_t ringmgr_getslot_spin(ringmgr_t *rm) {
  long long slot;
  while((slot = ringmgr_getslot(rm)) == -1);
  return (size_t)slot;
}

void ringmgr_advanceslot(ringmgr_t *rm) {
  rm->head = new_head(rm);
}

long long ringmgr_slotavailable_priv(ringmgr_t *rm, unsigned short consumer) {
  unsigned long slot = rm->consumer_slots[consumer];
  int n_consumers = rm->n_consumers;

  if (rm->head == slot)
    return -1;

  {
    volatile unsigned long *csp = rm->consumer_slots;
    bool *cdp = ((bool(*)[rm->n_consumers])rm->consumer_deps)[consumer];

    for (int i = 0; i < n_consumers; i++, csp++, cdp++)
      if (*cdp && *csp == slot)
        return -1;
  }

  return slot;
}

void ringmgr_finishslot_priv(ringmgr_t *rm, unsigned short consumer) {
  rm->consumer_slots[consumer] =
    rm->consumer_slots[consumer] + 1 & rm->n_slots_mask;
}

bool ringmgr_is_empty(ringmgr_t *rm) {
  return rm->head == rm->tail;
}

void ringmgr_join_spin(ringmgr_t *rm) {
  for (; !ringmgr_is_empty(rm);)
    advance_tail(rm);
}

