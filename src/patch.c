#include "patch.h"
#include <stdio.h>
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

#define BASE_ADDR 0x400000

static uint8_t COMPILER_VM[16 * 1024 * 1024];
static hashtable *CACHE;

static void load_vm(pid_t pid) {
  ptrace_read(pid, (void *)BASE_ADDR, COMPILER_VM, sizeof(COMPILER_VM));
}

static uint32_t find_str(char *str) {
  size_t len = strlen(str);
  for (int i = 0; i < sizeof(COMPILER_VM) - len; i++) {
    int j;
    for (j = 0; j < len; j++) {
      if (COMPILER_VM[i + j] != str[j]) {
        break;
      }
    }
    if (j == len) {
      return BASE_ADDR + i;
    }
  }
  ERROR("failed to find the string");
}

static uint32_t find_lea(uint32_t addr) {
  const int lea_size = 7;
  word_t w = {.qw = 0};
  for (int i = 0; i < sizeof(COMPILER_VM) - lea_size; i++) {
    if (COMPILER_VM[i] == 0x48 && COMPILER_VM[i + 1] == 0x8d && COMPILER_VM[i + 2] == 0x35) {
      memcpy(w.bytes, COMPILER_VM + i + 3, sizeof(w.dw));
      if ((int)w.dw + i + lea_size + BASE_ADDR == addr) {
        return BASE_ADDR + i + lea_size;
      }
    }
  }
  ERROR("failed to find LEA");
}

static uint32_t find_call(uint32_t lea) {
  const int call_range = 30;
  lea -= BASE_ADDR;
  for (int i = 0; i < call_range; i++) {
    if (COMPILER_VM[lea + i] == 0xe8) {
      return BASE_ADDR + lea + i;
    }
  }
  ERROR("failed to find CALL");
}

static void nop_call(pid_t pid, uint32_t call_addr) {
  uint8_t bytes[WORD_SIZE];
  if (ptrace_read(pid, (void *)(uint64_t)call_addr, bytes, sizeof(bytes)) < WORD_SIZE) {
    ERROR("failed to read the call");
  }
  bytes[0] = 0x90;
  bytes[1] = 0x90;
  bytes[2] = 0x90;
  bytes[3] = 0x90;
  bytes[4] = 0x90;
  if (ptrace_write(pid, (void *)(uint64_t)call_addr, bytes, sizeof(bytes)) < WORD_SIZE) {
    ERROR("failed to overwrite the call");
  }
}

int nop_softerrorf(pid_t pid, char *msg) {
  if (CACHE == NULL) {
    load_vm(pid);
    CACHE = hash_new(32, hash_str, hash_cmp_str);
  }
  kv cached_addr = hash_getv(CACHE, KV(.str = msg));
  if (cached_addr.int32 != 0) {
    nop_call(pid, cached_addr.int32);
    return 0;
  }
  uint32_t msg_addr = find_str(msg);
  uint32_t lea_addr = find_lea(msg_addr);
  uint32_t call_addr = find_call(lea_addr);
  nop_call(pid, call_addr);
  hash_set(CACHE, KV(.str = msg), KV(.int32 = call_addr));
  return 0;
}