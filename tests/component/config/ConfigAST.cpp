/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigAST.cpp
 *  @brief Tests for ConfigAST with various types and edge cases 
 */

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

// how to test write() ?

TEST_CASE("ASTLeaf", "[config]"){
  {
    int v = 0;
    mabe::ConfigEntry_Linked entry00("name00", v, "variable00", nullptr);

    entry_ptr_t ptr00 = &entry00;
    mabe::ASTNode_Leaf leaf00(ptr00);

  // Test getter functions
  std::string str00 = leaf00.GetName();
  REQUIRE(str00.compare("name00") == 0);

  // REQUIRE(leaf00.GetEntry() == *ptr00); how to check if these are equal ?

  // Test boolean functions
  REQUIRE(leaf00.IsLeaf());

  // Test Process()
  REQUIRE(leaf00.Process() == ptr00);
  }
}

TEST_CASE("ASTNode_Block", "[config]"){
  {
    // mabe::ASTNode_Block block00("name00"); how should this be set up ?

    // Test getter functions
    std::string str00 = block00.GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(block00.GetNumChildren() == 0);
    REQUIRE(block00.GetChild(0) == nullptr);

    // Test boolean functions
    REQUIRE(block00.IsInternal());

    // Test adding children
    // mabe::ASTNode node00() should this be an internal node ?
    //node_ptr_t child00 = &node00;
    //block00.AddChild(child00);

    REQUIRE(block00.GetNumChildren == 1);
    // REQUIRE(block00.GetChild(0) == child00);

    entry_ptr_t Process();
    /*override {
      for (auto node : children) {
        entry_ptr_t out = node->Process();
        if (out && out->IsTemporary()) out.Delete();
      }
      return nullptr;
    }*/
  }
}

TEST_CASE("ASTNode_Math1", "[config]"){
  {
    mabe::ASTNode_Math1 math100("name00");

    // Test getters
    std::string str00 = math100.GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(math100.GetNumChildren() == 0);

    // Test boolean functiosn
    REQUIRE(math100.IsInternal());

    size_t GetNumChildren() const override { return children.size(); }
    node_ptr_t GetChild(size_t id) override { return children[id]; }

    void AddChild(node_ptr_t child) { children.push_back(child); }
  }
}
