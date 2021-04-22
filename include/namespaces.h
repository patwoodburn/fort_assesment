#pragma once

#include "inputparser.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

// thread safe singleton of spawned proccesses
std::array<pid_t, 10> &get_children() {
  // std::stack would be a better container here if not for needing to peek
  // elements for printing. 
  static std::array<pid_t, 10> children;
  return children;
}

// forever running children function
int child_fn(__attribute__((unused)) void *args) {
  while (true) {
    sleep(1);
  }
  return 0;
}


// signal handlers
/* implamentation note:
*  it may be better for these functions to take a refrence to a std::array
*  this would make them more more geneal pures and allow them to maintain multiple trees
*/
void siguser1_handler() {
    auto result = std::find(get_children().begin(), get_children().end(), 0);
    if (result != get_children().end()) {
      char child_stack[255];
      uint clone_flags = SIGCHLD;
      get_children().at(result - get_children().begin()) = clone(
          child_fn, child_stack + sizeof(child_stack), clone_flags, nullptr);
    }
}

void sigusr2_handler() {
    std::ofstream out_file;
    out_file.open("pidlist.txt");
    out_file << "Existing Children:\n---\n";
    for (auto pid : get_children()) {
      if (pid != 0) {
        out_file << "child pid: " << (long)pid << "\n";
      }
    }
    out_file.close();
}

void sigint_handler() {
    auto result = std::find(get_children().begin(), get_children().end(), 0);
    if (result == get_children().end()) {
      kill(*(get_children().end() - 1), SIGTERM);
      get_children().at(get_children().end() - get_children().begin() - 1) = 0;
    } else if (result != get_children().begin()) {
      kill(*(result - 1), SIGTERM);
      get_children().at(result - get_children().begin() - 1) = 0;
    }
}


// call indevidual handlers to handle approprate signals
/* implamentation note:
*  this is probably overkill since signal can just call each function
*  but given that it may be better to pass handlers array refrence this
*  provides a place for that code to happen.
*/
void signal_handler(int signum) {
  switch(signum) {
      case SIGUSR1:
        siguser1_handler();
        break;
      case SIGUSR2:
        sigusr2_handler();
        break;
      case SIGINT:
        sigint_handler();
        break;
  }
}

// primarry loop function this will be the init pid for the sake of the name space.
int root_fn(__attribute__((unused)) void *args) {
  // register signals
  (void)signal(SIGUSR1, signal_handler);
  (void)signal(SIGUSR2, signal_handler);
  (void)signal(SIGINT, signal_handler);
  // when in interactive mode prove that were the top pid
  if (InputParser::getInstance().cmdOptionExists("-i")) {
      std::cout << "init pid: " << (long)getpid() << std::endl;
  }
  while (true) {
    sleep(1);
  }
  return 0;
}

void run_namespaces() {
  char stack[8192];
  unsigned int name_spaces = CLONE_NEWPID | SIGCHLD;
  pid_t root = clone(root_fn, stack + sizeof(stack), name_spaces, nullptr);
  std::cout << "host pid: " << (long)root << std::endl;

  // put entry point into interactive mode really just for printing right now
  // in the future maybe register signal so that killing this proccess kills root tree?
  if (InputParser::getInstance().cmdOptionExists("-i")) {
    waitpid(root, nullptr, 0);
  }
}
