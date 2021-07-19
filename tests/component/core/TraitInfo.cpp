/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for the main MABE object 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "core/TraitInfo.hpp"
#include "core/MABE.hpp"
#include "evaluate/static/EvalNK.hpp"


TEST_CASE("TraitInfo_Basic", "[core]"){
  {
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");

    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);
    mabe::EvalNK nk2_mod(control); 

    // Test the module counts before and after adding a module to the trait
    // Test the modules counts names before and after adding a module to the trait
    // Test the correct name is added to the ModuleNames vector
    REQUIRE(trait_i.GetModuleCount() == 0);
    REQUIRE(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED).size() == 0); 
    trait_i.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::REQUIRED); 
    REQUIRE(trait_i.GetModuleCount() == 1);
    REQUIRE(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED).at(0) == "mod_name"); 
    REQUIRE(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED).size() == 1);

    // Test REQUIRED is actually required. 
    REQUIRE(trait_i.IsRequired()); 

    // Test the description of the trait
    REQUIRE(trait_i.GetDesc() == ""); 
    trait_i.SetDesc("example trait"); 
    REQUIRE(trait_i.GetDesc() == "example trait"); 
    REQUIRE_FALSE(trait_i.GetDesc() == "shouldn't work"); 

    trait_i.SetDesc("new example name"); 
    REQUIRE(trait_i.GetDesc() == "new example name"); 

    // Test the name of the trait
    REQUIRE(trait_i.GetName() == "trait_i"); 
    REQUIRE_FALSE(trait_i.GetName() == "shouldn't work"); 

    trait_i.SetName("new_trait_name"); 
    REQUIRE(trait_i.GetName() == "new_trait_name"); 

    // Test HasAccess for nk and nk2
    REQUIRE(trait_i.HasAccess(&nk_mod) ); 
    REQUIRE(trait_i.HasAccess(&nk2_mod) == false);

  }
}

TEST_CASE("TraitInfo_GetMethods", "[core]") {
  {
    // Create the TraitInfos to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_1("trait_1");
    mabe::TypedTraitInfo<double> trait_2("trait_2");
    mabe::TypedTraitInfo<std::string> trait_3("trait_3");
    mabe::TypedTraitInfo<int> trait_4("trait_4");

    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");

    mabe::EvalNK nk1_mod(control);
    mabe::EvalNK nk2_mod(control); 
    mabe::EvalNK nk3_mod(control);  

    //Check to make sure all traits have no modules using them
    REQUIRE(trait_1.GetUnknownCount() == 0); 
    REQUIRE(trait_1.GetPrivateCount() == 0); 
    REQUIRE(trait_1.GetOwnedCount() == 0); 
    REQUIRE(trait_1.GetGeneratedCount() == 0); 
    REQUIRE(trait_1.GetSharedCount() == 0); 
    REQUIRE(trait_1.GetRequiredCount() == 0); 
    REQUIRE(trait_1.GetOptionalCount() == 0);

    REQUIRE(trait_2.GetUnknownCount() == 0); 
    REQUIRE(trait_2.GetPrivateCount() == 0); 
    REQUIRE(trait_2.GetOwnedCount() == 0); 
    REQUIRE(trait_2.GetGeneratedCount() == 0); 
    REQUIRE(trait_2.GetSharedCount() == 0); 
    REQUIRE(trait_2.GetRequiredCount() == 0); 
    REQUIRE(trait_2.GetOptionalCount() == 0);

    REQUIRE(trait_3.GetUnknownCount() == 0); 
    REQUIRE(trait_3.GetPrivateCount() == 0); 
    REQUIRE(trait_3.GetOwnedCount() == 0); 
    REQUIRE(trait_3.GetGeneratedCount() == 0); 
    REQUIRE(trait_3.GetSharedCount() == 0); 
    REQUIRE(trait_3.GetRequiredCount() == 0); 
    REQUIRE(trait_3.GetOptionalCount() == 0);

    REQUIRE(trait_4.GetUnknownCount() == 0); 
    REQUIRE(trait_4.GetPrivateCount() == 0); 
    REQUIRE(trait_4.GetOwnedCount() == 0); 
    REQUIRE(trait_4.GetGeneratedCount() == 0); 
    REQUIRE(trait_4.GetSharedCount() == 0); 
    REQUIRE(trait_4.GetRequiredCount() == 0); 
    REQUIRE(trait_4.GetOptionalCount() == 0);
    
    // Add the Traits to the modules
    trait_1.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::UNKNOWN); 
    trait_1.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE); 
    trait_1.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::OWNED); 

    trait_2.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::GENERATED);
    trait_2.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::SHARED);
    trait_2.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::REQUIRED); 

    trait_3.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::SHARED);
    trait_3.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE);
    trait_3.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::SHARED);

    trait_4.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::OPTIONAL);
    trait_4.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::OPTIONAL);
    trait_4.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::OWNED);

    // Test to see if Get{ACCESS}Count has updated for each trait
    REQUIRE(trait_1.GetUnknownCount() == 1); 
    REQUIRE(trait_1.GetPrivateCount() == 1); 
    REQUIRE(trait_1.GetOwnedCount() == 1); 
    REQUIRE(trait_1.GetGeneratedCount() == 0); 
    REQUIRE(trait_1.GetSharedCount() == 0); 
    REQUIRE(trait_1.GetRequiredCount() == 0); 
    REQUIRE(trait_1.GetOptionalCount() == 0);

    REQUIRE(trait_2.GetUnknownCount() == 0); 
    REQUIRE(trait_2.GetPrivateCount() == 0); 
    REQUIRE(trait_2.GetOwnedCount() == 0); 
    REQUIRE(trait_2.GetGeneratedCount() == 1); 
    REQUIRE(trait_2.GetSharedCount() == 1); 
    REQUIRE(trait_2.GetRequiredCount() == 1); 
    REQUIRE(trait_2.GetOptionalCount() == 0);

    REQUIRE(trait_3.GetUnknownCount() == 0); 
    REQUIRE(trait_3.GetPrivateCount() == 1); 
    REQUIRE(trait_3.GetOwnedCount() == 0); 
    REQUIRE(trait_3.GetGeneratedCount() == 0); 
    REQUIRE(trait_3.GetSharedCount() == 2); 
    REQUIRE(trait_3.GetRequiredCount() == 0); 
    REQUIRE(trait_3.GetOptionalCount() == 0);

    REQUIRE(trait_4.GetUnknownCount() == 0); 
    REQUIRE(trait_4.GetPrivateCount() == 0); 
    REQUIRE(trait_4.GetOwnedCount() == 1); 
    REQUIRE(trait_4.GetGeneratedCount() == 0); 
    REQUIRE(trait_4.GetSharedCount() == 0); 
    REQUIRE(trait_4.GetRequiredCount() == 0); 
    REQUIRE(trait_4.GetOptionalCount() == 2);

    // Test that GetAccess returns correctly for each type of trait access
    REQUIRE(trait_1.GetAccess(&nk1_mod) == mabe::TraitInfo::Access::UNKNOWN); 
    REQUIRE(trait_1.GetAccess(&nk2_mod) == mabe::TraitInfo::Access::PRIVATE);
    REQUIRE(trait_1.GetAccess(&nk3_mod) == mabe::TraitInfo::Access::OWNED);
    REQUIRE(trait_2.GetAccess(&nk1_mod) == mabe::TraitInfo::Access::GENERATED);
    REQUIRE(trait_2.GetAccess(&nk2_mod) == mabe::TraitInfo::Access::SHARED);
    REQUIRE(trait_2.GetAccess(&nk3_mod) == mabe::TraitInfo::Access::REQUIRED);
    REQUIRE(trait_4.GetAccess(&nk1_mod) == mabe::TraitInfo::Access::OPTIONAL);

    // Add a trait with no accessing modules
    mabe::TypedTraitInfo<int> trait_i("trait_i");

    // Test GetModuleNames (trait version) gets all module names
    REQUIRE(trait_1.GetModuleNames().size() == 3); 
    REQUIRE(trait_1.GetModuleNames().at(0) == "mod1_name"); 
    REQUIRE(trait_1.GetModuleNames().at(1) == "mod2_name"); 
    REQUIRE(trait_1.GetModuleNames().at(2) == "mod3_name"); 

    REQUIRE(trait_i.GetModuleNames().size() == 0); 

    // Test GetModuleNames (access version) gets all module names
    REQUIRE(trait_3.GetModuleNames(mabe::TraitInfo::Access::OWNED).size() == 0); 

    REQUIRE(trait_3.GetModuleNames(mabe::TraitInfo::Access::PRIVATE).size() == 1); 
    REQUIRE(trait_3.GetModuleNames(mabe::TraitInfo::Access::PRIVATE).at(0) == "mod2_name"); 

    REQUIRE(trait_3.GetModuleNames(mabe::TraitInfo::Access::SHARED).size() == 2); 
    REQUIRE(trait_3.GetModuleNames(mabe::TraitInfo::Access::SHARED).at(0) == "mod1_name"); 
    REQUIRE(trait_3.GetModuleNames(mabe::TraitInfo::Access::SHARED).at(1) == "mod3_name"); 
    
  }
}

TEST_CASE("TraitInfo_HasMethods", "[core]") {
  {
    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");

    // Create a module for each type of trait access + 1 that has no access
    mabe::EvalNK nk1_mod(control);
    mabe::EvalNK nk2_mod(control); 
    mabe::EvalNK nk3_mod(control); 
    mabe::EvalNK nk4_mod(control);
    mabe::EvalNK nk5_mod(control); 
    mabe::EvalNK nk6_mod(control); 
    mabe::EvalNK nk7_mod(control);
    mabe::EvalNK nk8_mod(control); 
    //mabe::EvalNK nk3_mod(control); 

    // Create a trait to get each type of access
    mabe::TypedTraitInfo<int> trait_1("trait_1");
    mabe::TypedTraitInfo<double> trait_2("trait_2");
    mabe::TypedTraitInfo<std::string> trait_3("trait_3");
    mabe::TypedTraitInfo<int> trait_4("trait_4");
    mabe::TypedTraitInfo<int> trait_5("trait_5");
    mabe::TypedTraitInfo<double> trait_6("trait_6");
    mabe::TypedTraitInfo<std::string> trait_7("trait_7");

    //Set up access
    trait_1.AddAccess("mod_name", &nk1_mod, mabe::TraitInfo::Access::UNKNOWN); 
    trait_2.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE); 
    trait_3.AddAccess("mod_name", &nk3_mod, mabe::TraitInfo::Access::OWNED); 
    trait_4.AddAccess("mod_name", &nk4_mod, mabe::TraitInfo::Access::GENERATED);
    trait_5.AddAccess("mod_name", &nk5_mod, mabe::TraitInfo::Access::SHARED);
    trait_6.AddAccess("mod_name", &nk6_mod, mabe::TraitInfo::Access::REQUIRED);   
    trait_7.AddAccess("mod_name", &nk7_mod, mabe::TraitInfo::Access::OPTIONAL); 
    

    // Test HasAccess returns correctly for both having and not having access to a trait
    
    //if access is unknown, access it not automatically given
    REQUIRE(trait_1.HasAccess(&nk1_mod) == false); 
    REQUIRE(trait_1.HasAccess(&nk8_mod) == false); 

    REQUIRE(trait_2.HasAccess(&nk2_mod)); 
    REQUIRE(trait_2.HasAccess(&nk8_mod) == false); 

    REQUIRE(trait_3.HasAccess(&nk3_mod)); 
    REQUIRE(trait_3.HasAccess(&nk8_mod) == false); 

    REQUIRE(trait_4.HasAccess(&nk4_mod)); 
    REQUIRE(trait_4.HasAccess(&nk8_mod) == false); 

    REQUIRE(trait_5.HasAccess(&nk5_mod)); 
    REQUIRE(trait_5.HasAccess(&nk8_mod) == false); 
  
    REQUIRE(trait_6.HasAccess(&nk6_mod)); 
    REQUIRE(trait_6.HasAccess(&nk8_mod) == false); 

    REQUIRE(trait_7.HasAccess(&nk7_mod)); 
    REQUIRE(trait_7.HasAccess(&nk8_mod) == false); 
  }
}

TEST_CASE ("TraitInfo_IsMethods", "[core]") {
  {
    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control); 
    mabe::EvalNK nk2_mod(control); 

    // Create a trait to get each type of access
    mabe::TypedTraitInfo<int> trait_1("trait_1");
    mabe::TypedTraitInfo<double> trait_2("trait_2");
    mabe::TypedTraitInfo<std::string> trait_3("trait_3");
    mabe::TypedTraitInfo<int> trait_4("trait_4");
    mabe::TypedTraitInfo<int> trait_5("trait_5");
    mabe::TypedTraitInfo<double> trait_6("trait_6");
    mabe::TypedTraitInfo<double> trait_7("trait_7");
    
    // Set up access
    trait_1.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::PRIVATE); 
    trait_2.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::OWNED); 
    trait_3.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::GENERATED);
    trait_4.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::SHARED);
    trait_5.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::REQUIRED);   
    trait_6.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::OPTIONAL); 
    trait_7.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::GENERATED); 
    trait_7.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::GENERATED); 


    // Check Is{ACESS} methods work
    REQUIRE(trait_1.IsPrivate()); 
    REQUIRE(trait_2.IsOwned()); 
    REQUIRE(trait_3.IsGenerated()); 
    REQUIRE(trait_4.IsShared()); 
    REQUIRE(trait_5.IsRequired()); 
    REQUIRE(trait_6.IsOptional());
    //check that when multiple modules have same access, returns correctly
    REQUIRE(trait_7.IsGenerated()); 

    // Check Is{ACCESS} methods return false when access isn't there
    REQUIRE(trait_2.IsPrivate() == false); 
    REQUIRE(trait_3.IsOwned() == false); 
    REQUIRE(trait_4.IsGenerated() == false); 
    REQUIRE(trait_5.IsShared() == false); 
    REQUIRE(trait_6.IsRequired() == false); 
    REQUIRE(trait_1.IsOptional() == false);
    REQUIRE(trait_7.IsShared() == false); 

  }
}


// Helpful to look at the TypeID tests in empirical, located 
//   ../../../source/third-party/empirical/tests/meta/TypeID.cpp
TEST_CASE("TraitInfo_TypesMethods", "[core]") {
  {
    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);
    mabe::TypedTraitInfo<int> trait_int("trait_int");
    mabe::TypedTraitInfo<double> trait_double("trait_double");
    mabe::TypedTraitInfo<std::string> trait_string("trait_string");

    // Test GetType returns main type
    REQUIRE(trait_int.GetType().GetName() == "int32_t"); 
    REQUIRE(trait_double.GetType().GetName() == "double"); 
    REQUIRE(trait_string.GetType().GetName() == "std::string");

    // Test AltType vector initially empty
    REQUIRE(trait_int.GetAltTypes().size() == 0); 

    // Test IsAllowedType includes main type
    //REQUIRE(trait_int.IsAllowedType(trait_int.GetType()));
    //REQUIRE(trait_double.IsAllowedType(trait_double.GetType()));
    //REQUIRE(trait_string.IsAllowedType(trait_string.GetType()));






   



    
  }
}

TEST_CASE("TraitInfo_DefaultMethods", "[core]") {
  {
    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control); 

    // -------------------------------------------------

    // [INT] 
    //   Create trait without default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<int> trait_i("trait_i");

    REQUIRE(trait_i.HasDefault() == false); 
    trait_i.SetDefault(7); 
    REQUIRE(trait_i.HasDefault()); 
    REQUIRE(trait_i.GetDefault() == 7); 

    //   Create trait with default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<int> trait_j("trait_j", 7);

    REQUIRE(trait_j.HasDefault()); 
    REQUIRE(trait_j.GetDefault() == 7);

    trait_j.SetDefault(10); 
    REQUIRE(trait_j.HasDefault()); 
    REQUIRE(trait_j.GetDefault() == 10);  

    // -------------------------------------------------

    // [DOUBLE] 
    //   Create trait without default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<double> trait_k("trait_k");

    REQUIRE(trait_k.HasDefault() == false); 
    trait_k.SetDefault(7.0); 
    REQUIRE(trait_k.HasDefault()); 
    REQUIRE(trait_k.GetDefault() == 7.0); 

    //   Create trait with default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<int> trait_l("trait_l", 7.0);

    REQUIRE(trait_l.HasDefault()); 
    REQUIRE(trait_l.GetDefault() == 7.0);

    trait_l.SetDefault(10.0); 
    REQUIRE(trait_l.HasDefault()); 
    REQUIRE(trait_l.GetDefault() == 10.0); 

    // -------------------------------------------------

    // [STRING] 
    //   Create trait without default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<std::string> trait_m("trait_m");

    REQUIRE(trait_m.HasDefault() == false); 
    trait_m.SetDefault("test string"); 
    REQUIRE(trait_m.HasDefault()); 
    REQUIRE(trait_m.GetDefault() == "test string"); 

    //   Create trait with default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<std::string> trait_n("trait_n", "test string");

    REQUIRE(trait_n.HasDefault()); 
    REQUIRE(trait_n.GetDefault() == "test string"); 

    trait_n.SetDefault("new test string"); 
    REQUIRE(trait_n.HasDefault()); 
    REQUIRE(trait_n.GetDefault() == "new test string"); 
  }
}
