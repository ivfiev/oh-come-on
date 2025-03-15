#include <sys/types.h>
#ifndef GO_PROC_H
#define GO_PROC_H

typedef struct {
  pid_t pid;
  char *filename;
} proc_t;

proc_t *proc_new(pid_t pid, char *filename);

proc_t *proc_get(pid_t pid);

void proc_add(proc_t *proc);

void proc_free(proc_t *proc);

void proc_print(proc_t *proc);

#endif