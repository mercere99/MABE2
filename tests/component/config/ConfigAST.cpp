/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigAST.cpp
 *  @brief Tests for ConfigAST with various types and edge cases 
 */

#include <iostream>
#include <functional>

// CATCH
#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG
#include "catch.hpp"

// MABE
#include "config/ConfigEntry.hpp"
#include "config/ConfigScope.hpp"
#include "config/ConfigAST.hpp"
#include "config/ConfigFunction.hpp"



using entry_ptr_t = emp::Ptr<mabe::ConfigEntry>;
using entry_vector_t = emp::vector<entry_ptr_t>;

using node_ptr_t = emp::Ptr<mabe::ASTNode>;
using node_vector_t = emp::vector<node_ptr_t>;


TEST_CASE("ASTLeaf", "[config]"){
  {
    int v = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v, "variable00", nullptr);

    entry_ptr_t ptr00 = &entry00;
    mabe::ASTNode_Leaf leaf00(ptr00);

    // Test getter functions
    std::string str00 = leaf00.GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(&leaf00.GetEntry() == ptr00.Raw());

    // Test boolean functions
    REQUIRE(leaf00.IsLeaf());

    // Test Process()
    REQUIRE(leaf00.Process() == ptr00);

    // Test Write()
    std::stringstream ss;
    leaf00.Write(ss, "");
    REQUIRE(ss.str().compare("name00") == 0);

  }
}


TEST_CASE("ASTNode_Block", "[config]"){
  {
    mabe::ASTNode_Block block00;

    // Test getter functions
    std::string str00 = block00.GetName();
    REQUIRE(str00.compare("") == 0);

    REQUIRE(block00.GetNumChildren() == 0);

    // Test boolean functions
    REQUIRE(block00.IsInternal());

    // Test adding children
    int v00 = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    block00.AddChild(leaf00);

    REQUIRE(block00.GetNumChildren() == 1);
    REQUIRE(block00.GetChild(0)->IsLeaf());
    REQUIRE(block00.GetChild(0)->Process() == leaf00->Process());

    // Test multiple children
    int v01 = 1;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    block00.AddChild(leaf01);

    REQUIRE(block00.GetNumChildren() == 2);
    REQUIRE(block00.GetChild(1)->IsLeaf());
    REQUIRE(block00.GetChild(1)->Process() == leaf01->Process());

    // Test Process()
    REQUIRE(block00.Process() == nullptr);

    // Test Write()
    std::stringstream ss;
    block00.Write(ss, "");
    REQUIRE(ss.str().compare("name00;\nname01;\n") == 0);
  }
}

double abs_value(double n) {
  if (n < 0) {
    n *= -1;
  }
  return n;
}
TEST_CASE("ASTNode_Math1", "[config]"){
  {
    mabe::ASTNode_Math1 math100("math00");

    // Test getters
    std::string str00 = math100.GetName();
    REQUIRE(str00.compare("math00") == 0);

    REQUIRE(math100.GetNumChildren() == 0);

    // Test adding children
    int v00 = -1;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    math100.AddChild(leaf00);

    REQUIRE(math100.GetNumChildren() == 1);
    REQUIRE(math100.GetChild(0)->IsLeaf());
    REQUIRE(math100.GetChild(0)->Process() == leaf00->Process());

    // Test boolean functions
    REQUIRE(math100.IsInternal());

    // Test setters
    math100.SetFun(abs_value);
    
    // Test Process() with one child
    entry_ptr_t result00 = math100.Process();
    REQUIRE(!emp::assert_last_fail);
    REQUIRE(result00->AsDouble() == 1.0);

    // Test Write()
    std::stringstream ss;
    math100.Write(ss, "");
    REQUIRE(ss.str().compare("math00name00") == 0);

    // Add multiple children
    int v01 = -2;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    math100.AddChild(leaf01);

    // Test getters for multiple children
    REQUIRE(math100.GetNumChildren() == 2);
    REQUIRE(math100.GetChild(1)->IsLeaf());

    // Test Process with multiple children
    entry_ptr_t result01 = math100.Process();
    REQUIRE(emp::assert_last_fail);
  }
}

double add_fun(double n, double m) {
  return n + m;
}
TEST_CASE("ASTNode_Math2", "[config]"){
  {
    mabe::ASTNode_Math2 math200("math00");

    // Test getters
    std::string str00 = math200.GetName();
    REQUIRE(str00.compare("math00") == 0);

    REQUIRE(math200.GetNumChildren() == 0);

    // Test boolean functions
    REQUIRE(math200.IsInternal());

    // Test adding children
    int v00 = 1;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    math200.AddChild(leaf00);

    REQUIRE(math200.GetNumChildren() == 1);
    REQUIRE(math200.GetChild(0)->IsLeaf());
    REQUIRE(math200.GetChild(0)->Process() == leaf00->Process());

    // Add second child
    int v01 = 2;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    math200.AddChild(leaf01);

    REQUIRE(math200.GetNumChildren() == 2);
    REQUIRE(math200.GetChild(1)->IsLeaf());
    REQUIRE(math200.GetChild(1)->Process() == leaf01->Process());

    // Set function
    math200.SetFun(add_fun);

    // Test Process()
    emp::assert_clear();
    entry_ptr_t result01 = math200.Process();
    REQUIRE(!emp::assert_last_fail);
    REQUIRE(result01->AsDouble() == 3.0);

    // Test Write()
    std::stringstream ss;
    math200.Write(ss, "");
    REQUIRE(ss.str().compare("name00 math00 name01") == 0);

    // Add third child
    int v02 = 2;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    math200.AddChild(leaf02);

    REQUIRE(math200.GetNumChildren() == 3);
    REQUIRE(math200.GetChild(2)->IsLeaf());
    REQUIRE(math200.GetChild(2)->Process() == leaf02->Process());

    // Test Process() with three children
    emp::assert_clear();
    entry_ptr_t result00 = math200.Process();
    REQUIRE(emp::assert_last_fail);
  }
}

TEST_CASE("ASTNode_Assign", "[config]"){
  {
    std::string v00 = "variable";
    mabe::ConfigEntry_Linked<std::string> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> lhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);

    int v01 = 1;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> rhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);

    mabe::ASTNode_Assign assign00(lhs, rhs);

    // Test getters
    std::string str00 = assign00.GetName();
    REQUIRE(str00.compare("") == 0);

    REQUIRE(assign00.GetNumChildren() == 2);

    // Test boolean functions
    REQUIRE(assign00.IsInternal());

    // Test Process()
    emp::assert_clear(); // do this other places too!
    entry_ptr_t result00 = assign00.Process();
    REQUIRE(!emp::assert_last_fail);
    REQUIRE(result00->AsDouble() == 1.0);

    // Add third child
    int v02 = 2;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    assign00.AddChild(leaf02);

    REQUIRE(assign00.GetNumChildren() == 3);
    REQUIRE(assign00.GetChild(2)->IsLeaf());
    REQUIRE(assign00.GetChild(2)->Process() == leaf02->Process());

    // Test Process() with too many children
    entry_ptr_t result01 = assign00.Process();
    REQUIRE(emp::assert_last_fail);

    // Test Write()
    std::stringstream ss;
    assign00.Write(ss, "");
    REQUIRE(ss.str().compare("name00 = name01") == 0);

  }
}

TEST_CASE("ASTNode_Call", "[config]"){
  {
    // Create function
    std::function<double(node_vector_t)> sum = [](node_vector_t nodes) {
      double sum = 0.0;
      for (node_ptr_t node : nodes) { // node_ptr_t = emp::Ptr<mabe::ASTNode>;
        entry_ptr_t value = node->Process();
        double result = value->AsDouble();
        sum += result;
      }
      return sum;
    };

    // Create ConfigFunction
    mabe::ConfigFunction entry_func("func00", "desc00", nullptr);
    entry_func.SetFunction(sum);
    emp::Ptr<mabe::ASTNode_Leaf> funcs00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry_func);

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
    int v02 = 4;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    args00.push_back(leaf02);

    // Create ASTNode_Call
    mabe::ASTNode_Call call00(funcs00, args00);

    // Test getters
    std::string str00 = call00.GetName();
    REQUIRE(str00.compare("") == 0);

    REQUIRE(call00.GetNumChildren() == args00.size() + 1);

    // Test boolean functions
    REQUIRE(call00.IsInternal());

    // Test Process()
    //REQUIRE(call00.Process()->AsDouble() == 9.0);

  // Test Write()
    std::stringstream ss;
    call00.Write(ss, "");
    REQUIRE(ss.str().compare("func00(name00, name01, name02)") == 0);
  }
}

TEST_CASE("ASTNode_Event", "[config]"){
  {
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

    int children_processed = 0;
    std::string action_result;

    std::function<void(node_ptr_t, const entry_vector_t &)> setup = [&children_processed, &action_result](node_ptr_t node, const entry_vector_t & vector) {
      std::stringstream ss;
      node->Write(ss, "");
      action_result = ss.str();

      for (emp::Ptr<mabe::ConfigEntry> child : vector) {
        children_processed++;
      }
    };

    mabe::ASTNode_Event event00("event00", action00, args00, setup);

    // Test getters
    std::string str00 = event00.GetName();
    REQUIRE(str00.compare("event00") == 0);

    REQUIRE(event00.GetNumChildren() == args00.size() + 1);

    // Test boolean functions
    REQUIRE(event00.IsInternal());

    // Test Process()
    event00.Process();
    REQUIRE(action_result.compare("action00") == 0);
    REQUIRE(children_processed == args00.size());

    /*
    entry_ptr_t Process();
    override {
      emp_assert(children.size() >= 1);
      entry_vector_t arg_entries;
      for (size_t id = 1; id < children.size(); id++) {
        arg_entries.push_back( children[id]->Process() );
      }
      setup_event(children[0], arg_entries);
      return nullptr;
    }
    */

    // Test Write()
    std::stringstream ss;
    event00.Write(ss, "");
    REQUIRE(ss.str().compare("@event00(name00, name01) action00") == 0);
  }
}
