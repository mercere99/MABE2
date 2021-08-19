/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigEvents.cpp
 *  @brief Tests for ConfigEvents with various types and edge cases
 */

#include <iostream>
#include <functional>

// CATCH
#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG
#include "catch.hpp"

// MABE
#include "config/ConfigAST.hpp"
#include "config/ConfigEvents.hpp"
#include "config/ConfigFunction.hpp"

using entry_ptr_t = emp::Ptr<mabe::ConfigEntry>;
using entry_vector_t = emp::vector<entry_ptr_t>;

using node_ptr_t = emp::Ptr<mabe::ASTNode>;
using node_vector_t = emp::vector<node_ptr_t>;

// test with call and assign
// test ranges to repeat every x updates
// first isn't neg
// timesteps manually throuhg update value
// if event starts at 12 and updates every 2 timestpes and you jump from 10 to 15 you should fire twice
TEST_CASE("ASTEvents_Leaf", "[config]"){
  {
    // Setup for ASTNode_Event
    // Create action
    std::string v = "action00";
    mabe::ConfigEntry_Var<std::string> entry("action00", v, "desc00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> action00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry);

    // Create ConfigEvents object
    mabe::ConfigEvents events00;

    // Test asserts in constructor
    // Negative value for first not allowed
    emp::assert_clear();
    events00.AddEvent(action00, -1.0, 0.0, -1.0);
    REQUIRE(emp::assert_last_fail);
    // Negative repeat not allowed
    emp::assert_clear();
    events00.AddEvent(action00, 0.0, -1.0, -1.0);
    REQUIRE(emp::assert_last_fail);

    // Add correctly formatted event
    emp::assert_clear();
    events00.AddEvent(action00, 0.0, 0.0, -1.0);
    REQUIRE(!emp::assert_last_fail);

    // Test TriggerAll()
    events00.TriggerAll();

    // Test Write()
    std::string command = "command";
    std::stringstream ss;
    events00.Write(command, ss);
    //REQUIRE(ss.str().compare() == 0);

  }
}
TEST_CASE("ASTEvents_Call", "[config]"){
  {
    // Create function
    int children_processed = 0;
    bool function_called;
    int times_called = 0;

    std::function<double(entry_vector_t)> setup = [&children_processed, &function_called, &times_called](entry_vector_t entries) {
      std::cout << "in function" << std::endl;
      children_processed = 0;
      for (entry_ptr_t entry : entries) {
        children_processed++;
      }
      function_called = true;
      times_called++;
      return 0;
    };

    // Create ConfigFunction
    mabe::ConfigFunction entry_func("func00", "desc00", nullptr);
    entry_func.SetFunction(setup);
    node_ptr_t funcs00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry_func);

    // Create vector of arguments
    node_vector_t args00;

    int v00 = 2;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    node_ptr_t leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    args00.push_back(leaf00);
    int v01 = 3;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    node_ptr_t leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    args00.push_back(leaf01);
    int v02 = 4;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);
    node_ptr_t leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    args00.push_back(leaf02);

    // Create ASTNode_Call
    mabe::ASTNode_Call call00(funcs00, args00);
    node_ptr_t ptr00 = emp::NewPtr<mabe::ASTNode_Call>(call00);

    // Create ConfigEvents object
    mabe::ConfigEvents events00;

    // Test asserts in constructor
    // Negative value for first not allowed
    emp::assert_clear();
    events00.AddEvent(ptr00, -1.0, 0.0, -1.0);
    REQUIRE(emp::assert_last_fail);
    // Negative repeat not allowed
    emp::assert_clear();
    events00.AddEvent(ptr00, 0.0, -1.0, -1.0);
    REQUIRE(emp::assert_last_fail);

    // Add correctly formatted event
    emp::assert_clear();
    events00.AddEvent(ptr00, 0.0, 0.0, -1.0);
    REQUIRE(!emp::assert_last_fail);

    // Test TriggerAll()
    events00.TriggerAll();
    REQUIRE(children_processed == args00.size());
    REQUIRE(function_called == true);
    REQUIRE(times_called == 1);

    // Add new event
    events00.AddEvent(ptr00, 3.0, 2.0, 4.0);

    // Test UpdateValue, new event should not be triggered
    function_called = false;
    events00.UpdateValue(2.0);
    REQUIRE(children_processed == 0);
    REQUIRE(function_called == false);
    REQUIRE(times_called == 1);
    // Test UpdateValue, new event should be triggered
    function_called = false;
    events00.UpdateValue(3.0);
    REQUIRE(children_processed == args00.size());
    REQUIRE(function_called == true);
    REQUIRE(times_called == 3);
    // Test UpdateValue, new event should not be triggered
    function_called = false;
    events00.UpdateValue(4.0);
    REQUIRE(children_processed == 0);
    REQUIRE(function_called == false);
    REQUIRE(times_called == 3);

    // Retest TriggerAll()
    function_called = false;
    events00.AddEvent(ptr00, 10.0, 0.0, -1.0);
    events00.AddEvent(ptr00, 15.0, 2.0, -1.0);
    events00.AddEvent(ptr00, 1.0, 0.0, -1.0);
    events00.TriggerAll();
    REQUIRE(children_processed == args00.size());
    REQUIRE(function_called == true);
    REQUIRE(times_called == 7);


    // Test Write() not working?
    const std::string command = "command";
    std::stringstream ss;
    events00.Write(command, ss);
    std::cout << "!!!" << ss.str() << std::endl;
    //REQUIRE(ss.str().compare() == 0)

  }
}
TEST_CASE("ASTEvents_Assign", "[config]"){
  {
    std::string v00 = "variable";
    mabe::ConfigEntry_Linked<std::string> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> lhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);

    int v01 = 1;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> rhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);

    mabe::ASTNode_Assign assign00(lhs, rhs);
    emp::Ptr ptr00 = emp::NewPtr<mabe::ASTNode_Assign>(assign00);

    // Create ConfigEvents object
    mabe::ConfigEvents events00;

    // Test asserts in constructor
    // Negative value for first not allowed
    emp::assert_clear();
    events00.AddEvent(ptr00, -1.0, 0.0, -1.0);
    REQUIRE(emp::assert_last_fail);
    // Negative repeat not allowed
    emp::assert_clear();
    events00.AddEvent(ptr00, 0.0, -1.0, -1.0);
    REQUIRE(emp::assert_last_fail);

    // Add correctly formatted event
    emp::assert_clear();
    events00.AddEvent(ptr00, 0.0, 0.0, -1.0);
    REQUIRE(!emp::assert_last_fail);

    // Test TriggerAll()
    events00.TriggerAll();
    REQUIRE(entry00.AsDouble() == entry01.AsDouble());

    // Update lhs value
    entry01.SetValue(2.0);

    // Add new event
    events00.AddEvent(ptr00, 3.0, 0.0, -1.0);

    // Test UpdateValue, new event should not be triggered
    events00.UpdateValue(2.0);
    REQUIRE_FALSE(entry00.AsDouble() == entry01.AsDouble());
    // Test UpdateValue, new event should be triggered
    events00.UpdateValue(3.0);
    REQUIRE(entry00.AsDouble() == entry01.AsDouble());

    // Update lhs value
    entry01.SetValue(3.0);

    // Retest TriggerAll()
    events00.AddEvent(ptr00, 1.0, 0.0, -1.0);
    events00.TriggerAll();
    REQUIRE(entry00.AsDouble() == entry01.AsDouble());

    // Test Write() not working?
    const std::string command = "command";
    std::stringstream ss;
    events00.Write(command, ss);
    std::cout << "!!!" << ss.str() << std::endl;
    //REQUIRE(ss.str().compare() == 0)

  }
}

// try with assign, test that copy went through
