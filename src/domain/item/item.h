#pragma once

#include <cstdint>
#include <string>

#include "domain/id/ulid.h"

struct ItemData {
  ULID id;
  uint64_t createdTime;
  uint64_t lastUpdateTime;

  std::string cwd;
  std::string gitRepoName;
  std::string gitBranchName;
  std::string content;
};

// -------------------------------------------------------------
// A single shelf entry: its content plus the context it was
// captured in (cwd, git repo, git branch).
// -------------------------------------------------------------
class Item {
 public:
  ULID id;
  uint64_t createdTime;
  uint64_t lastUpdateTime;

  std::string cwd;
  std::string gitRepoName;
  std::string gitBranchName;
  std::string content;

  Item(std::string content);
  Item(ItemData&& content);
  ~Item() {}

  void shortPrintItem(std::size_t contentLimit = 100) const;
  void fullPrintItem() const;
};
