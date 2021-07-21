/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for ConfigEntry with various types and edge cases 
 */

//#include <functional>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "config/ConfigEntry.hpp"
#include "config/ConfigScope.hpp"

// set min and max? but with set max
TEST_CASE("ConfigEntry_Linker_Int", "[config]"){
  {
    int v = 0;
    mabe::ConfigEntry_Linked<int> linked_entry_int("name00", v, "variable00", nullptr);

    // Test As() functions
    REQUIRE(linked_entry_int.AsDouble() == 0.0);
    std::string s00 = linked_entry_int.AsString();
    REQUIRE(s00.compare("0") == 0);

    // Test updating variable, ConfigEntry should change
    v = 1;

    REQUIRE(linked_entry_int.AsDouble() == 1.0);
    std::string s01 = linked_entry_int.AsString();
    REQUIRE(s01.compare("1") == 0);

    // Test bool functions 
    REQUIRE(linked_entry_int.IsTemporary() == false); // should always be false
    REQUIRE(linked_entry_int.IsBuiltIn() == false); // should all be false
    REQUIRE(linked_entry_int.IsNumeric() == true);
    REQUIRE(linked_entry_int.IsBool() == false);
    REQUIRE(linked_entry_int.IsInt() == true);
    REQUIRE(linked_entry_int.IsDouble() == false);
    REQUIRE(linked_entry_int.IsString() == false);
    REQUIRE(linked_entry_int.IsLocal() == false); // should this be true? def flase for linked, local for var
    REQUIRE(linked_entry_int.IsTemporary() == false);
    REQUIRE(linked_entry_int.IsBuiltIn() == false);
    REQUIRE(linked_entry_int.IsFunction() == false);
    REQUIRE(linked_entry_int.IsScope() == false);
    REQUIRE(linked_entry_int.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_int.GetName();
    REQUIRE(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_int.GetDesc();
    REQUIRE(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigScope> ptr = linked_entry_int.GetScope(); // should include config scope at the top, if doesn't work deal with when working on config scope
    REQUIRE(ptr == nullptr); // come back to after looking at config scope 
    std::string type = linked_entry_int.GetTypename();
    REQUIRE(type.compare("Value") == 0);

    // Test setter functions
    linked_entry_int.SetName("name01");
    std::string name01 = linked_entry_int.GetName();
    REQUIRE(name01.compare("name01") == 0);
    linked_entry_int.SetDesc("desc01");
    std::string desc01 = linked_entry_int.GetDesc();
    REQUIRE(desc01.compare("desc01") == 0);
    linked_entry_int.SetTemporary();
    REQUIRE(linked_entry_int.IsTemporary() == true);
    linked_entry_int.SetBuiltIn();
    REQUIRE(linked_entry_int.IsBuiltIn() == true);

    // Test setter functions, should update original variable
    linked_entry_int.SetValue(2.0);
    REQUIRE(linked_entry_int.AsDouble() == 2.0);
    REQUIRE(v == 2.0);
    linked_entry_int.SetString("3");
    std::string s02 = linked_entry_int.AsString();
    REQUIRE(s02.compare("3") == 0);
    REQUIRE(v == 3);

    // Test Clone()
    //emp::Ptr<ConfigEntry> clone_ptr = linked_entry_int.Clone();
    //REQUIRE(clone_ptr)
  }
}

TEST_CASE("ConfigEntry_Linker_Double", "[config]"){
  {
    double v = 0.0;
    mabe::ConfigEntry_Linked<double> linked_entry_double("name00", v, "variable00", nullptr);

    // Test As() functions
    REQUIRE(linked_entry_double.AsDouble() == 0.0);
    std::string s00 = linked_entry_double.AsString();
    REQUIRE(s00.compare("0") == 0);

    // Test updating variable, ConfigEntry should change
    v = 1;

    REQUIRE(linked_entry_double.AsDouble() == 1.0);
    std::string s01 = linked_entry_double.AsString();
    REQUIRE(s01.compare("1") == 0);

    // Test bool functions 
    REQUIRE(linked_entry_double.IsTemporary() == false);
    REQUIRE(linked_entry_double.IsBuiltIn() == false);
    REQUIRE(linked_entry_double.IsNumeric() == true);
    REQUIRE(linked_entry_double.IsBool() == false);
    REQUIRE(linked_entry_double.IsInt() == false);
    REQUIRE(linked_entry_double.IsDouble() == true);
    REQUIRE(linked_entry_double.IsString() == false);
    REQUIRE(linked_entry_double.IsLocal() == false);
    REQUIRE(linked_entry_double.IsTemporary() == false);
    REQUIRE(linked_entry_double.IsBuiltIn() == false);
    REQUIRE(linked_entry_double.IsFunction() == false);
    REQUIRE(linked_entry_double.IsScope() == false);
    REQUIRE(linked_entry_double.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_double.GetName();
    REQUIRE(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_double.GetDesc();
    REQUIRE(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigScope> ptr = linked_entry_double.GetScope();
    REQUIRE(ptr == nullptr);
    std::string type = linked_entry_double.GetTypename();
    REQUIRE(type.compare("Value") == 0);

    // Test setter functions
    linked_entry_double.SetName("name01");
    std::string name01 = linked_entry_double.GetName();
    REQUIRE(name01.compare("name01") == 0);
    linked_entry_double.SetDesc("desc01");
    std::string desc01 = linked_entry_double.GetDesc();
    REQUIRE(desc01.compare("desc01") == 0);
    linked_entry_double.SetTemporary();
    REQUIRE(linked_entry_double.IsTemporary() == true);
    linked_entry_double.SetBuiltIn();
    REQUIRE(linked_entry_double.IsBuiltIn() == true);

    // Test setter functions, original variable should change
    linked_entry_double.SetValue(2.0);
    REQUIRE(linked_entry_double.AsDouble() == 2.0);
    linked_entry_double.SetString("3");
    std::string s02 = linked_entry_double.AsString();
    REQUIRE(s02.compare("3") == 0);
  }
}

TEST_CASE("ConfigLEntry_Linker<std::string>", "[config]"){
  {
    std::string v = "0";
    mabe::ConfigEntry_Linked<std::string> linked_entry_str("name00", v, "variable00", nullptr);

    // Test As() functions
    REQUIRE(linked_entry_str.AsDouble() == 0.0);
    std::string s00 = linked_entry_str.AsString();
    REQUIRE(s00.compare("0") == 0);

    // Test updating variable, ConfigEntry should change
    v = "1";

    REQUIRE(linked_entry_str.AsDouble() == 1.0);
    std::string s01 = linked_entry_str.AsString();
    REQUIRE(s01.compare("1") == 0);

    // Test bool functions 
    REQUIRE(linked_entry_str.IsTemporary() == false);
    REQUIRE(linked_entry_str.IsBuiltIn() == false);
    REQUIRE(linked_entry_str.IsNumeric() == false);
    REQUIRE(linked_entry_str.IsBool() == false);
    REQUIRE(linked_entry_str.IsInt() == false);
    REQUIRE(linked_entry_str.IsDouble() == false);
    REQUIRE(linked_entry_str.IsString() == true);
    REQUIRE(linked_entry_str.IsLocal() == false);
    REQUIRE(linked_entry_str.IsTemporary() == false);
    REQUIRE(linked_entry_str.IsBuiltIn() == false);
    REQUIRE(linked_entry_str.IsFunction() == false);
    REQUIRE(linked_entry_str.IsScope() == false);
    REQUIRE(linked_entry_str.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_str.GetName();
    REQUIRE(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_str.GetDesc();
    REQUIRE(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigScope> ptr = linked_entry_str.GetScope();
    REQUIRE(ptr == nullptr);
    std::string type = linked_entry_str.GetTypename();
    REQUIRE(type.compare("String") == 0);

    // Test setter functions
    linked_entry_str.SetName("name01");
    std::string name01 = linked_entry_str.GetName();
    REQUIRE(name01.compare("name01") == 0);
    linked_entry_str.SetDesc("desc01");
    std::string desc01 = linked_entry_str.GetDesc();
    REQUIRE(desc01.compare("desc01") == 0);
    linked_entry_str.SetTemporary();
    REQUIRE(linked_entry_str.IsTemporary() == true);
    linked_entry_str.SetBuiltIn();
    REQUIRE(linked_entry_str.IsBuiltIn() == true);

    // Test setter functions, original variable should change
    linked_entry_str.SetValue(2.0);
    REQUIRE(linked_entry_str.AsDouble() == 2.0);
    REQUIRE(v == "2"); 
    linked_entry_str.SetValue(2.5);
    REQUIRE(linked_entry_str.AsDouble() == 2.5);
    REQUIRE(v == "2.5"); 
    linked_entry_str.SetString("3");
    std::string s02 = linked_entry_str.AsString();
    REQUIRE(s02.compare("3") == 0);
    REQUIRE(v == "3");
    
  }
}


int v = 0;
template<typename T>
T getter() {
  return (T) v;
}

template<typename T>
void setter(const T & in) {
  v += (int) in;
}

TEST_CASE("ConfigEntry_Functions", "[config]"){
  {
    mabe::ConfigEntry_Functions<int> linker_functions("name00", getter, setter, "desc00", nullptr);

    // Test As() functions
    REQUIRE(linker_functions.AsDouble() == 0.0);
    std::string s00 = linker_functions.AsString();
    REQUIRE(s00.compare("0") == 0);

    // Test bool functions 
    REQUIRE(linker_functions.IsTemporary() == false);
    REQUIRE(linker_functions.IsBuiltIn() == false);
    REQUIRE(linker_functions.IsNumeric() == true);
    REQUIRE(linker_functions.IsBool() == false);
    REQUIRE(linker_functions.IsInt() == true);
    REQUIRE(linker_functions.IsDouble() == false);
    REQUIRE(linker_functions.IsString() == false);
    REQUIRE(linker_functions.IsLocal() == false);
    REQUIRE(linker_functions.IsTemporary() == false);
    REQUIRE(linker_functions.IsBuiltIn() == false);
    REQUIRE(linker_functions.IsFunction() == true);
    REQUIRE(linker_functions.IsScope() == false);
    REQUIRE(linker_functions.IsError() == false);

    // Test getter functions
    std::string name00 = linker_functions.GetName();
    REQUIRE(name00.compare("name00") == 0);
    emp::Ptr<mabe::ConfigScope> ptr = linker_functions.GetScope();
    REQUIRE(ptr == nullptr);
    std::string type = linker_functions.GetTypename();
    REQUIRE(type.compare("[[Function]]") == 0);

    // Test setter functions
    linker_functions.SetName("name01");
    std::string name01 = linker_functions.GetName();
    REQUIRE(name01.compare("name01") == 0);
    linker_functions.SetTemporary();
    REQUIRE(linker_functions.IsTemporary() == true);
    linker_functions.SetBuiltIn();
    REQUIRE(linker_functions.IsBuiltIn() == true);

    // Test setter functions, original variable should change
    linker_functions.SetValue(2.0);
    REQUIRE(linker_functions.AsDouble() == 2.0);
    linker_functions.SetValue(2.5);
    REQUIRE(linker_functions.AsDouble() == 2.5);
    linker_functions.SetString("3");
    std::string s02 = linker_functions.AsString();
    REQUIRE(s02.compare("3") == 0);

    // assume nothing about variable
    // calling getter function will return some variable
      // what ever type we said, give something of this type
      // mirrors linked config entry, somewhere var has been defined 
    // calling setter function sets some variable 

    // could be used to get random int, then what would setter do?

    // setter and getter don't have to be linked
      // allows more flexibility than linked version 
    //mabe::ConfigEntry_Functions<char> funct_entry("name00, ")
    
    // mismatched get returns the value of a variable, set won't actually change variable 
    // replicate the linked config entry 
    // then also test in one direction and not the other 
  }
}


TEST_CASE("ConfigEntry_Var", "[config]"){
  {
    int v = 0;
    mabe::ConfigEntry_Var<int> var_entry_int("name00", v, "variable00", nullptr);

    // Test As() functions
    REQUIRE(var_entry_int.AsDouble() == 0.0);
    std::string s00 = var_entry_int.AsString();
    REQUIRE(s00.compare("0") == 0);

    // Test updating variable, ConfigEntry should not change
    v = 1;

    REQUIRE(var_entry_int.AsDouble() == 0.0);
    std::string s01 = var_entry_int.AsString();
    REQUIRE(s01.compare("0") == 0);

    // Test bool functions 
    REQUIRE(var_entry_int.IsTemporary() == false);
    REQUIRE(var_entry_int.IsBuiltIn() == false);
    REQUIRE(var_entry_int.IsNumeric() == true);
    REQUIRE(var_entry_int.IsBool() == false);
    REQUIRE(var_entry_int.IsInt() == true);
    REQUIRE(var_entry_int.IsDouble() == false);
    REQUIRE(var_entry_int.IsString() == false);

    REQUIRE(var_entry_int.IsLocal() == true);
    REQUIRE(var_entry_int.IsTemporary() == false);
    REQUIRE(var_entry_int.IsBuiltIn() == false);
    REQUIRE(var_entry_int.IsFunction() == false);
    REQUIRE(var_entry_int.IsScope() == false);
    REQUIRE(var_entry_int.IsError() == false);

    // Test getter functions
    std::string name00 = var_entry_int.GetName();
    REQUIRE(name00.compare("name00") == 0);
    std::string desc00 = var_entry_int.GetDesc();
    REQUIRE(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigScope> ptr = var_entry_int.GetScope();
    REQUIRE(ptr == nullptr);
    std::string type = var_entry_int.GetTypename();
    REQUIRE(type.compare("Value") == 0);

    // Test setter functions
    var_entry_int.SetName("name01");
    std::string name01 = var_entry_int.GetName();
    REQUIRE(name01.compare("name01") == 0);
    var_entry_int.SetDesc("desc01");
    std::string desc01 = var_entry_int.GetDesc();
    REQUIRE(desc01.compare("desc01") == 0);
    var_entry_int.SetTemporary();
    REQUIRE(var_entry_int.IsTemporary() == true);
    var_entry_int.SetBuiltIn();
    REQUIRE(var_entry_int.IsBuiltIn() == true);

    // Test setter functions, original variable should not change 
    var_entry_int.SetValue(2.0);
    REQUIRE(var_entry_int.AsDouble() == 2.0);
    REQUIRE(v == 1);
    var_entry_int.SetString("3");
    std::string s02 = var_entry_int.AsString();
    REQUIRE(s02.compare("3") == 0);
    REQUIRE(v == 1); 
  }
}

TEST_CASE("ConfigEntry_Var<std::string>", "[config]"){
  {
    std::string v = "0";
    mabe::ConfigEntry_Var<std::string> var_entry_str("name00", v, "variable00", nullptr);

    // Test As() functions
    REQUIRE(var_entry_str.AsDouble() == 0.0);
    std::string s00 = var_entry_str.AsString();
    REQUIRE(s00.compare("0") == 0);

    // Test updating variable, ConfigEntry should not change
    v = "1";

    REQUIRE(var_entry_str.AsDouble() == 0.0);
    std::string s01 = var_entry_str.AsString();
    REQUIRE(s01.compare("0") == 0);

    // Test bool functions 
    REQUIRE(var_entry_str.IsTemporary() == false);
    REQUIRE(var_entry_str.IsBuiltIn() == false);
    REQUIRE(var_entry_str.IsNumeric() == false);
    REQUIRE(var_entry_str.IsBool() == false);
    REQUIRE(var_entry_str.IsInt() == false);
    REQUIRE(var_entry_str.IsDouble() == false);
    REQUIRE(var_entry_str.IsString() == true);

    REQUIRE(var_entry_str.IsLocal() == true);
    REQUIRE(var_entry_str.IsTemporary() == false);
    REQUIRE(var_entry_str.IsBuiltIn() == false);
    REQUIRE(var_entry_str.IsFunction() == false);
    REQUIRE(var_entry_str.IsScope() == false);
    REQUIRE(var_entry_str.IsError() == false);

    // Test getter functions
    std::string name00 = var_entry_str.GetName();
    REQUIRE(name00.compare("name00") == 0);
    std::string desc00 = var_entry_str.GetDesc();
    REQUIRE(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigScope> ptr = var_entry_str.GetScope();
    REQUIRE(ptr == nullptr);
    std::string type = var_entry_str.GetTypename();
    REQUIRE(type.compare("String") == 0);

    // Test setter functions
    var_entry_str.SetName("name01");
    std::string name01 = var_entry_str.GetName();
    REQUIRE(name01.compare("name01") == 0);
    var_entry_str.SetDesc("desc01");
    std::string desc01 = var_entry_str.GetDesc();
    REQUIRE(desc01.compare("desc01") == 0);
    var_entry_str.SetTemporary();
    REQUIRE(var_entry_str.IsTemporary() == true);
    var_entry_str.SetBuiltIn();
    REQUIRE(var_entry_str.IsBuiltIn() == true);

    // Test setter functions, original variable should not change
    var_entry_str.SetValue(2.0);
    REQUIRE(var_entry_str.AsDouble() == 2.0);
    REQUIRE(v == "1");
    var_entry_str.SetString("3");
    std::string s02 = var_entry_str.AsString();
    REQUIRE(s02.compare("3") == 0);
    REQUIRE(v == "1");
  }
}