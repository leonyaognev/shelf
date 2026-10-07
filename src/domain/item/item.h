#pragma once

#include <cstdint>
#include <string>

#include "domain/id/ulid.h"

// -------------------------------------------------------------
// A single shelf entry: its content plus the context it was
// captured in (cwd, git repo, git branch).
// -------------------------------------------------------------
class item {
 public:
  ULID id;
  uint64_t createdTime;
  uint64_t lastUpdateTime;

  std::string cwd;
  std::string gitRepoName;
  std::string gitBranchName;
  std::string content;

  item(std::string content);
  ~item() {}
};
