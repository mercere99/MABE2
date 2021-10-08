/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for ConfigEntry with various types and edge cases
 */

#include <climits>
#include <vector>
// CATCH
#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG
#include "catch.hpp"
// MABE
#include "config/ConfigEntry_Linked.hpp"
#include "config/ConfigEntry_Scope.hpp"

TEST_CASE("ConfigEntry_Linked_Int", "[config]"){
  {
    // Create ConfigEntry object
    int v = 0;
    mabe::ConfigEntry_Linked<int> linked_entry_int("name00", v, "variable00", nullptr);

    // Test As() functions
    CHECK(linked_entry_int.As<int>() == 0);
    CHECK(linked_entry_int.As<double>() == 0.0);
    CHECK(linked_entry_int.As<bool>() == false);
    CHECK(linked_entry_int.AsDouble() == linked_entry_int.As<int>());
    std::string s00 = linked_entry_int.AsString();
    CHECK(s00.compare("0") == 0);
    CHECK(s00.compare(linked_entry_int.As<std::string>()) == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> scope_ptr = linked_entry_int.AsScopePtr();
    CHECK(scope_ptr == nullptr);
    emp::Ptr ptr00 = linked_entry_int.As<emp::Ptr<mabe::ConfigEntry>>();
    CHECK(&linked_entry_int == ptr00.Raw());
    mabe::ConfigEntry& ref00 = linked_entry_int.As<mabe::ConfigEntry&>();
    CHECK(&ref00 == &linked_entry_int);

    emp::assert_clear();
    mabe::ConfigEntry_Scope& scope = linked_entry_int.As<mabe::ConfigEntry_Scope&>();
    CHECK(emp::assert_last_fail);

    // Test LookupEntry()
    CHECK(linked_entry_int.LookupEntry("").Raw() == &linked_entry_int);
    CHECK(linked_entry_int.LookupEntry("test").Raw() == nullptr);

    // Test Has()
    CHECK(linked_entry_int.Has("") == true);
    CHECK(linked_entry_int.Has("test") == false);

    // Test Write()
    std::stringstream ss;
    linked_entry_int.Write(ss, "");
    std::string assignment = "name00 = 0;";
    std::string expected = assignment +std::string(32 - assignment.length(), ' ') + "// variable00" + '\n';
    CHECK(ss.str().compare(expected) == 0);

    // Test updating variable, ConfigEntry should change
    v = 1;

    CHECK(linked_entry_int.AsDouble() == 1.0);
    std::string s01 = linked_entry_int.AsString();
    CHECK(s01.compare("1") == 0);

    // Retest As<bool>()
    CHECK(linked_entry_int.As<bool>() == true);
    v = 2;
    CHECK(linked_entry_int.As<bool>() == true);
    v = -2;
    CHECK(linked_entry_int.As<bool>() == true);
    v = 1; // reset original variable and ConfigEntry

    // Test bool functions
    CHECK(linked_entry_int.IsNumeric() == true);
    CHECK(linked_entry_int.IsBool() == false);
    CHECK(linked_entry_int.IsInt() == true);
    CHECK(linked_entry_int.IsDouble() == false);
    CHECK(linked_entry_int.IsString() == false);
    CHECK(linked_entry_int.IsLocal() == false);
    CHECK(linked_entry_int.IsTemporary() == false);
    CHECK(linked_entry_int.IsBuiltIn() == false);
    CHECK(linked_entry_int.IsFunction() == false);
    CHECK(linked_entry_int.IsScope() == false);
    CHECK(linked_entry_int.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_int.GetName();
    CHECK(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_int.GetDesc();
    CHECK(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> ptr01 = linked_entry_int.GetScope();
    CHECK(ptr01 == nullptr);
    std::string type = linked_entry_int.GetTypename();
    CHECK(type.compare("Value") == 0);

    // Test setter functions
    linked_entry_int.SetName("name01");
    std::string name01 = linked_entry_int.GetName();
    CHECK(name01.compare("name01") == 0);
    linked_entry_int.SetDesc("desc01");
    std::string desc01 = linked_entry_int.GetDesc();
    CHECK(desc01.compare("desc01") == 0);
    linked_entry_int.SetTemporary();
    CHECK(linked_entry_int.IsTemporary() == true);
    linked_entry_int.SetBuiltIn();
    CHECK(linked_entry_int.IsBuiltIn() == true);

    linked_entry_int.SetMin(1.0);
    linked_entry_int.SetValue(0.0);
    CHECK(linked_entry_int.AsDouble() < 2.0);
    linked_entry_int.SetMax(0.0);
    linked_entry_int.SetValue(1.0);
    CHECK(linked_entry_int.AsDouble() > 0.0);

    // Reset Min and Max
    linked_entry_int.SetMin(INT_MIN);
    // linked_entry_int.SetMax(INT_MAX); // bug: SetMax() sets min, so not being reset right now
    linked_entry_int.SetValue(0.0);

    // Test setter functions, should update original variable
    linked_entry_int.SetValue(2.0);
    CHECK(linked_entry_int.AsDouble() == 2.0);
    CHECK(v == 2.0);
    linked_entry_int.SetString("3");
    std::string s02 = linked_entry_int.AsString();
    CHECK(s02.compare("3") == 0);
    CHECK(v == 3);

    // Test Clone()
    emp::Ptr<mabe::ConfigEntry> clone_ptr = linked_entry_int.Clone();
    const std::string s03 = clone_ptr->GetName();
    CHECK(s03.compare(linked_entry_int.GetName()) == 0);
    const std::string s04 = clone_ptr->GetDesc();
    CHECK(s04.compare(linked_entry_int.GetDesc()) == 0);

    CHECK(clone_ptr->AsDouble() == linked_entry_int.AsDouble());

    // Test updating clone, should update original ConfigEntry and original variable
    clone_ptr->SetValue(4.0);
    CHECK(clone_ptr->AsDouble() == 4.0);
    CHECK(linked_entry_int.AsDouble() == 4.0);
    CHECK(v == 4);

    // Test CopyValue()
    int n = 5;
    mabe::ConfigEntry_Linked<int> linked_entry_int_01("name01", n, "variable01", nullptr);
    linked_entry_int.CopyValue(linked_entry_int_01);
    CHECK(linked_entry_int.AsDouble() == 5.0);

    // Test changing CopyValue(), should not change original ConfigEntry
    linked_entry_int_01.SetValue(6.0);
    CHECK(linked_entry_int.AsDouble() == 5.0);

    // Test Copy Constructor, must point to same variable
    mabe::ConfigEntry_Linked<int> linked_entry_int_copy = linked_entry_int;
    linked_entry_int_copy.SetValue(7.0);
    CHECK(v == 7);
    CHECK(linked_entry_int.AsDouble() == 7.0);

    // Test Call, should return ConfigEntry_Error pointer
    emp::vector<emp::Ptr<mabe::ConfigEntry>> args;
    emp::Ptr arg00 = linked_entry_int.As<emp::Ptr<mabe::ConfigEntry>>();
    args.push_back(arg00);
    emp::Ptr<mabe::ConfigEntry> call_result = linked_entry_int.Call(args);
    CHECK(call_result->IsError() == true);
    
    // Delete pointers
    call_result.Delete();
    clone_ptr.Delete();
  }
}

TEST_CASE("ConfigEntry_Linked_Double", "[config]"){
  {
    // Create ConfigEntry object
    double v = 0.0;
    mabe::ConfigEntry_Linked<double> linked_entry_double("name00", v, "variable00", nullptr);

    // Test As() functions
    CHECK(linked_entry_double.As<int>() == 0);
    CHECK(linked_entry_double.As<double>() == 0.0);
    CHECK(linked_entry_double.As<bool>() == false);
    CHECK(linked_entry_double.AsDouble() == linked_entry_double.As<int>());
    CHECK(linked_entry_double.AsDouble() == 0.0);
    std::string s00 = linked_entry_double.AsString();
    CHECK(s00.compare("0") == 0);
    CHECK(s00.compare(linked_entry_double.As<std::string>()) == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> scope_ptr = linked_entry_double.AsScopePtr();
    CHECK(scope_ptr == nullptr);
    emp::Ptr ptr00 = linked_entry_double.As<emp::Ptr<mabe::ConfigEntry>>();
    CHECK(&linked_entry_double == ptr00.Raw());
    mabe::ConfigEntry& ref00 = linked_entry_double.As<mabe::ConfigEntry&>();
    CHECK(&ref00 == &linked_entry_double);

    emp::assert_clear();
    mabe::ConfigEntry_Scope& scope = linked_entry_double.As<mabe::ConfigEntry_Scope&>();
    CHECK(emp::assert_last_fail);

    // Test LookupEntry()
    CHECK(linked_entry_double.LookupEntry("").Raw() == &linked_entry_double);
    CHECK(linked_entry_double.LookupEntry("test").Raw() == nullptr);

    // Test Has()
    CHECK(linked_entry_double.Has("") == true);
    CHECK(linked_entry_double.Has("test") == false);

    // Test Write()
    std::stringstream ss;
    linked_entry_double.Write(ss, "");
    std::string assignment = "name00 = 0;";
    std::string expected = assignment +std::string(32 - assignment.length(), ' ') + "// variable00" + '\n';
    CHECK(ss.str().compare(expected) == 0);

    // Test updating variable, ConfigEntry should change
    v = 1;

    CHECK(linked_entry_double.AsDouble() == 1.0);
    std::string s01 = linked_entry_double.AsString();
    CHECK(s01.compare("1") == 0);

    // Retest As<bool>()
    CHECK(linked_entry_double.As<bool>() == true);
    v = 2;
    CHECK(linked_entry_double.As<bool>() == true);
    v = -2;
    CHECK(linked_entry_double.As<bool>() == true);
    v = 1; // reset original variable and ConfigEntry

    // Test bool functions
    CHECK(linked_entry_double.IsNumeric() == true);
    CHECK(linked_entry_double.IsBool() == false);
    CHECK(linked_entry_double.IsInt() == false);
    CHECK(linked_entry_double.IsDouble() == true);
    CHECK(linked_entry_double.IsString() == false);
    CHECK(linked_entry_double.IsLocal() == false);
    CHECK(linked_entry_double.IsTemporary() == false);
    CHECK(linked_entry_double.IsBuiltIn() == false);
    CHECK(linked_entry_double.IsFunction() == false);
    CHECK(linked_entry_double.IsScope() == false);
    CHECK(linked_entry_double.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_double.GetName();
    CHECK(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_double.GetDesc();
    CHECK(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> ptr01 = linked_entry_double.GetScope();
    CHECK(ptr01 == nullptr);
    std::string type = linked_entry_double.GetTypename();
    CHECK(type.compare("Value") == 0);

    // Test setter functions
    linked_entry_double.SetName("name01");
    std::string name01 = linked_entry_double.GetName();
    CHECK(name01.compare("name01") == 0);
    linked_entry_double.SetDesc("desc01");
    std::string desc01 = linked_entry_double.GetDesc();
    CHECK(desc01.compare("desc01") == 0);
    linked_entry_double.SetTemporary();
    CHECK(linked_entry_double.IsTemporary() == true);
    linked_entry_double.SetBuiltIn();
    CHECK(linked_entry_double.IsBuiltIn() == true);
    linked_entry_double.SetMin(1.0);
    linked_entry_double.SetValue(0.0);
    CHECK(linked_entry_double.AsDouble() < 2.0);
    linked_entry_double.SetMax(0.0);
    linked_entry_double.SetValue(1.0);
    CHECK(linked_entry_double.AsDouble() > 0.0);

    // Reset Min and Max
    linked_entry_double.SetMin(INT_MIN);
    // linked_entry_double.SetMax(INT_MAX); // bug: SetMax() sets min, so not being reset right now
    linked_entry_double.SetValue(0.0);

    // Test setter functions, original variable should change
    linked_entry_double.SetValue(2.0);
    CHECK(linked_entry_double.AsDouble() == 2.0);
    linked_entry_double.SetString("3");
    std::string s02 = linked_entry_double.AsString();
    CHECK(s02.compare("3") == 0);

    // Test Clone()
    emp::Ptr<mabe::ConfigEntry> clone_ptr = linked_entry_double.Clone();
    const std::string s03 = clone_ptr->GetName();
    CHECK(s03.compare(linked_entry_double.GetName()) == 0);
    const std::string s04 = clone_ptr->GetDesc();
    CHECK(s04.compare(linked_entry_double.GetDesc()) == 0);
    CHECK(clone_ptr->AsDouble() == linked_entry_double.AsDouble());

    // Test updating clone, should update original ConfigEntry and original variable
    clone_ptr->SetValue(4.0);
    CHECK(clone_ptr->AsDouble() == 4.0);
    CHECK(linked_entry_double.AsDouble() == 4.0);
    CHECK(v == 4.0);

    // Test CopyValue()
    double n = 5.0;
    mabe::ConfigEntry_Linked<double> linked_entry_double_01("name01", n, "variable01", nullptr);
    linked_entry_double.CopyValue(linked_entry_double_01);
    CHECK(linked_entry_double.AsDouble() == 5.0);

    // Test changing CopyValue(), should not change original ConfigEntry
    linked_entry_double_01.SetValue(6.0);
    CHECK(linked_entry_double.AsDouble() == 5.0);

    // Test Copy Constructor, must point to same variable
    mabe::ConfigEntry_Linked<double> linked_entry_double_copy = linked_entry_double;
    linked_entry_double_copy.SetValue(7.0);
    CHECK(v == 7);
    CHECK(linked_entry_double.AsDouble() == 7.0);

    // Test Call(), should return ConfigEntry_Error pointer
    emp::vector<emp::Ptr<mabe::ConfigEntry>> args;
    emp::Ptr arg00 = linked_entry_double.As<emp::Ptr<mabe::ConfigEntry>>();
    args.push_back(arg00);
    emp::Ptr<mabe::ConfigEntry> call_result = linked_entry_double.Call(args);
    CHECK(call_result->IsError() == true);
    
    // Delete pointers
    call_result.Delete();
    clone_ptr.Delete();
  }
}

TEST_CASE("ConfigEntry_Linked_Bool", "[config]"){
  {
    // Create ConfigEntry object
    bool v = false;
    mabe::ConfigEntry_Linked<bool> linked_entry_bool("name00", v, "variable00", nullptr);

    // Test As() functions
    CHECK(linked_entry_bool.As<int>() == 0);
    CHECK(linked_entry_bool.As<double>() == 0.0);
    CHECK(linked_entry_bool.As<bool>() == false);
    CHECK(linked_entry_bool.AsDouble() == linked_entry_bool.As<int>());
    CHECK(linked_entry_bool.AsDouble() == 0.0);
    std::string s00 = linked_entry_bool.AsString();
    CHECK(s00.compare("0") == 0);
    CHECK(s00.compare(linked_entry_bool.As<std::string>()) == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> scope_ptr = linked_entry_bool.AsScopePtr();
    CHECK(scope_ptr == nullptr);
    emp::Ptr ptr00 = linked_entry_bool.As<emp::Ptr<mabe::ConfigEntry>>();
    CHECK(&linked_entry_bool == ptr00.Raw());
    mabe::ConfigEntry& ref00 = linked_entry_bool.As<mabe::ConfigEntry&>();
    CHECK(&ref00 == &linked_entry_bool);

    emp::assert_clear();
    mabe::ConfigEntry_Scope& scope = linked_entry_bool.As<mabe::ConfigEntry_Scope&>();
    CHECK(emp::assert_last_fail);

    // Test LookupEntry()
    CHECK(linked_entry_bool.LookupEntry("").Raw() == &linked_entry_bool);
    CHECK(linked_entry_bool.LookupEntry("test").Raw() == nullptr);

    // Test Has()
    CHECK(linked_entry_bool.Has("") == true);
    CHECK(linked_entry_bool.Has("test") == false);

    // Test Write()
    std::stringstream ss;
    linked_entry_bool.Write(ss, "");
    std::string assignment = "name00 = 0;";
    std::string expected = assignment +std::string(32 - assignment.length(), ' ') + "// variable00" + '\n';
    CHECK(ss.str().compare(expected) == 0);

    // Test updating variable, ConfigEntry should change
    v = true;

    CHECK(linked_entry_bool.AsDouble() == 1.0);
    std::string s01 = linked_entry_bool.AsString();
    CHECK(s01.compare("1") == 0);

    // Retest As<bool>()
    CHECK(linked_entry_bool.As<bool>() == true);
    v = 2;
    CHECK(linked_entry_bool.As<bool>() == true);
    v = -2;
    CHECK(linked_entry_bool.As<bool>() == true);
    v = true; // reset original variable and ConfigEntry

    // Test bool functions
    CHECK(linked_entry_bool.IsNumeric() == true);
    CHECK(linked_entry_bool.IsBool() == true);
    CHECK(linked_entry_bool.IsInt() == false);
    CHECK(linked_entry_bool.IsDouble() == false);
    CHECK(linked_entry_bool.IsString() == false);
    CHECK(linked_entry_bool.IsLocal() == false);
    CHECK(linked_entry_bool.IsTemporary() == false);
    CHECK(linked_entry_bool.IsBuiltIn() == false);
    CHECK(linked_entry_bool.IsFunction() == false);
    CHECK(linked_entry_bool.IsScope() == false);
    CHECK(linked_entry_bool.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_bool.GetName();
    CHECK(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_bool.GetDesc();
    CHECK(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> ptr01 = linked_entry_bool.GetScope();
    CHECK(ptr01 == nullptr);
    std::string type = linked_entry_bool.GetTypename();
    CHECK(type.compare("Value") == 0);

    // Test setter functions
    linked_entry_bool.SetName("name01");
    std::string name01 = linked_entry_bool.GetName();
    CHECK(name01.compare("name01") == 0);
    linked_entry_bool.SetDesc("desc01");
    std::string desc01 = linked_entry_bool.GetDesc();
    CHECK(desc01.compare("desc01") == 0);
    linked_entry_bool.SetTemporary();
    CHECK(linked_entry_bool.IsTemporary() == true);
    linked_entry_bool.SetBuiltIn();
    CHECK(linked_entry_bool.IsBuiltIn() == true);

    // Test setter functions, original variable should change
    linked_entry_bool.SetValue(0.0);
    CHECK(linked_entry_bool.AsDouble() == 0.0);
    CHECK(!v);
    linked_entry_bool.SetString("1");
    std::string s02 = linked_entry_bool.AsString();
    CHECK(s02.compare("1") == 0);
    CHECK(v);

    // Test Clone()
    emp::Ptr<mabe::ConfigEntry> clone_ptr = linked_entry_bool.Clone();
    const std::string s03 = clone_ptr->GetName();
    CHECK(s03.compare(linked_entry_bool.GetName()) == 0);
    const std::string s04 = clone_ptr->GetDesc();
    CHECK(s04.compare(linked_entry_bool.GetDesc()) == 0);

    CHECK(clone_ptr->AsDouble() == linked_entry_bool.AsDouble());

    // Test updating clone, should update original ConfigEntry and original variable
    clone_ptr->SetValue(0.0);
    CHECK(clone_ptr->AsDouble() == 0.0);
    CHECK(linked_entry_bool.AsDouble() == 0.0);
    CHECK(v == 0.0);

    // Test CopyValue()
    bool n = true;
    mabe::ConfigEntry_Linked<bool> linked_entry_bool_01("name01", n, "variable01", nullptr);
    linked_entry_bool.CopyValue(linked_entry_bool_01);
    CHECK(linked_entry_bool.AsDouble() == 1.0);

    // Test changing CopyValue(), should not change original ConfigEntry
    linked_entry_bool_01.SetValue(0.0);
    CHECK(linked_entry_bool.AsDouble() == 1.0);

    // Test Copy Constructor, must point to same variable
    mabe::ConfigEntry_Linked<bool> linked_entry_bool_copy = linked_entry_bool;
    linked_entry_bool.SetValue(0.0); // Reset original ConfigEntry
    linked_entry_bool_copy.SetValue(1.0);
    CHECK(v == 1);
    CHECK(linked_entry_bool.AsDouble() == 1.0);

    // Test Call(), should return ConfigEntry_Error pointer
    emp::vector<emp::Ptr<mabe::ConfigEntry>> args;
    emp::Ptr arg00 = linked_entry_bool.As<emp::Ptr<mabe::ConfigEntry>>();
    args.push_back(arg00);
    emp::Ptr<mabe::ConfigEntry> call_result = linked_entry_bool.Call(args);
    CHECK(call_result->IsError() == true);
    
    // Delete pointers
    call_result.Delete();
    clone_ptr.Delete();
  }
}

TEST_CASE("ConfigEntry_Linked<std::string>", "[config]"){
  {
    // Create ConfigEntry object
    std::string v = "0";
    mabe::ConfigEntry_Linked<std::string> linked_entry_str("name00", v, "variable00", nullptr);

    // Test As() functions
    CHECK(linked_entry_str.As<int>() == 0);
    CHECK(linked_entry_str.As<double>() == 0.0);
    CHECK(linked_entry_str.As<bool>() == false);
    CHECK(linked_entry_str.AsDouble() == linked_entry_str.As<int>());
    CHECK(linked_entry_str.AsDouble() == 0.0);
    std::string s00 = linked_entry_str.AsString();
    CHECK(s00.compare("0") == 0);
    CHECK(s00.compare(linked_entry_str.As<std::string>()) == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> scope_ptr = linked_entry_str.AsScopePtr();
    CHECK(scope_ptr == nullptr);
    emp::Ptr ptr00 = linked_entry_str.As<emp::Ptr<mabe::ConfigEntry>>();
    CHECK(&linked_entry_str == ptr00.Raw());
    mabe::ConfigEntry& ref00 = linked_entry_str.As<mabe::ConfigEntry&>();
    CHECK(&ref00 == &linked_entry_str);

    emp::assert_clear();
    mabe::ConfigEntry_Scope& scope = linked_entry_str.As<mabe::ConfigEntry_Scope&>();
    CHECK(emp::assert_last_fail);

    // Test LookupEntry()
    CHECK(linked_entry_str.LookupEntry("").Raw() == &linked_entry_str);
    CHECK(linked_entry_str.LookupEntry("test").Raw() == nullptr);

    // Test Has()
    CHECK(linked_entry_str.Has("") == true);
    CHECK(linked_entry_str.Has("test") == false);

    // Test Write()
    std::stringstream ss;
    linked_entry_str.Write(ss, "");
    std::string assignment = "name00 = \"0\";";
    std::string expected = assignment +std::string(32 - assignment.length(), ' ') + "// variable00" + '\n';
    CHECK(ss.str().compare(expected) == 0);

    // Test updating variable, ConfigEntry should change
    v = "1";

    CHECK(linked_entry_str.AsDouble() == 1.0);
    std::string s01 = linked_entry_str.AsString();
    CHECK(s01.compare("1") == 0);

    // Retest As<bool>()
    CHECK(linked_entry_str.As<bool>() == true);
    v = "2";
    CHECK(linked_entry_str.As<bool>() == true);
    v = "-2";
    CHECK(linked_entry_str.As<bool>() == true);
    v = "1"; // reset original variable and ConfigEntry

    // Test bool functions
    CHECK(linked_entry_str.IsNumeric() == false);
    CHECK(linked_entry_str.IsBool() == false);
    CHECK(linked_entry_str.IsInt() == false);
    CHECK(linked_entry_str.IsDouble() == false);
    CHECK(linked_entry_str.IsString() == true);
    CHECK(linked_entry_str.IsLocal() == false);
    CHECK(linked_entry_str.IsTemporary() == false);
    CHECK(linked_entry_str.IsBuiltIn() == false);
    CHECK(linked_entry_str.IsFunction() == false);
    CHECK(linked_entry_str.IsScope() == false);
    CHECK(linked_entry_str.IsError() == false);

    // Test getter functions
    std::string name00 = linked_entry_str.GetName();
    CHECK(name00.compare("name00") == 0);
    std::string desc00 = linked_entry_str.GetDesc();
    CHECK(desc00.compare("variable00") == 0);
    emp::Ptr<mabe::ConfigEntry_Scope> ptr01 = linked_entry_str.GetScope();
    CHECK(ptr01 == nullptr);
    std::string type = linked_entry_str.GetTypename();
    CHECK(type.compare("String") == 0);

    // Test setter functions
    linked_entry_str.SetName("name01");
    std::string name01 = linked_entry_str.GetName();
    CHECK(name01.compare("name01") == 0);
    linked_entry_str.SetDesc("desc01");
    std::string desc01 = linked_entry_str.GetDesc();
    CHECK(desc01.compare("desc01") == 0);
    linked_entry_str.SetTemporary();
    CHECK(linked_entry_str.IsTemporary() == true);
    linked_entry_str.SetBuiltIn();
    CHECK(linked_entry_str.IsBuiltIn() == true);
    linked_entry_str.SetMin(1.0);
    linked_entry_str.SetValue(0.0);
    CHECK(linked_entry_str.AsDouble() < 2.0);
    linked_entry_str.SetMax(0.0);
    linked_entry_str.SetValue(1.0);
    CHECK(linked_entry_str.AsDouble() > 0.0);

    // Reset Min and Max
    linked_entry_str.SetMin(INT_MIN);
    // linked_entry_str.SetMax(INT_MAX); // bug: SetMax() sets min, so not being reset right now
    linked_entry_str.SetValue(0.0);

    // Test setter functions, original variable should change
    linked_entry_str.SetValue(2.0);
    CHECK(linked_entry_str.AsDouble() == 2.0);
    CHECK(v == "2");
    linked_entry_str.SetValue(2.5);
    CHECK(linked_entry_str.AsDouble() == 2.5);
    CHECK(v == "2.5");
    linked_entry_str.SetString("3");
    std::string s02 = linked_entry_str.AsString();
    CHECK(s02.compare("3") == 0);
    CHECK(v == "3");

    // Test Clone()
    emp::Ptr<mabe::ConfigEntry> clone_ptr = linked_entry_str.Clone();
    const std::string s03 = clone_ptr->GetName();
    CHECK(s03.compare(linked_entry_str.GetName()) == 0);
    const std::string s04 = clone_ptr->GetDesc();
    CHECK(s04.compare(linked_entry_str.GetDesc()) == 0);

    CHECK(clone_ptr->AsDouble() == linked_entry_str.AsDouble());

    // Test updating clone, should update original ConfigEntry and original variable
    clone_ptr->SetValue(4.0);
    CHECK(clone_ptr->AsDouble() == 4.0);
    CHECK(linked_entry_str.AsDouble() == 4.0);
    CHECK(v == "4");

    // Test CopyValue()
    std::string n = "5";
    mabe::ConfigEntry_Linked<std::string> linked_entry_str_01("name01", n, "variable01", nullptr);
    linked_entry_str.CopyValue(linked_entry_str_01);
    CHECK(linked_entry_str.AsDouble() == 5.0);

    // Test changing CopyValue(), should not change original ConfigEntry
    linked_entry_str_01.SetValue(6.0);
    CHECK(linked_entry_str.AsDouble() == 5.0);

    // Test Copy Constructor, must point to same variable
    mabe::ConfigEntry_Linked<std::string> linked_entry_str_copy = linked_entry_str;
    linked_entry_str_copy.SetValue(7.0);
    CHECK(v.compare("7") == 0);
    CHECK(linked_entry_str.AsDouble() == 7.0);

    // Test Call, should return ConfigEntry_Error pointer
    emp::vector<emp::Ptr<mabe::ConfigEntry>> args;
    emp::Ptr arg00 = linked_entry_str.As<emp::Ptr<mabe::ConfigEntry>>();
    args.push_back(arg00);
    emp::Ptr<mabe::ConfigEntry> call_result = linked_entry_str.Call(args);
    CHECK(call_result->IsError() == true);
    
    // Delete pointers
    call_result.Delete();
    clone_ptr.Delete();
  }
}
