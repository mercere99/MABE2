/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 * 
 *  @file  MABE.cpp
 *  @brief Tests for the main MABE object 
 */
#include <functional>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "core/TraitManager.hpp"
#include "core/MABE.hpp"
#include "evaluate/static/EvalNK.hpp"

TEST_CASE("TraitInfo_Basic", "[core]"){
  { 
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    auto error_func = [](const std::string & s){
      std::cout << "Error: " << s;
    }; 
    auto warning_func = [](const std::string & s){
      std::cout << "Warning: " << s;
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::TraitManager<mabe::ModuleBase> train_man(error_man);

    // Check that traitmap begins as empty!
  } 
}

TEST_CASE("TraitInfo_Locks", "[core]"){
  { 
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    auto error_func = [](const std::string & s){
      std::cout << "Error: " << s;
    }; 
    auto warning_func = [](const std::string & s){
      std::cout << "Warning: " << s;
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);

    // Make sure GetLocked returns a boolean
    REQUIRE(trait_man.GetLocked() <= 1); 
    REQUIRE(trait_man.GetLocked() >= 0);
    
    // Manager should begin as locked 
    REQUIRE(trait_man.GetLocked()); 

    // Check that Unlock works
    trait_man.Unlock(); 
    REQUIRE_FALSE(trait_man.GetLocked()); 

    // Check that Lock works
    trait_man.Lock(); 
    REQUIRE(trait_man.GetLocked()); 
  } 
}

TEST_CASE("TraitInfo_AddTrait", "[core]"){
  { 
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    auto error_func = [](const std::string & s){
      std::cout << "Error: " << s;
    }; 
    auto warning_func = [](const std::string & s){
      std::cout << "Warning: " << s;
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::TraitManager<mabe::ModuleBase> train_man(error_man);
  } 
}

TEST_CASE("TraitInfo_AccessSpecifications", "[core]"){
  { 
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    auto error_func = [](const std::string & s){
      std::cout << "Error: " << s;
    }; 
    auto warning_func = [](const std::string & s){
      std::cout << "Warning: " << s;
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::TraitManager<mabe::ModuleBase> train_man(error_man);
  } 
}

