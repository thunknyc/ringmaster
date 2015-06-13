#include "obstructor.h"
#include <assert.h>
#include <stdbool.h>

static unsigned long next_nat_pow_2(unsigned long n) {
  unsigned long x = 0;

  if (!n--) return 1;

  while(n) {
    n >>= 1;
    x = (x << 1) | 1;
  }
  return ++x;
}

static obstructor *alloc_obstructor(unsigned long n_slots,
                                    short n_consumers) {

  obstructor *o = malloc(sizeof(struct obstructor));
  assert(o != NULL);

  o->slots = calloc(1, n_slots * sizeof(void*));
  assert(o->slots != NULL);

  o->consumer_slots = calloc(1, n_consumers * sizeof(unsigned long));
  assert(o->consumer_slots);

  o->threads = calloc(1, n_consumers * sizeof(pthread_t));
  assert(o->threads);

  return o;
}

obstructor *make_obstructor(unsigned long requested_slots,
                            short n_consumers,
                            consumer *consumers,
                            bool consumer_deps[n_consumers][n_consumers]) {

  unsigned long n_slots = next_nat_pow_2(requested_slots);

  obstructor *o = alloc_obstructor(n_slots, n_consumers);
  assert(o != NULL);

  o->consumers = consumers;
  // o->threads
  // o->slots
  // o->consumer_slots
  o->consumer_deps = consumer_deps;
  o->head = 0;
  o->tail = 0;
  o->n_slots = n_slots;
  o->n_slots_mask = n_slots - 1;
  o->n_consumers = n_consumers;
  o->state = PAUSED;

  return o;
}

void start_obstructor(obstructor *o) {
  for (int i = 0; i < o->n_consumers; i++) {
    consumer_args *args = malloc(sizeof(consumer_args));
    assert(args != NULL);
    // Ownership of args is passed to consumer function.

    args->o = o;
    args->consumer = i;

    int result = pthread_create(&o->threads[i], NULL, o->consumers[i],
                                (void *)args);
    assert(result == 0);
  }
  o->state = ACTIVE;
}

void pause_obstructor(obstructor *o) {
  o->state = PAUSED;
}

void resume_obstructor(obstructor *o) {
  o->state = ACTIVE;
}

static void stop_threads(int n, pthread_t *threads) {
  for(int i = 0; i < n; i++)
    assert(pthread_join(threads[i], NULL) == 0);
}

void destroy_obstructor(obstructor *o) {
  // o->consumers belongs to someone else
  // o->consumer_deps belong to someone else
  o->state = STOPPED;
  stop_threads(o->n_consumers, o->threads);
  free(o->threads);
  free(o->consumer_slots);
  free(o->slots);
  free(o);
}

static long long new_head(obstructor *o) {
  unsigned long h = (o->head + 1) & o->n_slots_mask;
  return (h == o->tail) ? -1 : h;
}

static void advance_tail(obstructor *o) {

  // If buffer is empty do nothing.
  if (o->tail == o->head)
    return;

  unsigned long is[o->n_consumers];

  {
    unsigned long *isp = is;
    unsigned long *csp = o->consumer_slots;
    for (int i = 0; i < o->n_consumers; i++, isp++, csp++)
      *isp = (*csp + o->n_slots - o->tail) & o->n_slots_mask;
  }

  unsigned long advance = o->n_slots_mask;

  {
    unsigned long *isp = is;
    for (int i = 0; i < o->n_consumers; i++, isp++)
      if (*isp < advance)
        advance = *isp;
  }

  o->tail = (o->tail + advance) & o->n_slots_mask;
}

bool provide_obstructor(obstructor *o, void *v) {

  advance_tail(o);

  if (o->state != ACTIVE) return false;

  long long h = new_head(o);
  if (h == -1) return false;

  unsigned long new_head = (unsigned long)h;
  o->slots[o->head] = v;
  o->head = new_head;
  return true;
}

void provide_obstructor_block(obstructor *o, void *v) {
  while(!provide_obstructor(o, v));
}

long long datum_available_private(obstructor *o, unsigned short consumer) {
  unsigned long slot = o->consumer_slots[consumer];
  int n_consumers = o->n_consumers;

  if (o->head == slot)
    return -1;

  {
    unsigned long *csp = o->consumer_slots;
    bool *cdp = ((bool(*)[o->n_consumers])o->consumer_deps)[consumer];

    for (int i = 0; i < n_consumers; i++, csp++, cdp++)
      if (*cdp && *csp == slot)
        return -1;
  }

  return slot;
}

void finish_datum_private(obstructor *o, unsigned short consumer) {
  o->consumer_slots[consumer] =
    o->consumer_slots[consumer] + 1 & o->n_slots_mask;
}

bool obstructor_is_empty(obstructor *o) {
  return o->head == o->tail;
}

void join_obstructor(obstructor *o) {
  for (; !obstructor_is_empty(o);)
    advance_tail(o);
}

