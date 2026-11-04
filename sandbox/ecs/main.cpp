#include "sandbox/ecs/application.h"

#include <CLI/CLI.hpp>

#include <fmt/core.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

int main(int argc, char* argv[]) // NOLINT(bugprone-exception-escape)
{
  CLI::App cli_app{"ECS Demo Application"};
  if (argc < 1)
  {
    return EXIT_FAILURE;
  }
  // CLI11's vector overload expects arguments in reverse order, without argv[0].
  // Copy the runtime-provided range rather than indexing argv inside CLI11.
  cli_app.name(*argv);
  std::vector<std::string> arguments{std::next(argv), std::next(argv, argc)};
  std::reverse(arguments.begin(), arguments.end());
  CLI11_PARSE(cli_app, arguments);

  ecs::demo::Application app;

  if (!app.init("ECS Demo Application", 800, 600))
  {
    return EXIT_FAILURE;
  }

  app.run();

  return EXIT_SUCCESS;
}
