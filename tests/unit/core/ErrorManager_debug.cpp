/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ErrorManager_debug.cpp
 *  @brief Tests for error management in debug mode, which should immediately thrown an 
 *    emp_error (and thus abort) when AddError() is called
 */

// Testing debug
#define TDEBUG 1

// Ensure we're in debug mode (to test for emp_error calls) 
#undef NDEBUG

#include <functional>
#include <string>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "core/ErrorManager.hpp"



TEST_CASE("ErrorManager_Active_debug", "[core]"){
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
    CHECK(manager.IsActive());

    // Check defaults
    CHECK_FALSE(has_error_been_thrown);
    CHECK_FALSE(has_warning_been_thrown);
    CHECK(manager.GetNumErrors() == 0);
    CHECK(manager.GetErrors().empty());
    CHECK(manager.GetNumWarnings() == 0);
    CHECK(manager.GetWarnings().empty());

    // AddWarning() should *not* trigger an emp_error()
    CHECK_FALSE(emp::error_thrown);
    manager.AddWarning("Non-fatal warning");
    CHECK_FALSE(emp::error_thrown);
    
    // AddError() should trigger an emp_error()
    CHECK_FALSE(emp::error_thrown);
    manager.AddError("Fatal error");
    CHECK(emp::error_thrown);
    CHECK(emp::error_info.output == "Fatal error");
    emp::error_clear(); 
  }
}

TEST_CASE("ErrorManager_Inactive_debug", "[core]"){
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

    // Create error manager instance and leave it inactive
    mabe::ErrorManager manager(error_callback_00, warning_callback_00);
    CHECK_FALSE(manager.IsActive());

    // Check defaults
    CHECK_FALSE(has_error_been_thrown);
    CHECK_FALSE(has_warning_been_thrown);
    CHECK(manager.GetNumErrors() == 0);
    CHECK(manager.GetErrors().empty());
    CHECK(manager.GetNumWarnings() == 0);
    CHECK(manager.GetWarnings().empty());
    
    // AddWarning() should *not* trigger an emp_error()
    CHECK_FALSE(emp::error_thrown);
    manager.AddWarning("Non-fatal warning");
    CHECK_FALSE(emp::error_thrown);

    // AddError() should trigger an emp_error()
    CHECK_FALSE(emp::error_thrown);
    manager.AddError("Fatal error");
    CHECK(emp::error_thrown);
    CHECK(emp::error_info.output == "Fatal error");
    
  }
}

