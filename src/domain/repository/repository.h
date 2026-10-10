#include <filesystem>
#include <list>

#include "domain/item/item.h"

class Repository {
  std::filesystem::path path_;

 public:
  int saveItem(const Item& item);
  Item getItem(const ULID& id);
  std::list<Item> getList();
  int delItem(const ULID& id);

  Repository();
  Repository(std::filesystem::path& path);
};
