/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigEntry_Function.cpp
 *  @brief Tests for ConfigEntry_Function with various types and edge cases 
 */

#include <functional>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "config/ConfigEntry_Function.hpp"
#include "config/ConfigEntry_Linked.hpp"


TEST_CASE("ConfigEntry_Function_NoParameters", "[config]"){
  {
    // Cannot create a function with return type void -- bug???
    /* // Create a test function with no arguments
    std::function<void()> func1 = []() { int i = 7; return i; };
    emp::vector<emp::Ptr<mabe::ConfigEntry>> args1{nullptr, nullptr};

    // Create a config function and check it's set up 
    mabe::ConfigEntry_Function config_func1("func1 name", "simple int function", nullptr);
    CHECK(config_func1.GetName() == "func1 name"); 
    CHECK(config_func1.GetDesc() == "simple int function"); 
    CHECK(config_func1.GetScope() == nullptr); 

    // Try and SetFunction (no params) with no parameters passed in
    config_func1.SetFunction(func1);  
    CHECK(config_func1.IsFunction());
    CHECK(config_func1.Call(args1));  */
  }
}

int func01(int i) {int z = i; return z; }; 

TEST_CASE("ConfigEntry_Function_OneParameter", "[config]") {
    {
        // Create a function that returns an int
        int param01 = 6; 
        int ret = 0; 
        //std::function<int(int i )> func1 = [](int i) {int z = i; return z; }; 
        
        std::function<int(int i)> func1 = func01; 
        mabe::ConfigEntry_Linked<int> return_type("name", ret, "desc", nullptr); 
        emp::Ptr<mabe::ConfigEntry> return_type_ptr = &return_type;
        mabe::ConfigEntry_Linked<int> param1("param1", param01, "desc", nullptr);
        emp::Ptr<mabe::ConfigEntry> param1_ptr = &param1; 
        emp::vector<emp::Ptr<mabe::ConfigEntry>> args; 
        args.push_back(return_type_ptr); 
        args.push_back(param1_ptr); 

        // Create a ConfigEntry_Function and check setup
        mabe::ConfigEntry_Function config_func1("func1 name", "what it does", nullptr); 
        CHECK(config_func1.GetName() == "func1 name"); 
        CHECK(config_func1.GetDesc() == "what it does"); 
        CHECK(config_func1.GetScope() == nullptr); 

        // Set the function we made
        config_func1.SetFunction(func1);
        CHECK(config_func1.IsFunction()); 
        //CHECK(config_func1.Call(args) == 7); 

    }
}
