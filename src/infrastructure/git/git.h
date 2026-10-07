#pragma once

#include <string>

// -------------------------------------------------------------
// Git context captured for an item
// -------------------------------------------------------------
struct GitInfo {
  std::string gitRepoName;
  std::string gitBranchName;
};

// -------------------------------------------------------------
// Runs `git <param> <flag>` in a child process and stores its
// stdout (trailing newline stripped) in contentInfo.
// Returns 0 on success, 1 on any fork/pipe/read failure.
// -------------------------------------------------------------
int getGitInfo(std::string& contentInfo, const char* param, const char* flag);
