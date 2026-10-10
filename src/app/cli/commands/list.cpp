#include <list>

#include "app/cli/cli.h"
#include "domain/item/item.h"
#include "domain/repository/repository.h"

std::list<Item> App::runList() {
  Repository repo;
  std::list<Item> items = repo.getList();

  for (auto item : items) {
    item.shortPrintItem();
  }

  return items;
}
