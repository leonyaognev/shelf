#include "app/cli/cli.h"

#include <CLI11.h>

#include <string>

App::App() : cliApp("Temporary information shelf"), commands() {
  commands.push_back({cliApp.add_subcommand("add", "Add an item"),
                      [this]() { runAdd(content); }});
  commands.back()
      .cli->add_option("content", content, "Content to save")
      ->required();

  commands.push_back({cliApp.add_subcommand("list", "Get all items list"),
                      [this]() { runList(); }});
}

int App::run(int argc, char* argv[]) {
  try {
    cliApp.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return cliApp.exit(e);
  }

  for (auto command : commands) {
    if (*command.cli) command.execute();
  }

  return 0;
}
