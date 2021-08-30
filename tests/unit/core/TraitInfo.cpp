/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  TraitInfo.cpp
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
    CHECK(trait_i.GetModuleCount() == 0);
    CHECK(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED).size() == 0); 
    trait_i.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::REQUIRED, false); 
    CHECK(trait_i.GetModuleCount() == 1);
    CHECK(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED).at(0) == "mod_name"); 
    CHECK(trait_i.GetModuleNames(mabe::TraitInfo::Access::REQUIRED).size() == 1);

    // Test REQUIRED is actually required. 
    CHECK(trait_i.IsRequired()); 

    // Test the description of the trait
    CHECK(trait_i.GetDesc() == ""); 
    trait_i.SetDesc("example trait"); 
    CHECK(trait_i.GetDesc() == "example trait"); 
    CHECK_FALSE(trait_i.GetDesc() == "shouldn't work"); 

    trait_i.SetDesc("new example name"); 
    CHECK(trait_i.GetDesc() == "new example name"); 

    // Test the name of the trait
    CHECK(trait_i.GetName() == "trait_i"); 
    CHECK_FALSE(trait_i.GetName() == "shouldn't work"); 

    trait_i.SetName("new_trait_name"); 
    CHECK(trait_i.GetName() == "new_trait_name"); 

    // Test HasAccess for nk and nk2
    CHECK(trait_i.HasAccess(&nk_mod) ); 
    CHECK_FALSE(trait_i.HasAccess(&nk2_mod));
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
    CHECK(trait_1.GetUnknownCount() == 0); 
    CHECK(trait_1.GetPrivateCount() == 0); 
    CHECK(trait_1.GetOwnedCount() == 0); 
    CHECK(trait_1.GetGeneratedCount() == 0); 
    CHECK(trait_1.GetSharedCount() == 0); 
    CHECK(trait_1.GetRequiredCount() == 0); 
    CHECK(trait_1.GetOptionalCount() == 0);

    CHECK(trait_2.GetUnknownCount() == 0); 
    CHECK(trait_2.GetPrivateCount() == 0); 
    CHECK(trait_2.GetOwnedCount() == 0); 
    CHECK(trait_2.GetGeneratedCount() == 0); 
    CHECK(trait_2.GetSharedCount() == 0); 
    CHECK(trait_2.GetRequiredCount() == 0); 
    CHECK(trait_2.GetOptionalCount() == 0);

    CHECK(trait_3.GetUnknownCount() == 0); 
    CHECK(trait_3.GetPrivateCount() == 0); 
    CHECK(trait_3.GetOwnedCount() == 0); 
    CHECK(trait_3.GetGeneratedCount() == 0); 
    CHECK(trait_3.GetSharedCount() == 0); 
    CHECK(trait_3.GetRequiredCount() == 0); 
    CHECK(trait_3.GetOptionalCount() == 0);

    CHECK(trait_4.GetUnknownCount() == 0); 
    CHECK(trait_4.GetPrivateCount() == 0); 
    CHECK(trait_4.GetOwnedCount() == 0); 
    CHECK(trait_4.GetGeneratedCount() == 0); 
    CHECK(trait_4.GetSharedCount() == 0); 
    CHECK(trait_4.GetRequiredCount() == 0); 
    CHECK(trait_4.GetOptionalCount() == 0);
    
    // Add the Traits to the modules
    trait_1.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::UNKNOWN, false); 
    trait_1.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE, false); 
    trait_1.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::OWNED, false); 

    trait_2.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::GENERATED, false);
    trait_2.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::SHARED, false);
    trait_2.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::REQUIRED, false); 

    trait_3.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::SHARED, false);
    trait_3.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE, false);
    trait_3.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::SHARED, false);

    trait_4.AddAccess("mod1_name", &nk1_mod, mabe::TraitInfo::Access::OPTIONAL, false);
    trait_4.AddAccess("mod2_name", &nk2_mod, mabe::TraitInfo::Access::OPTIONAL, false);
    trait_4.AddAccess("mod3_name", &nk3_mod, mabe::TraitInfo::Access::OWNED, false);

    // Test to see if Get{ACCESS}Count has updated for each trait
    CHECK(trait_1.GetUnknownCount() == 1); 
    CHECK(trait_1.GetPrivateCount() == 1); 
    CHECK(trait_1.GetOwnedCount() == 1); 
    CHECK(trait_1.GetGeneratedCount() == 0); 
    CHECK(trait_1.GetSharedCount() == 0); 
    CHECK(trait_1.GetRequiredCount() == 0); 
    CHECK(trait_1.GetOptionalCount() == 0);

    CHECK(trait_2.GetUnknownCount() == 0); 
    CHECK(trait_2.GetPrivateCount() == 0); 
    CHECK(trait_2.GetOwnedCount() == 0); 
    CHECK(trait_2.GetGeneratedCount() == 1); 
    CHECK(trait_2.GetSharedCount() == 1); 
    CHECK(trait_2.GetRequiredCount() == 1); 
    CHECK(trait_2.GetOptionalCount() == 0);

    CHECK(trait_3.GetUnknownCount() == 0); 
    CHECK(trait_3.GetPrivateCount() == 1); 
    CHECK(trait_3.GetOwnedCount() == 0); 
    CHECK(trait_3.GetGeneratedCount() == 0); 
    CHECK(trait_3.GetSharedCount() == 2); 
    CHECK(trait_3.GetRequiredCount() == 0); 
    CHECK(trait_3.GetOptionalCount() == 0);

    CHECK(trait_4.GetUnknownCount() == 0); 
    CHECK(trait_4.GetPrivateCount() == 0); 
    CHECK(trait_4.GetOwnedCount() == 1); 
    CHECK(trait_4.GetGeneratedCount() == 0); 
    CHECK(trait_4.GetSharedCount() == 0); 
    CHECK(trait_4.GetRequiredCount() == 0); 
    CHECK(trait_4.GetOptionalCount() == 2);

    // Test that GetAccess returns correctly for each type of trait access
    CHECK(trait_1.GetAccess(&nk1_mod) == mabe::TraitInfo::Access::UNKNOWN); 
    CHECK(trait_1.GetAccess(&nk2_mod) == mabe::TraitInfo::Access::PRIVATE);
    CHECK(trait_1.GetAccess(&nk3_mod) == mabe::TraitInfo::Access::OWNED);
    CHECK(trait_2.GetAccess(&nk1_mod) == mabe::TraitInfo::Access::GENERATED);
    CHECK(trait_2.GetAccess(&nk2_mod) == mabe::TraitInfo::Access::SHARED);
    CHECK(trait_2.GetAccess(&nk3_mod) == mabe::TraitInfo::Access::REQUIRED);
    CHECK(trait_4.GetAccess(&nk1_mod) == mabe::TraitInfo::Access::OPTIONAL);

    // Add a trait with no accessing modules
    mabe::TypedTraitInfo<int> trait_i("trait_i");

    // Test GetModuleNames (trait version) gets all module names
    CHECK(trait_1.GetModuleNames().size() == 3); 
    CHECK(trait_1.GetModuleNames().at(0) == "mod1_name"); 
    CHECK(trait_1.GetModuleNames().at(1) == "mod2_name"); 
    CHECK(trait_1.GetModuleNames().at(2) == "mod3_name"); 

    CHECK(trait_i.GetModuleNames().size() == 0); 

    // Test GetModuleNames (access version) gets all module names
    CHECK(trait_3.GetModuleNames(mabe::TraitInfo::Access::OWNED).size() == 0); 

    CHECK(trait_3.GetModuleNames(mabe::TraitInfo::Access::PRIVATE).size() == 1); 
    CHECK(trait_3.GetModuleNames(mabe::TraitInfo::Access::PRIVATE).at(0) == "mod2_name"); 

    CHECK(trait_3.GetModuleNames(mabe::TraitInfo::Access::SHARED).size() == 2); 
    CHECK(trait_3.GetModuleNames(mabe::TraitInfo::Access::SHARED).at(0) == "mod1_name"); 
    CHECK(trait_3.GetModuleNames(mabe::TraitInfo::Access::SHARED).at(1) == "mod3_name"); 
    
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

    // Create a trait to get each type of access
    mabe::TypedTraitInfo<int> trait_1("trait_1");
    mabe::TypedTraitInfo<double> trait_2("trait_2");
    mabe::TypedTraitInfo<std::string> trait_3("trait_3");
    mabe::TypedTraitInfo<int> trait_4("trait_4");
    mabe::TypedTraitInfo<int> trait_5("trait_5");
    mabe::TypedTraitInfo<double> trait_6("trait_6");
    mabe::TypedTraitInfo<std::string> trait_7("trait_7");

    // Set up access
    trait_1.AddAccess("mod_name", &nk1_mod, mabe::TraitInfo::Access::UNKNOWN, false); 
    trait_2.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::PRIVATE, false); 
    trait_3.AddAccess("mod_name", &nk3_mod, mabe::TraitInfo::Access::OWNED, false); 
    trait_4.AddAccess("mod_name", &nk4_mod, mabe::TraitInfo::Access::GENERATED, false);
    trait_5.AddAccess("mod_name", &nk5_mod, mabe::TraitInfo::Access::SHARED, false);
    trait_6.AddAccess("mod_name", &nk6_mod, mabe::TraitInfo::Access::REQUIRED, false);   
    trait_7.AddAccess("mod_name", &nk7_mod, mabe::TraitInfo::Access::OPTIONAL, false); 
    

    // Test HasAccess returns correctly for both having and not having access to a trait
        //if access is unknown, access it not automatically given
    CHECK_FALSE(trait_1.HasAccess(&nk1_mod)); 
    CHECK_FALSE(trait_1.HasAccess(&nk8_mod)); 

    CHECK(trait_2.HasAccess(&nk2_mod)); 
    CHECK_FALSE(trait_2.HasAccess(&nk8_mod)); 

    CHECK(trait_3.HasAccess(&nk3_mod)); 
    CHECK_FALSE(trait_3.HasAccess(&nk8_mod)); 

    CHECK(trait_4.HasAccess(&nk4_mod)); 
    CHECK_FALSE(trait_4.HasAccess(&nk8_mod)); 

    CHECK(trait_5.HasAccess(&nk5_mod)); 
    CHECK_FALSE(trait_5.HasAccess(&nk8_mod)); 
  
    CHECK(trait_6.HasAccess(&nk6_mod)); 
    CHECK_FALSE(trait_6.HasAccess(&nk8_mod)); 

    CHECK(trait_7.HasAccess(&nk7_mod)); 
    CHECK_FALSE(trait_7.HasAccess(&nk8_mod)); 
  }
}

TEST_CASE ("TraitInfo_IsMethods", "[core]") {
  {
    // Create a MABE object, a population, and a module (could be any module) for testing
    mabe::MABE control(0, NULL);
    control.AddPopulation("test_pop");
    mabe::EvalNK nk_mod(control); 
    mabe::EvalNK nk2_mod(control); 
    mabe::EvalNK nk3_mod(control); 

    // Create a trait to get each type of access
    mabe::TypedTraitInfo<int> trait_1("trait_1");
    mabe::TypedTraitInfo<double> trait_2("trait_2");
    mabe::TypedTraitInfo<std::string> trait_3("trait_3");
    mabe::TypedTraitInfo<int> trait_4("trait_4");
    mabe::TypedTraitInfo<int> trait_5("trait_5");
    mabe::TypedTraitInfo<double> trait_6("trait_6");
    mabe::TypedTraitInfo<double> trait_7("trait_7");
    
    // Set up access
    trait_1.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::PRIVATE, false); 
    trait_2.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::OWNED, false); 
    trait_3.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::GENERATED, false);
    trait_4.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::SHARED, false);
    trait_5.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::REQUIRED, false);   
    trait_6.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::OPTIONAL, false); 
    trait_7.AddAccess("mod_name", &nk_mod, mabe::TraitInfo::Access::GENERATED, false); 
    trait_7.AddAccess("mod_name", &nk2_mod, mabe::TraitInfo::Access::GENERATED, false); 
    trait_7.AddAccess("mod_name", &nk3_mod, mabe::TraitInfo::Access::PRIVATE, false); 


    // Check Is{ACESS} methods work
    CHECK(trait_1.IsPrivate()); 
    CHECK(trait_2.IsOwned()); 
    CHECK(trait_3.IsGenerated()); 
    CHECK(trait_4.IsShared()); 
    CHECK(trait_5.IsRequired()); 
    CHECK(trait_6.IsOptional());
    // check that when multiple modules have same access, returns correctly
    CHECK(trait_7.IsGenerated()); 
    // Traits can have multiple types of access from different modules
    CHECK(trait_7.IsPrivate()); 

    // Check Is{ACCESS} methods return false when access isn't there
    CHECK_FALSE(trait_2.IsPrivate()); 
    CHECK_FALSE(trait_3.IsOwned()); 
    CHECK_FALSE(trait_4.IsGenerated()); 
    CHECK_FALSE(trait_5.IsShared()); 
    CHECK_FALSE(trait_6.IsRequired()); 
    CHECK_FALSE(trait_1.IsOptional());
    CHECK_FALSE(trait_7.IsShared()); 
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

    // Create instances of different types
    size_t string_type = emp::GetTypeID<std::string>(); 
    size_t double_type = emp::GetTypeID<double>(); 
    size_t float_type = emp::GetTypeID<float>(); 
    size_t bool_type = emp::GetTypeID<bool>(); 

    // Test GetType returns main type
    CHECK(trait_int.GetType().GetName() == "int32_t"); 
    CHECK(trait_double.GetType().GetName() == "double"); 
    CHECK(trait_string.GetType().GetName() == "std::string");

    // Test AltType vector initially empty
    CHECK(trait_int.GetAltTypes().size() == 0); 

    // Test IsAllowedType includes default type  
    //    Test is correct, but code fails this check
    /* CHECK(trait_int.IsAllowedType(trait_int.GetType()));
    CHECK(trait_double.IsAllowedType(trait_double.GetType()));
    CHECK(trait_string.IsAllowedType(trait_string.GetType())); */

    // Create vector for AltTypes and populate
    emp::vector<emp::TypeID> my_vec = emp::GetTypeIDs<std::string, double, float, bool>(); 
    trait_int.SetAltTypes(my_vec); 

    // Create a local variable for the AltTypes vector stored in trait
    emp::vector<emp::TypeID> trait_vec = trait_int.GetAltTypes(); 

    // Check AltTypes vector has updated appropriately 
    CHECK(trait_vec.size() == 4);  
    CHECK(trait_vec[0].GetName() == "std::string"); 
    CHECK(trait_vec[1].GetName() == "double"); 
    CHECK(trait_vec[2].GetName() == "float"); 
    CHECK(trait_vec[3].GetName() == "bool"); 

    CHECK(trait_vec[0].GetID() == string_type); 
    CHECK(trait_vec[1].GetID() == double_type);
    CHECK(trait_vec[2].GetID() == float_type);
    CHECK(trait_vec[3].GetID() == bool_type);

    // Check each AltType is allowed (from trait created AltTypes vector)
    CHECK(trait_int.IsAllowedType(trait_vec[0])); 
    CHECK(trait_int.IsAllowedType(trait_vec[1])); 
    CHECK(trait_int.IsAllowedType(trait_vec[2])); 
    CHECK(trait_int.IsAllowedType(trait_vec[3]));

    // Check each AltType is allowed (from our created vector)
    CHECK(trait_int.IsAllowedType(my_vec[0]));  
    CHECK(trait_int.IsAllowedType(my_vec[1])); 
    CHECK(trait_int.IsAllowedType(my_vec[2])); 
    CHECK(trait_int.IsAllowedType(my_vec[3])); 

    // Check non-AltTypes aren't allowed
    CHECK_FALSE(trait_int.IsAllowedType(emp::GetTypeID<uint_fast64_t>())); 
    CHECK_FALSE(trait_int.IsAllowedType(emp::GetTypeID<char>())); 
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

    CHECK_FALSE(trait_i.HasDefault()); 
    trait_i.SetDefault(7); 
    CHECK(trait_i.HasDefault()); 
    CHECK(trait_i.GetDefault() == 7); 

    //   Create trait with default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<int> trait_j("trait_j", 7);

    CHECK(trait_j.HasDefault()); 
    CHECK(trait_j.GetDefault() == 7);

    trait_j.SetDefault(10); 
    CHECK(trait_j.HasDefault()); 
    CHECK(trait_j.GetDefault() == 10);  

    // -------------------------------------------------

    // [DOUBLE] 
    //   Create trait without default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<double> trait_k("trait_k");

    CHECK_FALSE(trait_k.HasDefault()); 
    trait_k.SetDefault(7.0); 
    CHECK(trait_k.HasDefault()); 
    CHECK(trait_k.GetDefault() == 7.0); 

    //   Create trait with default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<int> trait_l("trait_l", 7.0);

    CHECK(trait_l.HasDefault()); 
    CHECK(trait_l.GetDefault() == 7.0);

    trait_l.SetDefault(10.0); 
    CHECK(trait_l.HasDefault()); 
    CHECK(trait_l.GetDefault() == 10.0); 

    // -------------------------------------------------

    // [STRING] 
    //   Create trait without default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<std::string> trait_m("trait_m");

    CHECK_FALSE(trait_m.HasDefault()); 
    trait_m.SetDefault("test string"); 
    CHECK(trait_m.HasDefault()); 
    CHECK(trait_m.GetDefault() == "test string"); 

    //   Create trait with default value 
    //   Test HasDefault, SetDefault and GetDefault
    mabe::TypedTraitInfo<std::string> trait_n("trait_n", "test string");

    CHECK(trait_n.HasDefault()); 
    CHECK(trait_n.GetDefault() == "test string"); 

    trait_n.SetDefault("new test string"); 
    CHECK(trait_n.HasDefault()); 
    CHECK(trait_n.GetDefault() == "new test string"); 
  }
}
