#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "util.h"
#include "proc.h"

void nop_call(pid_t pid, void *call_addr) {
  uint8_t bytes[WORD_SIZE];
  if (ptrace_read(pid, call_addr, bytes, sizeof(bytes)) < WORD_SIZE) {
    fprintf(stderr, "Failed to read the call.");
    exit(1);
  }
  bytes[0] = 0x90;
  bytes[1] = 0x90;
  bytes[2] = 0x90;
  bytes[3] = 0x90;
  bytes[4] = 0x90;
  if(ptrace_write(pid, call_addr, bytes, sizeof(bytes)) < WORD_SIZE) {
    fprintf(stderr, "Failed to overwrite the call.");
    exit(1);
  }
}

void handle_syscall(pid_t pid) {
  struct user_regs_struct regs;
  ptrace(PTRACE_GETREGS, pid, 0, &regs);
  proc_t *proc = proc_get(pid);
  if (regs.orig_rax == 59) {
    // execve(2)
    if (regs.rax == -ENOSYS && proc == NULL) {
      // entering execve
      void *str = (void *)regs.rdi;
      char filename[1024];
      ptrace_read(pid, str, (uint8_t *)filename, sizeof(filename));
      proc_t *proc = proc_new(pid, filename);
      proc_add(proc);
    } else {
      // exiting execve
      if (strstr(proc->filename, "compile")) {
        nop_call(pid, (void *)0xcd3f19); // delared and not used
        nop_call(pid, (void *)0xccd207); // imported and not used
        nop_call(pid, (void *)0xccd176); // imported as %q and not used
      }
    }
  }
}

void trace_tree(pid_t root) {
  int status;
  for (;;) {
    pid_t pid = waitpid(-1, &status, 0);
    if (WIFEXITED(status)) {
      if (pid == root) {
        break;
      } else {
        continue;
      }
    }
    handle_syscall(pid);
    ptrace(PTRACE_SYSCALL, pid, 0, 0);
  }
}

int main(int argc, char **argv) {
  if (argc < 2 || strcmp(argv[1], "go")) {
    fprintf(stderr, "Usage: %s go build|run xyz.go\n", argv[0]);
    return 1;
  }
  pid_t child = fork();
  if (child == 0) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    return execvp(argv[1], &argv[1]);
  } else {
    waitpid(child, NULL, 0);
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE);
    ptrace(PTRACE_SYSCALL, child, 0, 0);  
    trace_tree(child);
  }
  return 0;
}