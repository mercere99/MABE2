/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for ConfigEntry with various types and edge cases 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "config/ConfigEntry.hpp"


TEST_CASE("ConfigEntry_Linker", "[config]"){
  {
    int v = 2;
    mabe::ConfigEntry_Linked<int> linked_entry("name", v, "variable!", nullptr);
    // if we set v to 7 then in Linked should change to 7 
    // linked always in sync
    // configentry_var not linked REALLY make sure this works 

  }
}

TEST_CASE("ConfigLEntry_Linker<std::string>", "[config]"){
  {
    int v = 2;
    mabe::ConfigEntry_Linked<int> linked_string_entry("name", v, "variable!", nullptr);
    // if we set v to 7 then in Linked should change to 7 
    // linked always in sync
    // configentry_var not linked REALLY make sure this works 
    
  }
}

TEST_CASE("ConfigEntry_Functions", "[config]"){
  {
    int v = 2;
    mabe::ConfigEntry_Linked<int> functions_entry("name", v, "variable!", nullptr);
    // if we set v to 7 then in Linked should change to 7 
    // linked always in sync
    // configentry_var not linked REALLY make sure this works 
    
  }
}

TEST_CASE("ConfigEntry_Var", "[config]"){
  {
    int v = 2;
    mabe::ConfigEntry_Linked<int> var_entry("name", v, "variable!", nullptr);
    // if we set v to 7 then in Linked should change to 7 
    // linked always in sync
    // configentry_var not linked REALLY make sure this works 
    
  }
}

TEST_CASE("ConfigEntry_Var<std::string>", "[config]"){
  {
    int v = 2;
    mabe::ConfigEntry_Linked<int> var_string_entry("name", v, "variable!", nullptr);
    // if we set v to 7 then in Linked should change to 7 
    // linked always in sync
    // configentry_var not linked REALLY make sure this works 
    
  }
}