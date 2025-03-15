#include "proc.h"
#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static hashtable *PROCS = NULL;

proc_t *proc_new(pid_t pid, char *filename) {
  proc_t *proc = malloc(sizeof(proc_t));
  *proc = (proc_t){ 
    .pid = pid, 
    .filename = strdup(filename), 
  };
  return proc;
}

proc_t *proc_get(pid_t pid) {
  if (PROCS == NULL) {
    return NULL;
  }
  kv proc = hash_getv(PROCS, KV(.uint64 = pid));
  return proc.ptr;
}

void proc_add(proc_t *proc) {
  if (PROCS == NULL) {
    PROCS = hash_new(1024, hash_int, hash_cmp_int);
  }
  hash_set(PROCS, KV(.uint64 = proc->pid), KV(.ptr = proc));
}

void proc_free(proc_t *proc) {
  free(proc->filename);
  free(proc);
}

void proc_print(proc_t *proc) {
  printf("PID: [%d], FILENAME: [%s]\n", proc->pid, proc->filename);
}