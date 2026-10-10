#pragma once

// -------------------------------------------------------------
// Command line interface: parses argv and dispatches
// subcommands. Returns the process exit code.
// -------------------------------------------------------------
#include <functional>
#include <list>
#include <string>
#include <vector>

#include "CLI11.h"
#include "domain/item/item.h"

class App {
  struct Command {
    CLI::App* cli;
    std::function<void()> execute;
  };

  std::string content;
  CLI::App cliApp;
  std::vector<Command> commands;

  void runAdd(const std::string& content);
  std::list<Item> runList();

 public:
  App();

  int run(int argc, char* argv[]);
};
