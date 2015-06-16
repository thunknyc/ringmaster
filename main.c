#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffmgr.h"

#define LOG(...) fprintf(stderr, __VA_ARGS__)

#define IN_BUFFER_SIZE 65536
#define MAX_S_SIZE 32
#define OUT_BUFFER_SIZE 65536
#define MAX_JSON_SIZE 64

typedef struct in_event {
  char s[MAX_S_SIZE];
  long a,b,sum;
} in_event;

typedef struct out_event {
  char message[MAX_JSON_SIZE];
} out_event;

buffmgr *in, *out;
FILE *out_file;

in_event ievents[IN_BUFFER_SIZE];
out_event oevents[OUT_BUFFER_SIZE];

void init_ievent(size_t slot, long n) {
  snprintf(ievents[slot].s, MAX_S_SIZE, "%ld, %ld", n, n * 2);
}

CONSUMER(parse_ievent, slot) {
  static char *sep = ", ";
  char *last;
  char *as = strtok_r(ievents[slot].s, sep, &last);
  char *bs = strtok_r(NULL, sep, &last);
  ievents[slot].a = atoll(as);
  ievents[slot].b = atoll(bs);
} END_CONSUMER

CONSUMER(sum_ievent, slot) {
  ievents[slot].sum = ievents[slot].a + ievents[slot].b;
} END_CONSUMER

CONSUMER(jsonify_ievent, slot) {
  size_t slot = buffmgr_getslot_spin(out);
  snprintf(oevents[slot].message, MAX_JSON_SIZE,
           "{\"a\": %ld, \"b\": %ld, \"sum\": %ld}\n",
           ievents[slot].a, ievents[slot].b, ievents[slot].sum);
  buffmgr_advanceslot(out);
} END_CONSUMER

CONSUMER(write_oevent, slot) {
  fputs(oevents[slot].message, out_file);
} END_CONSUMER

int main(int argc, char **argv) {

  consumer in_consumers[] = {parse_ievent, sum_ievent, jsonify_ievent};

  bool in_deps[3][3] = {{false, false, false},
                        {true, false, false},
                        {false, true, false}};

  in = buffmgr_make(IN_BUFFER_SIZE, N_CONSUMERS(in_consumers),
                    in_consumers, in_deps);

  consumer out_consumers[] = {write_oevent};

  bool out_deps[1][1] = {{false}};

  out = buffmgr_make(OUT_BUFFER_SIZE, N_CONSUMERS(out_consumers),
                     out_consumers, out_deps);

  assert(argc == 2);
  assert(in != NULL);
  assert(out != NULL);

  long iterations = atol(argv[1]);
  assert(iterations > 0);

  LOG("iterations: %ld\n", iterations);

  out_file = stdout;

  buffmgr_start(in);
  buffmgr_start(out);

  for (long i = 0; i < iterations; i++) {
    size_t slot = buffmgr_getslot_spin(in);
    init_ievent(slot, i);
    buffmgr_advanceslot(in);
  }
  
  buffmgr_join_spin(in);
  buffmgr_destroy(in);
  
  buffmgr_join_spin(out);
  buffmgr_destroy(out);

  return 0;
}
