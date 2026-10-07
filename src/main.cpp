#include <CLI11.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>

class ULID {
  uint8_t data[16];

  void getTime() {
    auto now = std::chrono::system_clock::now();
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();

    for (uint8_t chunk = 0; chunk < 6; ++chunk) {
      uint8_t bits = 40 - chunk * 8;
      data[chunk] = (timestamp >> bits) & 0xFF;
    }
  }

  void getRandom() {
    std::random_device rd;

    std::seed_seq seq{rd(), rd(), rd(), rd()};

    std::mt19937_64 generator(seq);

    uint64_t random64 = generator();

    for (uint8_t chunk = 0; chunk < 8; ++chunk) {
      uint8_t bits = 56 - chunk * 8;
      data[chunk + 6] = (random64 >> bits) & 0xFF;
    }

    uint16_t random16 = generator() & 0xFFFF;

    data[14] = (random16 >> 8) & 0xFF;
    data[15] = random16 & 0xFF;
  }

  std::string base64(uint8_t data[16]) {
    const size_t chunkSize = 5;

    std::string result(26, '0');
    std::string letters = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

    for (uint8_t chunk = 0; chunk < 26; ++chunk) {
      uint8_t value = 0;

      for (uint8_t bit = 0; bit < 5; ++bit) {
        uint8_t currentBit = chunk * chunkSize + bit;

        if (currentBit < 2) continue;

        currentBit -= 2;
        uint8_t byteIndex = currentBit / 8;
        uint8_t bitIndex = 7 - currentBit % 8;

        value <<= 1;
        value |= (data[byteIndex] >> bitIndex) & 1;
      }
      result[chunk] = letters[value];
    }

    return result;
  }

 public:
  std::string id;

  ULID() {
    getTime();
    getRandom();

    id = base64(data);
  }
};

struct GitInfo {
  std::string gitRepoName;
  std::string gitBranchName;
};

int getGitInfo(std::string& contentInfo, const char* param, const char* flag);

class item {
 public:
  ULID id;
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
