#ifndef GO_PATCH_H
#define GO_PATCH_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

void nop_softerrorf(pid_t pid, char *msg);

#endif