#include "domain/item/item.h"

#include <filesystem>

#include "infrastructure/git/git.h"

item::item(std::string content) : content(std::move(content)) {
  cwd = std::filesystem::current_path().string();

  getGitInfo(gitRepoName, "rev-parse", "--show-toplevel");
  getGitInfo(gitBranchName, "branch", "--show-current");
}
