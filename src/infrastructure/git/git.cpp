#include "infrastructure/git/git.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

int getGitInfo(std::string& contentInfo, const char* param, const char* flag) {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return 1;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
    execlp("git", "git", param, flag, nullptr);
    perror("execlp");

    return 1;
  }

  close(pipefd[1]);
  char buffer[512] = {0};

  ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1);

  if (bytesRead == -1) {
    perror("read");
    close(pipefd[0]);
    waitpid(pid, nullptr, 0);
    return 1;
  }

  if (bytesRead > 0) {
    buffer[bytesRead] = '\0';
    contentInfo = buffer;

    if (!contentInfo.empty() && contentInfo.back() == '\n') {
      contentInfo.pop_back();
    }
  }

  close(pipefd[0]);
  waitpid(pid, nullptr, 0);

  return 0;
}
