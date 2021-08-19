/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 * 
 *  @file  MABE.cpp
 *  @brief Tests for the main MABE object 
 */

#define TDEBUG 1

#include <functional>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "core/TraitManager.hpp"
#include "core/MABE.hpp"
#include "evaluate/static/EvalNK.hpp"

// NOTE: Errors/Warnings will print to the terminal as "Error: " and "Warning: " without the
//          actual error/warning message. Add the message by writing 
//              std::cout << "Error: "/"Warning: " << s; 
//          instead of 
//              std::cout << "Error: "/"Warning: ";
// NOTE: Even if an error is thrown, the traitmap still updates with the new trait


TEST_CASE("TraitManager_Locks", "[core]"){
  { 
    //  [SETUP]
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    auto error_func = [](const std::string & s){
      std::cout << "Error: ";
    }; 
    auto warning_func = [](const std::string & s){
      std::cout << "Warning: ";
    }; 
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);

    //  [BEGIN TESTS]
    // Make sure GetLocked returns true by default 
    REQUIRE(trait_man.GetLocked()); 
    
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

TEST_CASE("TraitManager_Basic", "[core]"){
  { 
    //  [SETUP]
    // Finish setup for modules
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);
    mabe::EvalNK nk2_mod(control);
    

    bool has_error_been_thrown = false;
    bool has_warning_been_thrown = false; 
    std::string error_message = ""; 

    // Setup an ErrorManager
    auto error_func = [&has_error_been_thrown, &error_message](const std::string & s){
      std::cout << "Error: ";
      has_error_been_thrown = true;
      error_message = s;  
    }; 
    auto warning_func = [&has_warning_been_thrown](const std::string & s){
      std::cout << "Warning: ";
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
    // Verified due to error being thrown 
    // Make sure error message is correct one
    trait_man.AddTrait<double>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", 7.0); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);  
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE(error_message == "Module 'EvalNK' adding trait 'trait_i' before config files have loaded; should be done in SetupModule().");
    
    // Reset error flag and error message and unlock manager
    has_error_been_thrown = false; 
    error_message = "";  
    trait_man.Unlock(); 

    // Check trait with unknown access should throw error
    // Check error message is correct 
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::UNKNOWN, "trait_k", "a trait", 7); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 2); 
    REQUIRE(error_message == "Module EvalNK trying to add trait named 'trait_k' with UNKNOWN access type.");

    // Reset error flag
    has_error_been_thrown = false; 

    // Add a trait regularly and check to see if traitmap expands
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_l", "a trait", 7); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 3); 

    // Add same trait to a different module
    // Shouldn't expand traitmap
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_l", "a trait", 7); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 3); 
  } 
} 

TEST_CASE("TraitManager_AddTrait", "[core]"){

  //  [SETUP]
  // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
  // Add module(s) to access the trait
  mabe::MABE control(0, NULL);
  control.AddPopulation("test_pop");
  mabe::EvalNK nk_mod(control);
  mabe::EvalNK nk2_mod(control); 

  // Setup a TraitManager
  // Use bools to tell if an error has been thrown 
  bool has_error_been_thrown = false; 
  //bool has_error_been_thrown2 = false;
  bool has_warning_been_thrown = false; 
  std::string error_message = ""; 

  auto error_func = [&has_error_been_thrown, &error_message](const std::string & s){
    std::cout << "Error: ";
    has_error_been_thrown = true;
    error_message = s; 
  };
  auto warning_func = [&has_warning_been_thrown](const std::string & s){
    std::cout << "Warning: ";
    has_warning_been_thrown = true; 
  }; 

  { 
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    error_man.Activate(); 
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man); 
    trait_man.Unlock();  

    //  [BEGIN TESTS]
    // Cannot add the same trait twice

    // Initial traitmap is empty
    REQUIRE(trait_man.GetSize() == 0); 

    // Add a trait normally
    mabe::TypedTraitInfo<std::string>& trait_i = (mabe::TypedTraitInfo<std::string>& )trait_man.AddTrait<std::string>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", "test string"); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    // Check default value added 
    REQUIRE(trait_i.GetDefault() == "test string"); 
  

    // Add same trait to same module
    // Should throw error and not add it again to the map
    // Check correct error message is printed
    trait_man.AddTrait<std::string>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", "test string"); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(trait_man.GetSize() == 1);
    REQUIRE(error_message == "Module EvalNK is creating multiple traits named 'trait_i'.");
  } 

  {
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    error_man.Activate(); 
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    trait_man.Unlock();

    // Make sure re-used variables are reset 
    has_error_been_thrown = false; 
    error_message = ""; 

    //  [BEGIN TESTS]
    // Check new modules with accepted AltTypes can be added 

    // Create a trait with ints, doubles and strings allowed
    trait_man.AddTrait<int, double, std::string>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Add a module that accesses with one of the AltTypes
    trait_man.AddTrait<double>(&nk2_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", 7.0); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Also check the reverse order! 

    trait_man.AddTrait<double>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_ij", "a trait", 7.0); 
    REQUIRE(trait_man.GetSize() == 2); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    trait_man.AddTrait<int, double, std::string>(&nk2_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_ij", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 2); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);

    // -----------------------------------------------------
    // Test to pass valid non-AltType 

    // Create a trait with ints, doubles and strings allowed
    trait_man.AddTrait<int, double, std::string>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_j", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 3); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // ----------------------------------------------------
    // Test pass invalid non-AltType 

    // Create trait that only takes ints and doubles
    trait_man.AddTrait<int, double>(&nk_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_k", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 4); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 


    // Add another module to that trait with non-AltTypes type
    // Check correct error message prints
    trait_man.AddTrait<std::string>(&nk2_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_k", "a trait", "test string"); 
    REQUIRE(trait_man.GetSize() == 4);
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);  
    REQUIRE(error_message == "Module EvalNK is trying to use trait 'trait_k' of type std::string; Previously defined in module(s) EvalNK as type int32_t"); 
    
  }
} 

TEST_CASE("TraitManager_Verify", "[core]") {

  //  [SETUP]
  // Add module(s) to access the trait
  mabe::MABE control(0, NULL);
  control.AddPopulation("test_pop");
  mabe::EvalNK nk_mod(control);
  mabe::EvalNK nk2_mod(control); 
  mabe::EvalNK nk3_mod(control); 

  // Setup a TraitManager
  // Use bools to tell if an error has been thrown 
  bool has_error_been_thrown = false; 
  bool has_error_been_thrown2 = false; 
  bool has_warning_been_thrown = false; 
  std::string error_message = ""; 

  auto error_func = [&has_error_been_thrown, &error_message](const std::string & s){
    std::cout << "Error: ";
    has_error_been_thrown = true;
    error_message = s; 
  }; 
  auto error_func2 = [&has_error_been_thrown2, &error_message](const std::string & s){
    std::cout << "Error: ";
    has_error_been_thrown2 = true;
    error_message = s; 
  }; 
  auto warning_func = [&has_warning_been_thrown](const std::string & s){
    std::cout << "Warning: ";
    has_warning_been_thrown = true; 
  };

  { 
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    error_man.Activate(); 
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    trait_man.Unlock(); 

    // Make sure re-used variables are reset
    has_error_been_thrown = false; 
    has_error_been_thrown2 = false; 
    error_message = ""; 

    //  [BEGIN TESTS] 
    // REQUIRED traits must have another trait (SHARED, GENERATED or OWNED) that writes to them

    // Check that if a REQUIRED trait doesn't have one that adds to them, Verify throws error
    // Check correct error message prints
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::REQUIRED, "trait_i", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1);
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Trait 'trait_i' marked REQUIRED by module(s) 'EvalNK'; must be written to by other modules.\n[Suggestion: set another module to write to this trait (where it is either\n SHARED or OWNED).]");

    // Add a Trait that OWNES the trait, check Verify works
    has_error_been_thrown = false; 
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::OWNED, "trait_i", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    trait_man.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Reset error_message 
    error_message = ""; 

    // Do the check again, this time with a trait that GENERATES
    // Check that if a REQUIRED trait doesn't have one that adds to them, Verify throws error
    // Check correct error message prints
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::REQUIRED, "trait_j", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 2);
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Trait 'trait_j' marked REQUIRED by module(s) 'EvalNK'; must be written to by other modules.\n[Suggestion: set another module to write to this trait (where it is either\n SHARED or OWNED).]");


    // Add a Trait that GENERATES the trait, check Verify works
    has_error_been_thrown = false; 
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::GENERATED, "trait_j", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 2);
    trait_man.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);

    // Reset error message
    error_message = ""; 

    // Do the check again, this time with a trait that SHARES
    // Check that if a REQUIRED trait doesn't have one that adds to them, Verify throws error
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::REQUIRED, "trait_k", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 3);
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Trait 'trait_k' marked REQUIRED by module(s) 'EvalNK'; must be written to by other modules.\n[Suggestion: set another module to write to this trait (where it is either\n SHARED or OWNED).]");

    // Add a Trait that GENERATES the trait, check Verify works
    has_error_been_thrown = false; 
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::SHARED, "trait_k", "a trait", 7); 

    trait_man.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);
  } 
 
  {
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::ErrorManager error_man2(error_func2, warning_func); 
    error_man.Activate(); 
    error_man2.Activate();
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    mabe::TraitManager<mabe::ModuleBase> trait_man2(error_man2);
    trait_man.Unlock(); 
    trait_man2.Unlock();

    // Make sure re-used variables are reset 
    has_error_been_thrown = false;  
    has_error_been_thrown2 = false; 
    error_message = ""; 

    //  [BEGIN TESTS] 
    // A trait that is OWNED or GENERATED cannot have other modules writing to it.

    //Create a new OWNED trait
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::OWNED, "trait_i", "a trait",  7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Check Verify throws error if another module tries OWNING it
    // Check error message is correct
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::OWNED, "trait_i", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Multiple modules declaring ownership of trait 'trait_i': EvalNK and EvalNK.\n[Suggestion: if traits are supposed to be distinct, prepend names with a\n module-specific prefix.  Otherwise modules should be edited to change trait\n to be SHARED (and all can modify) or have all but one shift to REQUIRED.]");

    // ------------------------ Must be done with new Manager to avoid earlier error

    //Create a new OWNED trait
    trait_man2.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::OWNED, "trait_j", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Check Verify throws error if another module tries GENERATING it
    // Check correct error message is shown
    trait_man2.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::GENERATED, "trait_j", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    trait_man2.Verify(true); 
    REQUIRE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(error_message == "Multiple modules declaring ownership of trait 'trait_j': EvalNK and EvalNK.\n[Suggestion: if traits are supposed to be distinct, prepend names with a\n module-specific prefix.  Otherwise modules should be edited to change trait\n to be SHARED (and all can modify) or have all but one shift to REQUIRED.]");
  }

  {
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::ErrorManager error_man2(error_func2, warning_func); 
    error_man.Activate(); 
    error_man2.Activate();
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    mabe::TraitManager<mabe::ModuleBase> trait_man2(error_man2);
    trait_man.Unlock(); 
    trait_man2.Unlock();

    // Make sure re-used variables are reset 
    has_error_been_thrown = false;  
    has_error_been_thrown2 = false; 
    error_message = ""; 

    //  [BEGIN TESTS]
    //Create a new GENERATED trait
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::GENERATED, "trait_k", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Add a module that REQUIRES the GENERATED one
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::REQUIRED, "trait_k", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Check Verify doesn't throw error because no other module tries to OWN or GENERATE 
    trait_man.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Check Verify throws error if another module tries OWNING it
    // Check error message is correct
    trait_man.AddTrait<int>(&nk3_mod, mabe::TraitInfo::Access::OWNED, "trait_k", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Multiple modules declaring ownership of trait 'trait_k': EvalNK and EvalNK.\n[Suggestion: if traits are supposed to be distinct, prepend names with a\n module-specific prefix.  Otherwise modules should be edited to change trait\n to be SHARED (and all can modify) or have all but one shift to REQUIRED.]");

    // ------------------------ Must be done with new Manager to avoid earlier error

    //Create a new GENERATED trait
    trait_man2.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::GENERATED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Add a module that REQUIRES the GENERATED one
    trait_man2.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::REQUIRED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Check Verify doesn't throw error because no other module tries to OWN or GENERATE 
    trait_man2.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Check Verify throws error if another module tries GENERATING it
    trait_man2.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::GENERATED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    trait_man2.Verify(true); 
    REQUIRE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(error_message == "Multiple modules declaring ownership of trait 'trait_l': EvalNK and EvalNK.\n[Suggestion: if traits are supposed to be distinct, prepend names with a\n module-specific prefix.  Otherwise modules should be edited to change trait\n to be SHARED (and all can modify) or have all but one shift to REQUIRED.]");
  }
  
  {
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::ErrorManager error_man2(error_func2, warning_func); 
    error_man.Activate(); 
    error_man2.Activate();
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    mabe::TraitManager<mabe::ModuleBase> trait_man2(error_man2);
    trait_man.Unlock(); 
    trait_man2.Unlock();

    // Setup a TraitManager
    // Reset re-used variables 
    has_error_been_thrown = false; 
    has_error_been_thrown2 = false; 
    error_message = ""; 

    //  [BEGIN TESTS] 
    // A GENERATED trait must have another module REQUIRE it

    // Create a generated trait
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::GENERATED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Verify should fail because another module does not REQUIRE
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);
    REQUIRE(error_message == "Trait 'trait_l' marked GENERATED by module(s) EvalNK'; must be read by other modules."); 

    // Reset bool
    has_error_been_thrown = false; 

    // Add a module that REQUIRES the trait
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::REQUIRED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Verify should succeed now
    trait_man.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // -------------------------------------------------------
    // A trait can only be PRIVATE to one module

    // Another module tries to access it
    // Add a private trait
    trait_man2.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::PRIVATE, "trait_i", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Verify should succeed
    trait_man2.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown2);
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Add another module that accesses it
    trait_man2.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::OPTIONAL, "trait_i", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Verify should fail
    // Check correct error message prints
    trait_man2.Verify(true); 
    REQUIRE(has_error_been_thrown2);
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Trait 'trait_i' is private in module 'EvalNK'; should not be used by other modules.\n[Suggestion: if traits are supposed to be distinct, prepend private name with\n a module-specific prefix.  Otherwise module needs to be edited to not\n have trait private.]");

    //Another module also tries to claim that it's private
    // TODO
  }
    
  {
    //  [FINISH SETUP]
    mabe::ErrorManager error_man(error_func, warning_func);
    mabe::ErrorManager error_man2(error_func2, warning_func); 
    error_man.Activate(); 
    error_man2.Activate();
    mabe::TraitManager<mabe::ModuleBase> trait_man(error_man);
    mabe::TraitManager<mabe::ModuleBase> trait_man2(error_man2);
    trait_man.Unlock(); 
    trait_man2.Unlock();  

    // Reset re-used variables 
    has_error_been_thrown = false; 
    has_error_been_thrown2 = false; 
    error_message = "";  

    //  [BEGIN TESTS] 
    // OWNED/GENERATED traits cannot be SHARED

    // Create a OWNED trait
    trait_man.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::OWNED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown);

    // Verify should succeed
    trait_man.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 

    // Try and SHARE it
    trait_man.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::SHARED, "trait_l", "a trait", 7); 
    REQUIRE(trait_man.GetSize() == 1); 

    // Verify should fail
    trait_man.Verify(true); 
    REQUIRE(has_error_been_thrown); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    REQUIRE(error_message == "Trait 'trait_l' is fully OWNED by module 'EvalNK'; it cannot be SHARED (written to) by other modules:EvalNK[Suggestion: if traits are supposed to be distinct, prepend private name with a\n module-specific prefix.  Otherwise module needs to be edited to make trait\n SHARED or have all but one shift to REQUIRED.]");
    
    // ----------------------------------- Use new Manager to avoid error carry-over

    // Crate a GENERATED trait that also is REQUIRED
    trait_man2.AddTrait<int>(&nk_mod, mabe::TraitInfo::Access::GENERATED, "trait_i", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1);  
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown);


    trait_man2.AddTrait<int>(&nk2_mod, mabe::TraitInfo::Access::REQUIRED, "trait_i", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1);   
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown);

    // Verify should succeed
    trait_man2.Verify(true); 
    REQUIRE_FALSE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); 
    

    // Try and SHARE it
    trait_man2.AddTrait<int>(&nk3_mod, mabe::TraitInfo::Access::SHARED, "trait_i", "a trait", 7); 
    REQUIRE(trait_man2.GetSize() == 1);  
    
    
    // Verify should fail -- fix this check to also provide instructions for if it does the thing we say it should
    /*trait_man2.Verify(true); // << THIS SIGABORTS?
    REQUIRE(has_error_been_thrown2); 
    REQUIRE_FALSE(has_warning_been_thrown); */
    
  }
   
}


