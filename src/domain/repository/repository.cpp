#include <domain/repository/repository.h>
#include <unistd.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "domain/item/item.h"

class BinaryWriter {
 public:
  explicit BinaryWriter(std::ofstream& file) : file_(file) {}

  template <typename T>
  void write(T value) {
    file_.write(reinterpret_cast<const char*>(&value), sizeof(T));
  }

  void write(const std::string& value) {
    uint64_t size = value.size();

    write(size);
    file_.write(value.data(), size);
  }

 private:
  std::ofstream& file_;
};

class BinaryReader {
 public:
  explicit BinaryReader(std::ifstream& file) : file_(file) {}

  template <typename T>
  bool read(T& value) {
    return static_cast<bool>(
        file_.read(reinterpret_cast<char*>(&value), sizeof(T)));
  }

  bool read(std::string& value) {
    uint64_t size = 0;

    if (!read(size)) return false;

    const auto pos = file_.tellg();
    if (pos < 0) return false;

    file_.seekg(0, std::ios::end);
    const auto end = file_.tellg();

    if (end < pos) return false;

    const auto remaining = static_cast<uint64_t>(end - pos);

    file_.seekg(pos);

    if (!file_) return false;
    if (size > remaining) return false;
    if (size > value.max_size()) return false;

    value.resize(static_cast<std::string::size_type>(size));

    if (size > 0 &&
        !file_.read(value.data(), static_cast<std::streamsize>(size))) {
      return false;
    }

    return true;
  }

 private:
  std::ifstream& file_;
};

int Repository::saveItem(const Item& item) {
  std::ofstream file(path_ / (item.id.str + ".shelf"), std::ios::binary);
  if (!file) return -1;

  BinaryWriter writer(file);

  file.write(reinterpret_cast<const char*>(item.id.data), 16);
  writer.write(item.createdTime);
  writer.write(item.lastUpdateTime);

  writer.write(item.cwd);
  writer.write(item.gitRepoName);
  writer.write(item.gitBranchName);
  writer.write(item.content);

  return 0;
}

Item Repository::getItem(const ULID& id) {
  (void)id;
  return Item("");
}

std::list<Item> Repository::getList() {
  std::list<Item> items;

  for (const auto& entry : std::filesystem::directory_iterator(path_)) {
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != ".shelf") continue;

    std::ifstream file(entry.path(), std::ios::binary);
    if (!file) continue;

    BinaryReader reader(file);
    ItemData data;

    std::vector<uint8_t> idData(16);

    if (!file.read(reinterpret_cast<char*>(idData.data()),
                   static_cast<std::streamsize>(idData.size()))) {
      continue;
    }

    data.id = ULID(std::move(idData));

    if (!reader.read(data.createdTime)) continue;
    if (!reader.read(data.lastUpdateTime)) continue;

    if (!reader.read(data.cwd)) continue;
    if (!reader.read(data.gitRepoName)) continue;
    if (!reader.read(data.gitBranchName)) continue;
    if (!reader.read(data.content)) continue;

    Item item(std::move(data));
    items.push_back(std::move(item));
  }

  return items;
}
int Repository::delItem(const ULID& id) {
  (void)id;
  return 0;
}

Repository::Repository()
    : path_(std::filesystem::path(std::getenv("HOME")) / ".local" / "share" /
            "shelf") {
  std::filesystem::create_directories(path_);
}

Repository::Repository(std::filesystem::path& path) : path_(path) {
  std::filesystem::create_directories(path_);
}
