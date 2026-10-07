#include "app/cli/commands/add.h"

#include <iostream>

#include "domain/item/item.h"

void runAdd(const std::string& content) {
  item item(content);
  std::cout << item.id.id << "\n";
}
