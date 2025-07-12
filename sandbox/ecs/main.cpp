#include "sandbox/ecs/application.h"

#include <CLI/CLI.hpp>

#include <fmt/core.h>

int main(int argc, char* argv[])
{
  CLI::App cli_app{"ECS Demo Application"};
  CLI11_PARSE(cli_app, argc, argv);

  ecs::demo::Application app;

  if (!app.init("ECS Demo Application", 800, 600))
  {
    return EXIT_FAILURE;
  }

  app.run();

  return EXIT_SUCCESS;
}
