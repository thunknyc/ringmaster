#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obstructor.h"

#define LOG(...) fprintf(stderr, __VA_ARGS__)

#define IN_BUFFER_SIZE 1024
#define OUT_BUFFER_SIZE 1024

typedef struct in_event {
  char *s;
  long long a,b,sum;
} in_event;

typedef struct out_event {
  char *message;
} out_event;

obstructor *in, *out;
FILE *out_file;

in_event *make_ievent(long i) {
  in_event *e = calloc(1, sizeof(in_event));
  assert(e != NULL);
  asprintf(&e->s, "%ld, %ld", i, i * 2);
  assert(e->s != NULL);
  return e;
}

CONSUMER(parse_ievent, in_event *, event) {
  char *sep = ", ";
  char *tok, *last;
  char *as = strtok_r(event->s, sep, &last);
  char *bs = strtok_r(NULL, sep, &last);
  event->a = atoll(as);
  event->b = atoll(bs);
} END_CONSUMER

CONSUMER(sum_ievent, in_event *, event) {
  event->sum = event->a + event->b;
} END_CONSUMER

CONSUMER(jsonify_ievent, in_event *, event) {
  char *json;
  asprintf(&json, "{\"a\": %lld, \"b\": %lld, \"sum\": %lld}\n",
           event->a, event->b, event->sum);
  assert(json != NULL);

  out_event *oevent = calloc(1, sizeof(out_event));
  assert(oevent != NULL);

  oevent->message = json;
  provide_obstructor_block(out, oevent);
} END_CONSUMER

CONSUMER(destroy_ievent, in_event *, event) {
  long long sum = event->sum;

  free(event->s);
  free(event);
} END_CONSUMER

CONSUMER(write_oevent, out_event *, event) {
  assert(event->message != NULL);
  fputs(event->message, out_file);
} END_CONSUMER

CONSUMER(destroy_oevent, out_event *, event) {
  free(event->message);
  free(event);
} END_CONSUMER

int main(int argc, char **argv) {

  consumer in_consumers[] = {parse_ievent, sum_ievent,
                             jsonify_ievent, destroy_ievent};

  bool in_deps[4][4] = {{false, false, false, false},
                        {true, false, false, false},
                        {false, true, false, false},
                        {false, false, true, false}};

  in = make_obstructor(IN_BUFFER_SIZE, N_CONSUMERS(in_consumers),
                       in_consumers, in_deps);

  consumer out_consumers[] = {write_oevent, destroy_oevent};

  bool out_deps[2][2] = {{false, false}, {true, false}};

  out = make_obstructor(OUT_BUFFER_SIZE, N_CONSUMERS(out_consumers),
                        out_consumers, out_deps);

  assert(argc == 2);
  assert(in != NULL);
  assert(out != NULL);

  long iterations = atol(argv[1]);
  assert(iterations > 0);

  LOG("iterations: %ld\n", iterations);

  out_file = stdout;

  start_obstructor(in);
  start_obstructor(out);

  for (long i = 0; i < iterations; i++)
    provide_obstructor_block(in, make_ievent(i));

  join_obstructor(in);
  destroy_obstructor(in);

  join_obstructor(out);
  destroy_obstructor(out);

  return 0;
}
