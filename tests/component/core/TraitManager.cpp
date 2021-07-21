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

TEST_CASE("TraitInfo_Locks", "[core]"){
  { 
    //  [SETUP]
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    auto error_func = [](const std::string & s){
      std::cout << "Error: " << s;
    }; 
    auto warning_func = [](const std::string & s){
      std::cout << "Warning: " << s;
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);

    //  [BEGIN TESTS]
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

TEST_CASE("TraitInfo_Basic", "[core]"){
  { 
    //  [SETUP]
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    mabe::TypedTraitInfo<double> trait_k("trait_k"); 
    
    // Add module(s) to access the trait
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);
    mabe::EvalNK nk2_mod(control); 

    // Setup a TraitManager
    // Use bools to tell if an error has been thrown 
    bool has_error_been_thrown = false; 
    bool has_warning_been_thrown = false; 

    auto error_func = [&has_error_been_thrown](const std::string & s){
      std::cout << "Error: " << s;
      has_error_been_thrown = true;
    }; 
    auto warning_func = [&has_warning_been_thrown](const std::string & s){
      std::cout << "Warning: " << s;
      has_warning_been_thrown = true; 
    }; 
    
    mabe::ErrorManager error_man(error_func, warning_func);
    error_man.Activate(); 
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);

    //  [BEGIN TESTS]
    // Check that traitmap begins as empty
    REQUIRE(trait_man.GetSize() == 0); 

    // Check manager is initially locked
    REQUIRE(trait_man.GetLocked()); 

    // Traits cannot be added if manager is locked
    //   TEST IS CORRECT, DISPLAYING INCORRECT BEHAVIOR
    // trait_man.AddTrait(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_k", "a trait", emp::GetTypeID<double>()); 
    // REQUIRE(has_error_been_thrown); 
    // REQUIRE(trait_man.GetLocked()); 
    // REQUIRE(trait_man.GetSize() == 0); 

    // Reset error flag and unlock manager
    has_error_been_thrown = false; 
    trait_man.Unlock(); 

    // Check trait with unknown access shouldn't add to the traitmap
    //   TESTS ARE CORRECT, DISPLAYING WEIRD BEHAVIOR
    //trait_man.AddTrait(&nk_mod, mabe::TraitInfo::Access::UNKNOWN, "trait_i", "a trait", emp::GetTypeID<int>()); 
    //REQUIRE(has_error_been_thrown); 
    //REQUIRE(trait_man.GetSize() == 0); 

    // Reset error flag
    has_error_been_thrown = false; 

    // Add a trait regularly and check to see if traitmap expands
    trait_man.AddTrait(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", emp::GetTypeID<int>()); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 1); 

    // Add same trait to a different module
    // Shouldn't expand traitmap
    trait_man.AddTrait(&nk2_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", emp::GetTypeID<int>()); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 1); 



    /*
    Problems
    1. 
    When only a trait with unknown access is added, trait_man.GetSize() returns 1 instead of 0
    When only a trait with known access is added, trait_man.GetSize() returns 1 as expected

    When both a trait with known and unknown access is added, trait_man.GetSize() returns 1....????

    2. 
    When manager is locked and you try and add a trait, an error is thrown (correct), but the traitmap 
    expands, meaning that the trait is added anyway -> ISSUE
    */
  } 
}

TEST_CASE("TraitInfo_AddTrait", "[core]"){
  { 
    //  [SETUP]
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    mabe::TypedTraitInfo<double> trait_k("trait_k"); 
    
    // Add module(s) to access the trait
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);

    // Setup a TraitManager
    // Use bools to tell if an error has been thrown 
    bool has_error_been_thrown = false; 
    bool has_warning_been_thrown = false; 

    auto error_func = [&has_error_been_thrown](const std::string & s){
      std::cout << "Error: " << s;
      has_error_been_thrown = true;
    }; 
    auto warning_func = [&has_warning_been_thrown](const std::string & s){
      std::cout << "Warning: " << s;
      has_warning_been_thrown = true; 
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    error_man.Activate(); 
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    trait_man.Unlock(); 

    //  [BEGIN TESTS]

    // Initial traitmap is empty
    REQUIRE(trait_man.GetSize() == 0); 

    // Add a trait with invalid type
    trait_man.AddTrait(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", emp::GetTypeID<std::string>()); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 0); 

    // Add same 

  } 
}

TEST_CASE("TraitInfo_AccessSpecifications", "[core]"){
  { 
    /  [SETUP]
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");
    mabe::TypedTraitInfo<double> trait_k("trait_k"); 
    
    // Add module(s) to access the trait
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);

    // Setup a TraitManager
    // Use bools to tell if an error has been thrown 
    bool has_error_been_thrown = false; 
    bool has_warning_been_thrown = false; 

    auto error_func = [&has_error_been_thrown](const std::string & s){
      std::cout << "Error: " << s;
      has_error_been_thrown = true;
    }; 
    auto warning_func = [&has_warning_been_thrown](const std::string & s){
      std::cout << "Warning: " << s;
      has_warning_been_thrown = true; 
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    error_man.Activate(); 
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    trait_man.Unlock(); 

    //  [BEGIN TESTS]
  } 
}

