#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "util.h"
#include "proc.h"
#include "patch.h"

void handle_syscall(pid_t pid) {
  struct user_regs_struct regs;
  ptrace(PTRACE_GETREGS, pid, 0, &regs);
  if (regs.orig_rax == 59) {
    // execve(2)
    proc_t *proc = proc_get(pid);
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
        nop_softerrorf(pid, "declared and not used: \%s");
        nop_softerrorf(pid, "\%q imported and not used");
        nop_softerrorf(pid, "\%q imported as \%s and not used");
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
      }
    } else {
      handle_syscall(pid);
      ptrace(PTRACE_SYSCALL, pid, 0, 0); // 0xCC
    }
  }
}

int main(int argc, char **argv) {
  struct utsname u;
  if (uname(&u) || strcmp(u.sysname, "Linux") || strcmp(u.machine, "x86_64")) {
    FATAL("only x86_64 Linux is supported");
  }
  if (argc < 2 || (strcmp(argv[1], "go") && strcmp(argv[1], "./go"))) {
    fprintf(stderr, "usage: %s go build|run xyz.go\n", argv[0]);
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