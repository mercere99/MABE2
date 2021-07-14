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

// * We can create an ErrorManager simply by passing 1) an error callback function and 2) a warning callback function
// * If manager is *not* active, we can send errors/warnings without them being immediately displayed.
// * If the manager _is_ active, all errors/warnings that are sent to the manager are automatically displayed
// * Do SetErrorCallback and SetWarningCallback actually work?
// * Flush will print all _held_ errors/warnings (those in the queue that have not yet been displayed
// * Activate / deactivate do what they are supposed to
// Getter functions work as expected

TEST_CASE("ErrorManagerBasic", "[core]"){
  {
    bool has_error_been_thrown = false;
    bool has_warning_been_thrown = false;

    // Error callback function
    std::function<void(const std::string &)> error_callback_00 = [&has_error_been_thrown](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
      has_error_been_thrown = true;
    };
    // Warning callback function
    std::function<void(const std::string &)> warning_callback_00 = [&has_warning_been_thrown](const std::string & s){
      std::cout << "This is a warning:" << std::endl;
      std::cout << s << std::endl;
      has_warning_been_thrown = true;
    };

    // Create error manager instance
    mabe::ErrorManager manager(error_callback_00, warning_callback_00);

    // Add errors and warnings to inactive manager, nothing thrown
    manager.AddError("Error00");
    manager.AddWarning("Warning00");
    
    REQUIRE(has_error_been_thrown == 0);
    REQUIRE(has_warning_been_thrown == 0);
    /*
    // Flush manager to display queued errors and warnings
    manager.Flush();
    REQUIRE(has_error_been_thrown == 1);
    REQUIRE(has_warning_been_thrown == 1);

    // Add errors and warnings to active manager, thrown immediately 
    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    manager.Activate();

    manager.AddError("Error01");
    REQUIRE(has_error_been_thrown == 1);
    manager.AddWarning("Warning01");
    REQUIRE(has_warning_been_thrown == 1);

    // Update error and warnign callback functions
    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    bool new_error_cb = false;
    bool new_warning_cb = false;

    std::function<void(const std::string &)> error_callback_01 = [&has_error_been_thrown, &new_error_cb](const std::string & s){
      std::cout << "This is an warning:" << std::endl;
      std::cout << s << std::endl;
      has_error_been_thrown = true;
      new_error_cb = true;
    };
    std::function<void(const std::string &)> warning_callback_01 = [&has_warning_been_thrown, &new_warning_cb](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
      has_warning_been_thrown = true;
      new_warning_cb = true;
    };

    manager.SetErrorCallback(error_callback_01);
    manager.SetWarningCallback(warning_callback_01);

    // Add errors and warnings to active manager, throw immediately
    manager.Activate();
    manager.AddError("Error02");
    manager.AddWarning("Warning02");

    REQUIRE(has_error_been_thrown == 1);
    REQUIRE(new_error_cb == 1);
    REQUIRE(has_warning_been_thrown == 0);
    REQUIRE(new_warning_cb == 1);
    */
  }
}
/*
TEST_CASE("ErrorManagerGetters", "[core]"){
  {
     // Error callback function
    std::function<void(const std::string &)> error_callback_00 = [](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
    };
    // Warning callback function
    std::function<void(const std::string &)> warning_callback_00 = [](const std::string & s){
      std::cout << "This is a warning:" << std::endl;
      std::cout << s << std::endl;
    };

    // Create error manager instance
    mabe::ErrorManager manager(error_callback_00, warning_callback_00);

    //Getter defaults
    REQUIRE(GetErrors() == "");
    REQUIRE(GetWarnings() == "");
    REQUIRE(NumErrors() == 0);
    REQUIRE(NumWarnings() == 0);
    REQUIRE(IsActive() == false);

    manager.Activate();
    REQUIRE(IsActive() == true);
    // Error and Warnings getter functions
    manager.AddError("Error00");
    manager.AddWarnings("Warning00");

    REQUIRE(GetErrors() == "Error00");
    REQUIRE(GetWarnings() == "Warning00");

    //NumError and NumWarning getter functions
    REQUIRE(NumErrors() == 1);
    REQUIRE(NumWarnings() == 1);

    manager.AddError("Error01");
    manager.AddWarning("Warning01");

    REQUIRE(GetErrors() == "Error01 Error00");
    REQUIRE(GetWarnings() == "Warning01 Warning00");
    REQUIRE(NumErrors() == 2);
    REQUIRE(NumWarnings() == 2);
}
*/