/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ErrorManager.cpp
 *  @brief Tests for error management with various types and edge cases 
 */

// Testing debug
#define TDEBUG 1

// Pretend we're not in debug mode (for _most_ tests)
#define NDEBUG 1

#include <functional>
#include <string>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "core/ErrorManager.hpp"


TEST_CASE("ErrorManager_Getters", "[core]"){
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
    REQUIRE(manager01.GetErrors().empty());
    REQUIRE(manager01.GetWarnings().empty());
    REQUIRE(manager01.GetNumErrors() == 0);
    REQUIRE(manager01.GetNumWarnings() == 0);
    REQUIRE_FALSE(manager01.IsActive());

    // Activation functions
    manager01.Activate();
    REQUIRE(manager01.IsActive());
    manager01.Deactivate();
    REQUIRE_FALSE(manager01.IsActive());
    manager01.Activate();
    REQUIRE(manager01.IsActive());


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

    REQUIRE(manager01.GetErrors().empty());
    REQUIRE(manager01.GetNumErrors() == 0);
    REQUIRE(manager01.GetWarnings().empty());
    REQUIRE(manager01.GetNumWarnings() == 0);
  }
}

TEST_CASE("ErrorManager_Active", "[core]"){
  {
    bool has_error_been_thrown = false;
    bool has_warning_been_thrown = false;

    // Error callback function
    std::function<void(const std::string &)> error_callback_00 = 
        [&has_error_been_thrown](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
      has_error_been_thrown = true;
    };
    // Warning callback function
    std::function<void(const std::string &)> warning_callback_00 = 
        [&has_warning_been_thrown](const std::string & s){
      std::cout << "This is a warning:" << std::endl;
      std::cout << s << std::endl;
      has_warning_been_thrown = true;
    };

    // Create error manager instance and activate it
    mabe::ErrorManager manager(error_callback_00, warning_callback_00);
    manager.Activate();

    // Check defaults
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 0);
    REQUIRE(manager.GetErrors().empty());
    REQUIRE(manager.GetNumWarnings() == 0);
    REQUIRE(manager.GetWarnings().empty());

    // Add errors and warnings to active manager, thrown immediately 
    manager.AddError("Error01");
    REQUIRE(has_error_been_thrown);
    manager.AddWarning("Warning01");
    REQUIRE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 1);
    REQUIRE(manager.GetErrors().size() == 1);
    REQUIRE(manager.GetErrors()[0] == "Error01");
    REQUIRE(manager.GetNumWarnings() == 1);
    REQUIRE(manager.GetWarnings().size() == 1);
    REQUIRE(manager.GetWarnings()[0] == "Warning01");
    
    has_error_been_thrown = false; 
    has_warning_been_thrown = false;
    
    // Ensure we can store more than one error 
    manager.AddError("Error02");
    REQUIRE(has_error_been_thrown);
    manager.AddWarning("Warning02");
    REQUIRE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 2);
    REQUIRE(manager.GetErrors().size() == 2);
    REQUIRE(manager.GetErrors()[1] == "Error02");
    REQUIRE(manager.GetNumWarnings() == 2);
    REQUIRE(manager.GetWarnings().size() == 2);
    REQUIRE(manager.GetWarnings()[1] == "Warning02");

    // Clear will clear all errors and warnings 
    manager.Clear();
    REQUIRE(manager.GetNumErrors() == 0);
    REQUIRE(manager.GetErrors().empty());
    REQUIRE(manager.GetNumWarnings() == 0);
    REQUIRE(manager.GetWarnings().empty());

    // Update error and warning callback functions
    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    size_t error_callback_count = 0;
    size_t warning_callback_count= 0;
    std::function<void(const std::string &)> error_callback_01 = 
        [&has_error_been_thrown, &error_callback_count](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
      has_error_been_thrown = true;
      error_callback_count++;
    };
    std::function<void(const std::string &)> warning_callback_01 = 
        [&has_warning_been_thrown, &warning_callback_count](const std::string & s){
      std::cout << "This is a warning:" << std::endl;
      std::cout << s << std::endl;
      has_warning_been_thrown = true;
      warning_callback_count++;
    };
    manager.SetErrorCallback(error_callback_01);
    manager.SetWarningCallback(warning_callback_01);

    // Add errors and warnings to active manager, throw immediately
    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(error_callback_count == 0);
    REQUIRE(warning_callback_count == 0);

    manager.AddError("Error03");
    manager.AddWarning("Warning03");
    REQUIRE(has_error_been_thrown == 1);
    REQUIRE(has_warning_been_thrown == 1);
    REQUIRE(error_callback_count == 1);
    REQUIRE(warning_callback_count == 1);
    REQUIRE(manager.GetNumErrors() == 1);
    REQUIRE(manager.GetNumWarnings() == 1);

    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    error_callback_count = 0;
    warning_callback_count = 0;
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(error_callback_count == 0);
    REQUIRE(warning_callback_count == 0);

    // Deactivate manager -- now adding errors/warnings should _not_ immediately trigger
    manager.Deactivate();
    manager.AddError("Error03");
    manager.AddWarning("Warning03");

    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(error_callback_count == 0);
    REQUIRE(warning_callback_count == 0);
    REQUIRE(manager.GetNumErrors() == 2);
    REQUIRE(manager.GetNumWarnings() == 2);

    // Flush should call the callback for errors/warnings that *have not been already seen*
    manager.Flush();
    REQUIRE(has_error_been_thrown);
    REQUIRE(has_warning_been_thrown);
    REQUIRE(error_callback_count == 1);
    REQUIRE(warning_callback_count == 1);
    REQUIRE(manager.GetNumErrors() == 2);
    REQUIRE(manager.GetNumWarnings() == 2);

    // If we're in debug mode, AddError() should instantly call emp_error
#undef NDEBUG

    REQUIRE_FALSE(emp::error_thrown);
    manager.AddError("Fatal error");
    emp_error("Fatal error");
    REQUIRE(emp::error_thrown);
    REQUIRE(emp::error_info.output == "Fatal error");
    
  }
}

TEST_CASE("ErrorManager_Inactive", "[core]"){
  {
    bool has_error_been_thrown = false;
    bool has_warning_been_thrown = false;

    // Error callback function
    std::function<void(const std::string &)> error_callback_00 = 
        [&has_error_been_thrown](const std::string & s){
      std::cout << "This is an error:" << std::endl;
      std::cout << s << std::endl;
      has_error_been_thrown = true;
    };
    // Warning callback function
    std::function<void(const std::string &)> warning_callback_00 = 
        [&has_warning_been_thrown](const std::string & s){
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
    REQUIRE(manager.GetNumErrors() == 1);
    REQUIRE(manager.GetNumWarnings() == 1);
    REQUIRE(manager.GetErrors().size() == 1);
    REQUIRE(manager.GetWarnings().size() == 1);

    // Flush manager to display queued errors and warnings
    manager.Flush();
    REQUIRE(has_error_been_thrown == 1);
    REQUIRE(has_warning_been_thrown == 1);
    REQUIRE(manager.GetNumErrors() == 1);
    REQUIRE(manager.GetNumWarnings() == 1);
    REQUIRE(manager.GetErrors().size() == 1);
    REQUIRE(manager.GetWarnings().size() == 1);

    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);

    // Add errors and warnings to inactive manager again, still nothing thrown
    manager.AddError("Error01");
    manager.AddWarning("Warning01");
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 2);
    REQUIRE(manager.GetNumWarnings() == 2);

    // Calling Activate() should flush all stored errors and warnings
    manager.Activate();
    REQUIRE(has_error_been_thrown);
    REQUIRE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 2);
    REQUIRE(manager.GetNumWarnings() == 2);

    has_error_been_thrown = false;
    has_warning_been_thrown = false;
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    
    // Clear() should clear all errors and warnings
    manager.Deactivate();
    manager.AddError("Error01");
    manager.AddWarning("Warning01");
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 3);
    REQUIRE(manager.GetNumWarnings() == 3);
    manager.Clear();
    REQUIRE_FALSE(has_error_been_thrown);
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(manager.GetNumErrors() == 0);
    REQUIRE(manager.GetNumWarnings() == 0);
  }
}
