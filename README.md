# Ringmaster: Ring Buffer Manager

Disruptor-inspired graph computing.

type  
`typedef struct ringmaster_t`

Data structure for managing access to ring buffer among a producer
thread and consumer threads.

type  
`typedef void *(*consumer)(void *arguments)`

Function signature of consumer functions. See `CONSUMER()` for
information on defining consumer functions.

function  
`extern ringmaster_t *ringmaster_make(size_t requested_slots, short n_consumers, consumer *consumers, bool consumer_deps[n_consumers][n_consumers])`

Allocate a ringmaster and initialize it to manage a ring buffer of
size `requested_slots`. The ringmaster will coordinate access to the
buffer for `n_consumer` consumer functions, provided in
`consumers`. The dependencies between consumers are modeled in a
two-dimensional array `consumer_deps`.

function  
`extern void ringmaster_start(ringmaster_t *rm)`

Spawn a POSIX thread for each consumer and put ringmaster into a
running state.

function  
`extern void ringmaster_pause(ringmaster_t *rm)`

Put the ringmaster into the paused state. Consumers will spin while
paused.

function  
`extern void ringmaster_resume(ringmaster_t *rm)`

Put the ringmaster (back) into the running state.

function  
`extern void ringmaster_destroy(ringmaster_t *rm)`

Signal consumer threads to stop, wait for each to do so, and
deallocate storage for the ringmaster.

function  
`extern size_t ringmaster_getslot(ringmaster_t *rm)`

Return index of slot to deposit a newly produced datum. Return -1 if
not space is available in the ring buffer.

function  
`extern size_t ringmaster_getslot_spin(ringmaster_t *rm)`

Return index of slot to desposit a newly produced datum. Spin,
possibly forever, until a slot is available. The producing thread
should prepare the conents of the slot and then call
`ringmaster_advanceslot`.

function  
`extern void ringmaster_advanceslot(ringmaster_t *rm)`

Indicate to the ringmaster that a datum is in an internally consistent
state suitable for distribution to consumers. Used in conjunction with
`ringmaster_getslot` or `ringmaster_getslot_spin`.

function  
`extern bool ringmaster_is_empty(ringmaster_t *rm)`

Return a boolean value indicating with there are any unprocessed items
in the ring buffer.

function  
`extern void ringmaster_join_spin(ringmaster_t *rm)`

Return when there are no unprocessed items in the ring buffer, spinning potentially forever.

macro  
`CONSUMER(CONSUMER_FUNCTION, SLOT)`

Open a function definition named `CONSUMER_FUNCTION` that manages the
process of acquiring each datum available to be processed. The value
of the slot index at which the datum is available will be stored in a
variable named `SLOT`.

macro  
`END_CONSUMER`

Close a consumer function definition.

macro  
`N_CONSUMERS(CS)`

A helper macro to calculate the number of consumers in an array of
`consumer` functions.
