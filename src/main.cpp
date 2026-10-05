#include <CLI11.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdint>
#include <filesystem>
#include <string>

class UULD {};

struct GitInfo {
  std::string gitRepoName;
  std::string gitBranchName;
};

int getGitInfo(std::string& contentInfo, const char* param, const char* flag);

class item {
 public:
  UULD id;
  uint64_t createdTime;
  uint64_t lastUpdateTime;

  std::string cwd;
  std::string gitRepoName;
  std::string gitBranchName;
  std::string content;

  item(std::string content) : content(std::move(content)) {
    cwd = std::filesystem::current_path().string();

    getGitInfo(gitRepoName, "rev-parse", "--show-toplevel");
    getGitInfo(gitBranchName, "branch", "--show-current");
  }
  ~item() {}
};

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

int main(int argc, char* argv[]) {
  CLI::App app{"Temporary information shelf"};

  std::string content;

  auto* add = app.add_subcommand("add", "Add an item");

  add->add_option("content", content, "Content to save")->required();

  CLI11_PARSE(app, argc, argv);

  if (*add) {
    item{content};
  }
}
