#include "util.h"
#include <sys/ptrace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

ssize_t ptrace_read(pid_t pid, void *addr, uint8_t buf[], size_t count) {
  if (count % WORD_SIZE != 0) {
    fprintf(stderr, "Count [%ld] must be a multiple of word size!", count);
    exit(1);
  }
  size_t i;
  word_t word;
  for (i = 0; i < count; i += WORD_SIZE) {
    word.w = ptrace(PTRACE_PEEKDATA, pid, addr + i, 0);
    memcpy(buf + i, word.bytes, WORD_SIZE);
  }
  return i >= count ? i : -1;
}

ssize_t ptrace_write(pid_t pid, void *addr, uint8_t buf[], size_t count) {
  if (count % WORD_SIZE != 0) {
    fprintf(stderr, "Count [%ld] must be a multiple of word size!", count);
    exit(1);
  }
  size_t i;
  word_t word;
  for (i = 0; i < count; i += WORD_SIZE) {
    memcpy(word.bytes, buf + i, WORD_SIZE);
    ptrace(PTRACE_POKEDATA, pid, addr + i, word.w);
  }
  return i >= count ? i : -1;
}