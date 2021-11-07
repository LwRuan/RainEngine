#include <iostream>

#include "app.h"
#include "spdlog/spdlog.h"

using namespace Rain;

int main() {
  spdlog::set_pattern("[%^%l%$] %v");
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::info);
#else
  spdlog::set_level(spdlog::level::debug);
#endif
  App app;
  app.Init();
  app.MainLoop();
  app.CleanUp();
}