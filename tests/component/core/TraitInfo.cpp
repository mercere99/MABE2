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


TEST_CASE("TraitInfoBasic", "[core]"){
  {
    // Create the TraitInfo to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_i("trait_i");

    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control);
    mabe::EvalNK nk2_mod(control); 

    // Test the module counts before and after adding a module to the trait
    // Test that the correct module name is associated with the trait
    REQUIRE(trait_i.GetModuleCount() == 0);
//!!!!!!REQUIRE(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED) == ""); 
    trait_i.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::REQUIRED); 
    REQUIRE(trait_i.GetModuleCount() == 1);
//!!!!!REQUIRE(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED) == "mod_name"); 

    // Test REQUIRED is actually required. 
    REQUIRE(trait_i.IsRequired() == true); 

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
    REQUIRE(trait_i.HasAccess(&nk_mod) == true); 
    REQUIRE(trait_i.HasAccess(&nk2_mod) == false);

  }
}

TEST_CASE("TraitInfoGet{ACCESS}Count", "[core]") {
  {
    // Create the TraitInfos to be tested (TraitInfo is abstract so we must make a TypedTraitInfo)
    mabe::TypedTraitInfo<int> trait_1("trait_1");
    mabe::TypedTraitInfo<int> trait_2("trait_2");
    mabe::TypedTraitInfo<int> trait_3("trait_3");

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
    
    // Add the Traits to the modules
    trait_1.AddAccess("mod_name", &nk1_mod, mabe::TraitInfo::Access::UNKNOWN); 
    trait_1.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE); 
    trait_1.AddAccess("mod_name", &nk3_mod, mabe::TraitInfo::Access::OWNED); 

    trait_2.AddAccess("mod_name", &nk1_mod, mabe::TraitInfo::Access::GENERATED);
    trait_2.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::SHARED);
    trait_2.AddAccess("mod_name", &nk3_mod, mabe::TraitInfo::Access::REQUIRED); 

    trait_3.AddAccess("mod_name", &nk1_mod, mabe::TraitInfo::Access::SHARED);
    trait_3.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE);
    trait_3.AddAccess("mod_name", &nk3_mod, mabe::TraitInfo::Access::SHARED);


    // Test to see if Get{ACCESS}Count has updated
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

  }
}
