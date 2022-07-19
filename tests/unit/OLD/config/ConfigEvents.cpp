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
#include "config/ConfigEvents.hpp"
#include "config/ConfigEntry_Function.hpp"

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
    emp::Ptr<mabe::ConfigEvents> events00_ptr = emp::NewPtr<mabe::ConfigEvents>();

    // Test asserts in constructor
    // Negative value for first not allowed
    emp::assert_clear();
    events00_ptr->AddEvent(action00, -1.0, 0.0, -1.0);
    CHECK(emp::assert_last_fail);
    // Negative repeat not allowed
    emp::assert_clear();
    events00_ptr->AddEvent(action00, 0.0, -1.0, -1.0);
    CHECK(emp::assert_last_fail);

    // Add correctly formatted event
    emp::assert_clear();
    events00_ptr->AddEvent(action00, 0.0, 0.0, -1.0);
    CHECK(!emp::assert_last_fail);

    // Test TriggerAll()
    events00_ptr->TriggerAll();

    // Test Write() with no repeat
    events00_ptr->AddEvent(action00, 0.0, 0.0, -1.0);
    std::string command = "command";
    std::stringstream ss;
    events00_ptr->Write(command, ss);
    CHECK(ss.str().compare("@command(0) action00;\n") == 0);
    // Test Write() with repeat
    events00_ptr->AddEvent(action00, 1.0, 2.0, -1.0);
    std::stringstream ss01;
    events00_ptr->Write(command, ss01);
    CHECK(ss01.str().compare("@command(0) action00;\n@command(1, 2) action00;\n") == 0);

    // Test Destructor
    events00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(events00_ptr.id));
    action00.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(action00.id));


  }
}
TEST_CASE("ASTEvents_Call", "[config]"){
  {
    // Create function
    size_t children_processed = 0;
    bool function_called;
    int times_called = 0;

    std::function<double(const entry_vector_t&)> setup = [&children_processed, &function_called, &times_called](const entry_vector_t& entries) {
      for (entry_ptr_t entry : entries) {
        children_processed++;
      }
      function_called = true;
      times_called += 1;
      return 0;
    };

    // Create ConfigEntry_Function
    mabe::ConfigEntry_Function entry_func("func00", "desc00", nullptr);
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
    node_ptr_t ptr00 = emp::NewPtr<mabe::ASTNode_Call>(funcs00, args00);

    // Create ConfigEvents object
    emp::Ptr<mabe::ConfigEvents> events00_ptr = emp::NewPtr<mabe::ConfigEvents>();

    // Test asserts in constructor
    // Negative value for first not allowed
    emp::assert_clear();
    events00_ptr->AddEvent(ptr00, -1.0, 0.0, -1.0);
    CHECK(emp::assert_last_fail);
    // Negative repeat not allowed
    emp::assert_clear();
    events00_ptr->AddEvent(ptr00, 0.0, -1.0, -1.0);
    CHECK(emp::assert_last_fail);

    // Add correctly formatted event
    emp::assert_clear();
    events00_ptr->AddEvent(ptr00, 0.0, 0.0, -1.0);
    CHECK(!emp::assert_last_fail);

    // Test TriggerAll() (should have been called twice)
    children_processed = 0;
    function_called = false;
    times_called = 0;
    events00_ptr->TriggerAll();
    CHECK(children_processed == 2 * args00.size());
    CHECK(function_called == true);
    CHECK(times_called == 2);

    // Add new event
    events00_ptr->AddEvent(ptr00, 3.0, 2.0, 4.0);

    // Test UpdateValue, new event should not be triggered
    children_processed = 0;
    function_called = false;
    times_called = 0;
    events00_ptr->UpdateValue(2.0);
    CHECK(children_processed == 0);
    CHECK(function_called == false);
    CHECK(times_called == 0);
    // Test UpdateValue, new event should be triggered
    children_processed = 0;
    function_called = false;
    times_called = 0;
    events00_ptr->UpdateValue(3.0);
    CHECK(children_processed == args00.size());
    CHECK(function_called == true);
    CHECK(times_called == 1);
    // Test UpdateValue, new event should not be triggered
    children_processed = 0;
    function_called = false;
    times_called = 0;
    events00_ptr->UpdateValue(4.0);
    CHECK(children_processed == 0);
    CHECK(function_called == false);
    CHECK(times_called == 0);

    // Retest TriggerAll()
    children_processed = 0;
    function_called = false;
    times_called = 0;
    events00_ptr->AddEvent(ptr00, 10.0, 0.0, -1.0);
    events00_ptr->AddEvent(ptr00, 15.0, 2.0, -1.0);
    events00_ptr->AddEvent(ptr00, 1.0, 0.0, -1.0);
    events00_ptr->TriggerAll();
    CHECK(children_processed == 2 * args00.size());
    CHECK(function_called == true);
    CHECK(times_called == 2);

    // Test Write()
    events00_ptr->AddEvent(ptr00, 0.0, 0.0, -1.0);
    const std::string command = "command";
    std::stringstream ss;
    events00_ptr->Write(command, ss);
    // CHECK(ss.str().compare("@command(0) func00(name00, name01, name02);\n") == 0);

    // Test Destructors
    events00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(events00_ptr.id));

    // Delete ASTNode pointer
    ptr00.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(ptr00.id));
  }
}
/*
TEST_CASE("ASTEvents_Assign", "[config]"){
  {
    // Create ConfigEntry objects for lhs and rhs of assignment
    std::string v00 = "variable";
    mabe::ConfigEntry_Linked<std::string> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> lhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);

    int v01 = 1;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> rhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);

    // Create ASTNode object
    mabe::ASTNode_Assign assign00(lhs, rhs);
    emp::Ptr ptr00 = emp::NewPtr<mabe::ASTNode_Assign>(assign00);

    // Create ConfigEvents object
    emp::Ptr<mabe::ConfigEvents> events00_ptr = emp::NewPtr<mabe::ConfigEvents>();

    // Test asserts in constructor
    // Negative value for first not allowed
    emp::assert_clear();
    events00_ptr->AddEvent(ptr00, -1.0, 0.0, -1.0);
    CHECK(emp::assert_last_fail);
    // Negative repeat not allowed
    emp::assert_clear();
    events00_ptr->AddEvent(ptr00, 0.0, -1.0, -1.0);
    CHECK(emp::assert_last_fail);

    // Add correctly formatted event
    emp::assert_clear();
    events00_ptr->AddEvent(ptr00, 0.0, 0.0, -1.0);
    CHECK(!emp::assert_last_fail);

    // Test TriggerAll()
    events00_ptr->TriggerAll();
    CHECK(entry00.AsDouble() == entry01.AsDouble());

    // Update lhs value
    entry01.SetValue(2.0);

    // Add new event
    events00_ptr->AddEvent(ptr00, 3.0, 0.0, -1.0);

    // Test UpdateValue, new event should not be triggered
    events00_ptr->UpdateValue(2.0);
    CHECK_FALSE(entry00.AsDouble() == entry01.AsDouble());
    // Test UpdateValue, new event should be triggered
    events00_ptr->UpdateValue(3.0);
    CHECK(entry00.AsDouble() == entry01.AsDouble());

    // Update rhs value
    entry01.SetValue(3.0);

    // Retest TriggerAll(), new event should not be added because first < cur_value !
    events00_ptr->AddEvent(ptr00, 0.0, 0.0, -1.0);
    events00_ptr->TriggerAll();
    CHECK(entry00.AsDouble() != entry01.AsDouble());
    // Retest TriggerAll(), new event should be added
    events00_ptr->AddEvent(ptr00, 3.0, 0.0, -1.0);
    events00_ptr->TriggerAll();
    CHECK(entry00.AsDouble() == entry01.AsDouble());

    // Test Write()
    events00_ptr->UpdateValue(0.0);
    events00_ptr->AddEvent(ptr00, 0.0, 0.0, -1.0);
    const std::string command = "command";
    std::stringstream ss;
    events00_ptr->Write(command, ss);
    CHECK(ss.str().compare("@command(0) name00 = name01;\n") == 0);

    // Test Destructor
    events00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(events00_ptr.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(ptr00.id));

    // Delete additional pointers
    //ptr00.Delete();


  }
}

// try with assign, test that copy went through
*/
