# Ringmaster: Ring Buffer Manager

Disruptor-inspired graph computing.


```c
typedef struct ringmaster_t /* type */
```

Data structure for managing access to ring buffer among a producer
thread and consumer threads.


```c
extern ringmaster_t *ringmaster_create(size_t requested_slots, short n_consumers, consumer *consumers, short consumer_deps[n_consumers][n_consumers]) /* function */
```

Allocate a ringmaster and initialize it to manage a ring buffer of
size `requested_slots`. The ringmaster will coordinate access to the
buffer for `n_consumer` consumer functions, provided in
`consumers`. The dependencies between consumers are modeled in a
two-dimensional array `consumer_deps`, which is an array of dependency
arrays, each dependency array containing zero or more consumer ids
(one-based indexing) referring to consumer functions speciifced in
`consumers`.


```c
extern void ringmaster_start(ringmaster_t *rm) /* function */
```

Spawn a POSIX thread for each consumer and put ringmaster into a
running state.


```c
extern void ringmaster_pause(ringmaster_t *rm) /* function */
```

Put the ringmaster into the paused state. Consumers will spin while
paused.


```c
extern void ringmaster_resume(ringmaster_t *rm) /* function */
```

Put the ringmaster (back) into the running state.


```c
extern void ringmaster_destroy(ringmaster_t *rm) /* function */
```

Signal consumer threads to stop, wait for each to do so, and
deallocate storage for the ringmaster.


```c
extern size_t ringmaster_getslot(ringmaster_t *rm) /* function */
```

Return index of slot to deposit a newly produced datum. Return -1 if
not space is available in the ring buffer.


```c
extern size_t ringmaster_getslot_spin(ringmaster_t *rm) /* function */
```

Return index of slot to desposit a newly produced datum. Spin,
possibly forever, until a slot is available. The producing thread
should prepare the conents of the slot and then call
`ringmaster_advanceslot`.


```c
extern void ringmaster_advanceslot(ringmaster_t *rm) /* function */
```

Indicate to the ringmaster that a datum is in an internally consistent
state suitable for distribution to consumers. Used in conjunction with
`ringmaster_getslot` or `ringmaster_getslot_spin`.


```c
extern bool ringmaster_is_empty(ringmaster_t *rm) /* function */
```

Return a boolean value indicating with there are any unprocessed items
in the ring buffer.


```c
extern void ringmaster_join_spin(ringmaster_t *rm) /* function */
```

Return when there are no unprocessed items in the ring buffer, spinning potentially forever.


```c
CONSUMER(CONSUMER_FUNCTION, SLOT) /* macro */
```

Open a function definition named `CONSUMER_FUNCTION` that manages the
process of acquiring each datum available to be processed. The value
of the slot index at which the datum is available will be stored in a
variable named `SLOT`.


```c
END_CONSUMER /* macro */
```

Close a consumer function definition.


```c
N_CONSUMERS(CS) /* macro */
```

A helper macro to calculate the number of consumers in an array of
`consumer` functions.
