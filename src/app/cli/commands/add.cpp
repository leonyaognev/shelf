#include <iostream>

#include "app/cli/cli.h"
#include "domain/item/item.h"
#include "domain/repository/repository.h"

void App::runAdd(const std::string& content) {
  Item item(content);
  Repository repo;

  item.fullPrintItem();

  repo.saveItem(item);
}
