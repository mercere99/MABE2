/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for error management with various types and edge cases 
 */

#include <functional>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "core/ErrorManager.hpp"
#include "core/MABE.hpp"
#include "evaluate/static/EvalNK.hpp"

// to test:

// We can create an ErrorManager simply by passing 1) an error callback function and 2) a warning callback function
// If manager is *not* active, we can send errors/warnings without them being immediately displayed.
// If the manager _is_ active, all errors/warnings that are sent to the manager are automatically displayed
// Do SetErrorCallback and SetWarningCallback actually work?
// Flush will print all _held_ errors/warnings (those in the queue that have not yet been displayed
// Activate / deactivate do what they are supposed to
// Getter functions work as expected

TEST_CASE("ErrorManagerBasic", "[core]"){
  {
    bool has_error_been_thrown = false;
    bool has_warning_been_thrown = false;

    // Warning callback function
    std::function<void(const std::string &)> warning_callback = [&has_warning_been_thrown](const std::string & s){
      std::cout << "This is an warning:" << std::endl;
      std::cout << s << std::endl;
      has_warning_been_thrown = true;
    };
    // Error callback function
    std::function<void(const std::string &)> error_callback = [&has_error_been_thrown](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
      has_error_been_thrown = true;
    };

    
    // Create error manager instance
    mabe::ErrorManager manager(error_callback, warning_callback);

    // Add errors and warnings to inactive manager, nothing thrown
    manager.AddError("Error00");
    manager.AddWarning("Warning00");

    REQUIRE(has_error_been_thrown == 0);
    REQUIRE(has_warning_been_thrown == 0);

    // Flush manager to display errors
    REQUIRE(has_error_been_thrown == 1);
    REQUIRE(has_warning_been_thrown == 1);

    // Set manager to active
    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    manager.Activate();

    // Add errors and warnings to active manager, thrown immediately 
    manager.AddError("Error01");
    REQUIRE(has_error_been_thrown == 1);
    manager.AddWarning("Warning01");
    REQUIRE(has_warning_been_thrown == 1);
  }
}

/*
bool has_error_been_thrown = false;
  // Lambda callback that does NOT capture any variables
  std::function<void(const std::string &)> warning_callback = [](const std::string & s){
    std::cout << "This is an warning:" << std::endl;
    std::cout << s << std::endl;
  };
  // Lambda callback that captures the boolean variable by reference
  std::function<void(const std::string &)> error_callback = [&has_error_been_thrown](const std::string & s){
    std::cout << "This is an error:" << std::endl;
    std::cout << s << std::endl;
    has_error_been_thrown = true;
  };
  std::cout << "Has error been thrown: " << has_error_been_thrown << std::endl;
  // Create our error manager instance
  mabe::ErrorManager manager(error_callback, warning_callback);
  std::cout << "Has error been thrown: " << has_error_been_thrown << std::endl;
  // The manager defaults to inactive, and thus adding an error will not immediately triggger it
  manager.AddError("This is an error!!! AHHHH!!!");
  std::cout << "This should print before the error because the manager is inactive" << std::endl;
  std::cout << "Has error been thrown: " << has_error_been_thrown << std::endl;
  // Actually "trigger" (for lack of a better term) the pending error
  manager.Flush();
  std::cout << "Has error been thrown: " << has_error_been_thrown << std::endl;
  // How we might leverage the captured variables for testing: 
  // REQUIRE(has_error_been_thrown == true);
  // We can still use this variable outside the callback!
  has_error_been_thrown = false;
*/