#include "domain/item/item.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "infrastructure/git/git.h"

constexpr auto RESET = "\033[0m";
constexpr auto BOLD = "\033[1m";
constexpr auto CYAN = "\033[36m";
constexpr auto GREEN = "\033[32m";
constexpr auto YELLOW = "\033[33m";
constexpr auto MAGENTA = "\033[35m";
constexpr auto BLUE = "\033[34m";
constexpr auto GRAY = "\033[37m";

std::string formatTimestamp(uint64_t ms) {
  using namespace std::chrono;

  constexpr uint64_t minTimestamp = 946684800000ULL;
  auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                 .count();

  if (ms < minTimestamp || ms > now + 31'536'000'000ULL) return "unknown";

  std::time_t time = ms / 1000;
  std::tm* local = std::localtime(&time);

  if (!local) return "unknown";

  std::ostringstream out;
  out << std::put_time(local, "%Y-%m-%d %H:%M");
  return out.str();
}

std::string truncateUtf8(const std::string& text, size_t limit) {
  size_t pos = 0;
  size_t chars = 0;

  while (pos < text.size() && chars < limit) {
    unsigned char ch = text[pos];
    pos += (ch < 0x80) ? 1 : (ch < 0xE0) ? 2 : (ch < 0xF0) ? 3 : 4;
    ++chars;
  }

  if (pos >= text.size()) return text;
  if (!limit) return {};

  pos = 0;
  for (size_t i = 1; i < limit; ++i) {
    unsigned char ch = text[pos];
    pos += (ch < 0x80) ? 1 : (ch < 0xE0) ? 2 : (ch < 0xF0) ? 3 : 4;

    if (pos >= text.size()) return text;
  }

  return text.substr(0, pos) + "…";
}

std::string flattenWhitespace(const std::string& text) {
  std::string result;
  bool space = false;

  for (unsigned char ch : text) {
    if (std::isspace(ch)) {
      space = !result.empty();
    } else {
      if (space) result += ' ';
      result += static_cast<char>(ch);
      space = false;
    }
  }

  return result;
}

void printField(const char* label, const std::string& value) {
  std::cout << "  " << GRAY << std::left << std::setw(14) << label << RESET
            << value << '\n';
}

Item::Item(std::string content) : content(std::move(content)) {
  cwd = std::filesystem::current_path().string();

  createdTime = lastUpdateTime =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  getGitInfo(gitRepoName, "rev-parse", "--show-toplevel");
  getGitInfo(gitBranchName, "branch", "--show-current");
}

Item::Item(ItemData&& data) {
  id = data.id;
  createdTime = data.createdTime;
  lastUpdateTime = data.lastUpdateTime;
  cwd = std::move(data.cwd);
  gitRepoName = std::move(data.gitRepoName);
  gitBranchName = std::move(data.gitBranchName);
  content = std::move(data.content);
}

void Item::shortPrintItem(size_t contentLimit) const {
  std::cout << BOLD << CYAN << id.str << RESET << "  " << GREEN
            << formatTimestamp(createdTime) << RESET;

  if (!gitBranchName.empty())
    std::cout << "  " << MAGENTA << "git:" << gitBranchName << RESET;

  std::cout << '\n';

  if (!cwd.empty()) std::cout << "  " << GRAY << cwd << RESET << '\n';

  std::string preview = truncateUtf8(flattenWhitespace(content), contentLimit);

  std::cout << "  " << YELLOW << "> " << RESET
            << (preview.empty() ? "(empty content)" : preview) << "\n\n";
}

void Item::fullPrintItem() const {
  std::cout << BOLD << CYAN << "◆ " << id.str << RESET << '\n'
            << GRAY << "────────────────────────────────────────" << RESET
            << '\n';

  printField("Created:", formatTimestamp(createdTime));
  printField("Updated:", formatTimestamp(lastUpdateTime));
  printField("Directory:", cwd.empty() ? "unknown" : cwd);

  if (!gitRepoName.empty() || !gitBranchName.empty()) {
    std::string git = gitBranchName.empty() ? "unknown branch" : gitBranchName;

    if (!gitRepoName.empty() && gitRepoName != cwd)
      git += " (" + gitRepoName + ')';

    printField("Git:", git);
  }

  std::cout << '\n' << BOLD << BLUE << "  Content" << RESET << '\n';

  if (content.empty()) {
    std::cout << "  " << GRAY << "(empty content)" << RESET << '\n';
  } else {
    std::istringstream input(content);
    std::string line;

    while (std::getline(input, line))
      std::cout << "  " << GRAY << "│ " << RESET << line << '\n';
  }

  std::cout << '\n';
}
