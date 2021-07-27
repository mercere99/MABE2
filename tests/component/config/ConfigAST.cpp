/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigAST.cpp
 *  @brief Tests for ConfigAST with various types and edge cases 
 */

#include <iostream>
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "config/ConfigEntry.hpp"
#include "config/ConfigScope.hpp"
#include "config/ConfigAST.hpp"

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
    //REQUIRE_THROWS(block00.GetChild(0));

    // Test boolean functions
    REQUIRE(block00.IsInternal());

    // Test adding children
    /*int v00 = 0;
    mabe::ConfigEntry_Linked entry00("name00", v00, "variable00", nullptr);

    entry_ptr_t ptr00 = &entry00;
    mabe::ASTNode_Leaf leaf00(ptr00);
    node_ptr_t child00 = &leaf00;
    block00.AddChild(child00);

    REQUIRE(block00.GetNumChildren() == 1);
    REQUIRE(&block00.GetChild(0) == child00.GetEntry().Raw());

    // Test multiple children
    int v01 = 1;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);

    entry_ptr_t ptr01 = &entry01;
    mabe::ASTNode_Leaf leaf01(ptr01);
    node_ptr_t child01 = &leaf01;
    block00.AddChild(child01);

    REQUIRE(block00.GetNumChildren() == 2);

    REQUIRE(block00.GetChild(0) == child01);
    

    // Test Process()
    REQUIRE(block00.Process() == nullptr);

    // Test Write()
    std::stringstream ss;
    block00.Write(ss, "");
    std::cout << "ss:" << ss.str() << "done";
    REQUIRE(ss.str().compare("") == 0);
    */
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
    mabe::ASTNode_Math1 math100("name00");

    // Test getters
    std::string str00 = math100.GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(math100.GetNumChildren() == 0);
    REQUIRE(math100.GetChild(0) == nullptr);

    // Test adding children
    int v00 = -1;
    mabe::ConfigEntry_Linked entry00("name00", v00, "variable00", nullptr);

    entry_ptr_t ptr00 = &entry00;
    mabe::ASTNode_Leaf leaf00(ptr00);
    node_ptr_t child00 = &leaf00;
    block00.AddChild(child00);

    REQUIRE(block00.GetNumChildren() == 1);
    REQUIRE(block00.GetChild(0) == child00);

    // Test boolean functions
    REQUIRE(math100.IsInternal());

    // Test setters
    math100.SetFun(abs_value);

    //void AddChild(node_ptr_t child) { children.push_back(child); }

    // for process, test w more than one child, require throw
  }
}

/*
TEST_CASE("ASTNode_Math2", "[config]"){
  {
    mabe::ASTNode_Math2 math200("name00");

    // Test getters
    std::string str00 = math200.GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(math200.GetNumChildren() == 0);

    // Test boolean functions
    REQUIRE(math200.IsInternal());

    size_t GetNumChildren() const override { return children.size(); }
    node_ptr_t GetChild(size_t id) override { return children[id]; }

    void AddChild(node_ptr_t child) { children.push_back(child); }
  }
}

TEST_CASE("ASTNode_Assign", "[config]"){
  {
    mabe::ASTNode_Assign assign00("name00");

    // Test getters
    std::string str00 = assign00.GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(assign00.GetNumChildren() == 0);

    // Test boolean functions
    REQUIRE(assign00.IsInternal());

    entry_ptr_t Process();
    override {
      emp_assert(children.size() == 2);
      entry_ptr_t lhs = children[0]->Process();  // Determine the left-hand-side value.
      entry_ptr_t rhs = children[1]->Process();  // Determine the right-hand-side value.
      // @CAO Should make sure that lhs is properly assignable.
      lhs->CopyValue(*rhs);
      if (rhs->IsTemporary()) rhs.Delete();
      return lhs;
    }

  }
}

TEST_CASE("ASTNode_Call", "[config]"){
  {
    //mabe::ASTNode_Call call00()
  }
}

TEST_CASE("ASTNode_Event", "[config]"){
  {
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
  }
}
*/
