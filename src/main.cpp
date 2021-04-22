#include "inputparser.h"
#include "namespaces.h"

int main(int argc, char *argv[]) {
  // parse command line arguments if applicable
  InputParser::getInstance().add_values(argc, argv);
  if (InputParser::getInstance().cmdOptionExists("--help")) {
    std::cout << "help menue";
    return 0;
  }

  run_namespaces();

  return 0;
}