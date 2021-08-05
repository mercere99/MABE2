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

using entry_ptr_t = emp::Ptr<mabe::ConfigEntry>;
using entry_vector_t = emp::vector<entry_ptr_t>;

using node_ptr_t = emp::Ptr<mabe::ASTNode>;
using node_vector_t = emp::vector<node_ptr_t>;


TEST_CASE("ASTEvents_Leaf", "[config]"){
  {
    // Setup for ASTNode_Event
    // Create action
    std::string v = "action00";
    mabe::ConfigEntry_Var<std::string> entry("action00", v, "desc00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> action00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry);

    // Create ConfigEvents object
    mabe::ConfigEvents events00;
    events00.AddEvent(action00, 0.0, 0.0, -1.0);


    // Test TriggerAll()
    events00.TriggerAll();
    // REQUIRE(function_called == true);
    // REQUIRE(action_result.compare("action00") == 0);
    // REQUIRE(children_processed == args00.size());


  }
}

TEST_CASE("ASTEvents_Event", "[config]"){
  {
    // Setup for ASTNode_Event
    // Create action
    std::string v = "action00";
    mabe::ConfigEntry_Var<std::string> entry("action00", v, "desc00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> action00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry);

    // Create vector of arguments
    node_vector_t args00;

    int v00 = 2;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    args00.push_back(leaf00);
    int v01 = 3;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    args00.push_back(leaf01);

    // Create setup function
    bool function_called = false;
    int children_processed = 0;
    std::string action_result;

    std::function<void(node_ptr_t, const entry_vector_t &)> setup = [&function_called, &children_processed, &action_result](node_ptr_t node, const entry_vector_t & vector) {
      function_called = true;
      for (emp::Ptr<mabe::ConfigEntry> child : vector) {
        children_processed++;
      }

      std::stringstream ss;
      node->Write(ss, "");
      action_result = ss.str();
    };

    // Create ASTNode_Event
    const mabe::ASTNode_Event event("event00", action00, args00, setup);
    emp::Ptr<mabe::ASTNode_Event> event00 = emp::NewPtr<mabe::ASTNode_Event>(event);
    REQUIRE(event00->GetNumChildren() == args00.size() + 1);

    // // Create ConfigEvents object
    mabe::ConfigEvents events00;
    events00.AddEvent(event00, 0.0, 0.0, -1.0); // bool AddEvent(emp::Ptr<ASTNode> action, double first=0.0, double repeat=0.0, double max=-1.0) {


    // Test TriggerAll()
    emp::assert_clear();
    events00.TriggerAll(); // seg fault here, setup is being called and completed
    REQUIRE(emp::assert_last_fail);
    // REQUIRE(function_called == true);
    // REQUIRE(action_result.compare("action00") == 0);
    // REQUIRE(children_processed == args00.size());

    /*
    // Create new ConfigEntry
    int v00 = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    // Create new ConfigASTNode
    emp::Ptr<mabe::ASTNode_Leaf> action00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);

    events00.AddEvent(action00, 0.0, 0.0, -1.0);
    */

  }
}