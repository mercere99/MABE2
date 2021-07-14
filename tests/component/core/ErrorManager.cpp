/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for error management with various types and edge cases 
 */

#include <functional>
#include <string>
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

    // Update error and warning callback functions
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
    has_error_been_thrown = false;
    has_warning_been_thrown = false;

    manager.Activate();
    manager.AddError("Error02");
    manager.AddWarning("Warning02");

    REQUIRE(has_error_been_thrown == 1);
    REQUIRE(new_error_cb == 1);
    REQUIRE(has_warning_been_thrown == 1);
    REQUIRE(new_warning_cb == 1);

    // Deactivate manager
    has_error_been_thrown = false;
    has_warning_been_thrown = false;

    manager.Deactivate();
    manager.AddError("Error03");
    manager.AddWarning("Warning03");

    REQUIRE(has_error_been_thrown == 0);
    REQUIRE(has_warning_been_thrown == 0);

  }
}


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
    mabe::ErrorManager manager01(error_callback_00, warning_callback_00);

    //Getter defaults
    REQUIRE(manager01.GetErrors().empty() == true);
    REQUIRE(manager01.GetWarnings().empty() == true);
    REQUIRE(manager01.GetNumErrors() == 0);
    REQUIRE(manager01.GetNumWarnings() == 0);
    REQUIRE(manager01.IsActive() == false);

    manager01.Activate();
    REQUIRE(manager01.IsActive() == true);

    // Error and Warnings getter functions
    manager01.AddError("Error01");
    manager01.AddWarning("Warning01");

    std::string error01 = manager01.GetErrors().back();
    REQUIRE(error01.compare("Error01") == 0);
    std::string warning01 = manager01.GetWarnings().back();
    REQUIRE(warning01.compare("Warning01") == 0);

    //NumError and NumWarning getter functions
    REQUIRE(manager01.GetNumErrors() == 1);
    REQUIRE(manager01.GetNumWarnings() == 1);

    // Getters with multiple errors and warnings
    manager01.AddError("Error02");
    manager01.AddWarning("Warning02");

    std::string error02 = manager01.GetErrors().back();
    REQUIRE(error02.compare("Error02") == 0);
    std::string warning02 = manager01.GetWarnings().back();
    REQUIRE(warning02.compare("Warning02") == 0);
    REQUIRE(manager01.GetNumErrors() == 2);
    REQUIRE(manager01.GetNumWarnings() == 2);

    // Clear manager
    manager01.Clear();

    REQUIRE(manager01.GetErrors().empty() == true);
    REQUIRE(manager01.GetWarnings().empty() == true);
    REQUIRE(manager01.GetNumErrors() == 0);
    REQUIRE(manager01.GetNumWarnings() == 0);

  }
}
