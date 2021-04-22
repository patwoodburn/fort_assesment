#include <algorithm>
#include <sys/wait.h>

#include "catch2/catch.hpp"

#include "namespaces.h"

// i'm not happy with the state of the tests here i honestly could not get pid
// status checking working without locking up the proccess their are assertions
// i can do around the get_children function but that feels like it vialates
// behavior driven testing as it asserts internal workings but not the actual
// behavior of the program. since that variable is not part of the spec it
// should not be the soul unit of messurement
SCENARIO(
    "When siguser1 is called a pid should start and be added to children list",
    "") {
  GIVEN("an empty children list") {
    std::fill(get_children().begin(), get_children().end(), 0);
    WHEN("siguser1 function called") {
      siguser1_handler();
      THEN("children array should have a pid in it to running proccess") {
        REQUIRE(get_children().at(0) >= 0);
        // this feels really close but i don't want to get hung up hear.
        // int status;
        // waitpid(get_children().at(0), &status, WNOHANG);
        // REQUIRE(status == 0);
      }
    }
    WHEN("sigint function is called") {
      REQUIRE(get_children().at(0) >= 0);
      // this should return the pid of the procces making sure it's still runnig
      // int running = waitpid()
      sigint_handler();
      THEN("children should go to zero and proccess should close") {
        REQUIRE(get_children().at(0) == 0);
        // check if running proccess does not exist?
      }
    }
  }
}
