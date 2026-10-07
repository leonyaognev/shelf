#include "app/cli/cli.h"

#include <CLI11.h>

#include <iostream>
#include <string>

#include "app/cli/commands/add.h"

int run(int argc, char* argv[]) {
  CLI::App app{"Temporary information shelf"};

  std::string content;

  auto* add = app.add_subcommand("add", "Add an item");

  add->add_option("content", content, "Content to save")->required();

  CLI11_PARSE(app, argc, argv);

  if (*add) {
    runAdd(content);
  }

  return 0;
}
