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
#include "config/ConfigEntry_Scope.hpp"
#include "config/ConfigAST.hpp"
#include "config/ConfigEntry_Function.hpp"

#include "emp/base/Ptr.hpp"

using entry_ptr_t = emp::Ptr<mabe::ConfigEntry>;
using entry_vector_t = emp::vector<entry_ptr_t>;

using node_ptr_t = emp::Ptr<mabe::ASTNode>;
using node_vector_t = emp::vector<node_ptr_t>;


TEST_CASE("ASTLeaf", "[config]"){
  {
    // Create ConifgEntry object
    int v = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v, "variable00", nullptr);
    entry_ptr_t ptr00 = &entry00;

    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf00_ptr = emp::NewPtr<mabe::ASTNode_Leaf>(ptr00);

    // Create ConfigEntry object with no name
    int v01 = 1;
    mabe::ConfigEntry_Linked<int> entry01("", v01, "variable01", nullptr);

    // Create ASTNode object
    entry_ptr_t ptr01 = &entry01;
    emp::Ptr<mabe::ASTNode_Leaf> leaf01_ptr = emp::NewPtr<mabe::ASTNode_Leaf>(ptr01);

    // Test getter functions
    std::string str00 = leaf00_ptr->GetName();
    CHECK(str00.compare("name00") == 0);
    CHECK(&leaf00_ptr->GetEntry() == ptr00.Raw());
    CHECK(leaf00_ptr->GetNumChildren() == 0);
    emp::assert_clear();
    leaf00_ptr->GetChild(0);
    CHECK(emp::assert_last_fail);

    // Test boolean functions
    CHECK(leaf00_ptr->IsLeaf());
    CHECK(leaf00_ptr->IsInternal() == false);

    // Test Process()
    CHECK(leaf00_ptr->Process() == ptr00);

    // Test Write()
    std::stringstream ss;
    leaf00_ptr->Write(ss, "");
    CHECK(ss.str().compare("name00") == 0);

    std::stringstream ss01;
    leaf01_ptr->Write(ss01, "");
    CHECK(ss01.str().compare("1") == 0);

    // Test Destructor()
    leaf00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00_ptr.id));
    leaf01_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01_ptr.id));
  }
}


TEST_CASE("ASTNode_Block", "[config]"){
  {
    // Create ASTNode object
    mabe::ConfigEntry_Scope fake_scope("fake_scope", "Fake scope for testing", nullptr);
    emp::Ptr<mabe::ASTNode_Block> block00_ptr = emp::NewPtr<mabe::ASTNode_Block>(fake_scope);

    // Test getter functions
    std::string str00 = block00_ptr->GetName();
    CHECK(str00.compare("") == 0);

    CHECK(block00_ptr->GetNumChildren() == 0);

    // Test boolean functions
    CHECK(block00_ptr->IsInternal());
    CHECK(block00_ptr->IsLeaf() == false);

    // Test adding children
    // Create ConfigEntry object
    int v00 = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    // Create ASTNode object
    node_ptr_t leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    block00_ptr->AddChild(leaf00);

    CHECK(block00_ptr->GetNumChildren() == 1);
    CHECK(block00_ptr->GetChild(0)->IsLeaf());
    CHECK(block00_ptr->GetChild(0)->Process() == leaf00->Process());

    // Test multiple children
    // Create ConfigEntry object
    int v01 = 1;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);
    // Create ASTNode object
    node_ptr_t leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    block00_ptr->AddChild(leaf01);

    CHECK(block00_ptr->GetNumChildren() == 2);
    CHECK(block00_ptr->GetChild(1)->IsLeaf());
    CHECK(block00_ptr->GetChild(1)->Process() == leaf01->Process());

    // Test Process()
    CHECK(block00_ptr->Process() == nullptr);

    // Test Write()
    std::stringstream ss;
    block00_ptr->Write(ss, "");
    CHECK(ss.str().compare("name00;\nname01;\n") == 0);

    // Test Destructor
    block00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
  }
}

// Uniary math function
double abs_value(double n) {
  if (n < 0) {
    n *= -1;
  }
  return n;
}
TEST_CASE("ASTNode_Math1", "[config]"){
  {
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Math1> math100_ptr = emp::NewPtr<mabe::ASTNode_Math1>("math00");

    // Test getters
    std::string str00 = math100_ptr->GetName();
    CHECK(str00.compare("math00") == 0);

    CHECK(math100_ptr->GetNumChildren() == 0);

    // Test adding children
    // Create Config Entry object
    int v00 = -1;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    math100_ptr->AddChild(leaf00);

    CHECK(math100_ptr->GetNumChildren() == 1);
    CHECK(math100_ptr->GetChild(0)->IsLeaf());
    CHECK(math100_ptr->GetChild(0)->Process() == leaf00->Process());

    // Test boolean functions
    CHECK(math100_ptr->IsInternal());
    CHECK(math100_ptr->IsLeaf() == false);

    // Test Process() before SetFun() is called
    CHECK_THROWS(math100_ptr->Process());

    // Test setters
    math100_ptr->SetFun(abs_value);

    // Test Process() with one child
    emp::assert_clear();
    entry_ptr_t result00 = math100_ptr->Process();
    CHECK(!emp::assert_last_fail);
    CHECK(result00->AsDouble() == 1.0);
    CHECK(result00->IsTemporary() == true);

    // Test Write()
    std::stringstream ss;
    math100_ptr->Write(ss, "");
    CHECK(ss.str().compare("math00name00") == 0);

    // Add multiple children
    // Create ConfigEntry object
    int v01 = -2;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    math100_ptr->AddChild(leaf01);

    // Test getters for multiple children
    CHECK(math100_ptr->GetNumChildren() == 2);
    CHECK(math100_ptr->GetChild(1)->IsLeaf());

    // Test Process with multiple children
    entry_ptr_t result01 = math100_ptr->Process();
    CHECK(emp::assert_last_fail);

    // Test Destructor()
    math100_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));

    // Delete additional pointers
    result00.Delete();
    result01.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(result00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(result01.id));
  }
}

// Binary math function
double add_fun(double n, double m) {
  return n + m;
}
TEST_CASE("ASTNode_Math2", "[config]"){
  {
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Math2> math200_ptr = emp::NewPtr<mabe::ASTNode_Math2>("math00");

    // Test getters
    std::string str00 = math200_ptr->GetName();
    CHECK(str00.compare("math00") == 0);

    CHECK(math200_ptr->GetNumChildren() == 0);

    // Test boolean functions
    CHECK(math200_ptr->IsInternal());
    CHECK(math200_ptr->IsLeaf() == false);

    // Test adding children
    // Create ConfigEntry object
    int v00 = 1;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    math200_ptr->AddChild(leaf00);

    CHECK(math200_ptr->GetNumChildren() == 1);
    CHECK(math200_ptr->GetChild(0)->IsLeaf());
    CHECK(math200_ptr->GetChild(0)->Process() == leaf00->Process());

    // Add second child
    // Create ConfigEntry object
    int v01 = 2;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    math200_ptr->AddChild(leaf01);

    CHECK(math200_ptr->GetNumChildren() == 2);
    CHECK(math200_ptr->GetChild(1)->IsLeaf());
    CHECK(math200_ptr->GetChild(1)->Process() == leaf01->Process());

    // Test Process() before SetFun() is called
    CHECK_THROWS(math200_ptr->Process());

    // Set function
    math200_ptr->SetFun(add_fun);

    // Test Process()
    emp::assert_clear();
    entry_ptr_t result00 = math200_ptr->Process();
    CHECK(!emp::assert_last_fail);
    CHECK(result00->AsDouble() == 3.0);
    CHECK(result00->IsTemporary() == true);

    // Test Write()
    std::stringstream ss;
    math200_ptr->Write(ss, "");
    CHECK(ss.str().compare("name00 math00 name01") == 0);

    // Add third child
    // Create ConfigEntry object
    int v02 = 2;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    math200_ptr->AddChild(leaf02);

    CHECK(math200_ptr->GetNumChildren() == 3);
    CHECK(math200_ptr->GetChild(2)->IsLeaf());
    CHECK(math200_ptr->GetChild(2)->Process() == leaf02->Process());

    // Test Process() with three children
    emp::assert_clear();
    entry_ptr_t result01 = math200_ptr->Process();
    CHECK(emp::assert_last_fail);

    // Test Destructor
    math200_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));

    // Delete additional pointers
    result00.Delete();
    result01.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(result00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(result01.id));

  }
}

TEST_CASE("ASTNode_Assign", "[config]"){
  {
    // Create ConfigEntry object
    std::string v00 = "variable";
    mabe::ConfigEntry_Linked<std::string> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> lhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    // Create ConfigEntry object
    int v01 = 1;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> rhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);

    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Assign> assign00_ptr = emp::NewPtr<mabe::ASTNode_Assign>(lhs, rhs);

    // Test getters
    std::string str00 = assign00_ptr->GetName();
    CHECK(str00.compare("") == 0);

    CHECK(assign00_ptr->GetNumChildren() == 2);

    // Test boolean functions
    CHECK(assign00_ptr->IsInternal());
    CHECK(assign00_ptr->IsLeaf() == false);

    // Test Process()
    emp::assert_clear();
    entry_ptr_t result00 = assign00_ptr->Process();
    CHECK(!emp::assert_last_fail);
    CHECK(result00->AsDouble() == rhs->Process());

    // Add third child
    // Create ConfigEntry object
    int v02 = 2;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    assign00_ptr->AddChild(leaf02);

    CHECK(assign00_ptr->GetNumChildren() == 3);
    CHECK(assign00_ptr->GetChild(2)->IsLeaf());
    CHECK(assign00_ptr->GetChild(2)->Process() == leaf02->Process());

    // Test Process() with too many children
    entry_ptr_t result01 = assign00_ptr->Process();
    CHECK(emp::assert_last_fail);

    // Test Write()
    std::stringstream ss;
    assign00_ptr->Write(ss, "");
    CHECK(ss.str().compare("name00 = name01") == 0);

    // Test Destructor
    assign00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(lhs.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(rhs.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));

  }
}

TEST_CASE("ASTNode_Call", "[config]"){
  {
    // Create setup function
    size_t children_processed = 0;
    bool function_called;

    std::function<double(const entry_vector_t&)> setup = [&children_processed, &function_called](const entry_vector_t& entries) {
      for (entry_ptr_t entry : entries) {
        children_processed++;
      }
      function_called = true;
      return 0;
    };

    // Create ConfigEntry_Function object
    mabe::ConfigEntry_Function entry_func("func00", "desc00", nullptr);
    entry_func.SetFunction(setup);
    node_ptr_t funcs00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry_func);

    // Create vector of arguments
    node_vector_t args00;
    // Create Config Entry objects
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
    emp::Ptr<mabe::ASTNode_Call> call00_ptr = emp::NewPtr<mabe::ASTNode_Call>(funcs00, args00);

    // Test getters
    std::string str00 = call00_ptr->GetName();
    CHECK(str00.compare("") == 0);

    CHECK(call00_ptr->GetNumChildren() == args00.size() + 1);

    // Test boolean functions
    CHECK(call00_ptr->IsInternal());
    CHECK(call00_ptr->IsLeaf() == false);

    // Test Process()
    entry_ptr_t result = call00_ptr->Process();
    CHECK(children_processed == args00.size());
    CHECK(function_called == true);
    CHECK(result->AsDouble() == 0);

    // Test Write()
    std::stringstream ss;
    call00_ptr->Write(ss, "");
    CHECK(ss.str().compare("func00(name00, name01, name02)") == 0);

    // Test Destructor
    call00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(funcs00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));

    // Delete additional pointers
    result.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(result.id));
  }
}

TEST_CASE("ASTNode_Event", "[config]"){
  {
    // Create ConfigEntry object
    std::string v = "action00";
    mabe::ConfigEntry_Var<std::string> entry("action00", v, "desc00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> action00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry);

    // Create vector of arguments
    node_vector_t args00;
    // Create ConfigEntry objects
    int v00 = 2;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    args00.push_back(leaf00);
    int v01 = 3;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    args00.push_back(leaf01);

    // Create setup function
    size_t children_processed = 0;
    std::string action_result;

    std::function<entry_ptr_t(node_ptr_t, const entry_vector_t &)> setup = [&children_processed, &action_result](node_ptr_t node, const entry_vector_t & vector) {
      std::stringstream ss;
      node->Write(ss, "");
      action_result = ss.str();

      for (emp::Ptr<mabe::ConfigEntry> child : vector) {
        children_processed++;
      }
      return nullptr;
    };

    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Event> event00_ptr = emp::NewPtr<mabe::ASTNode_Event>("event00", action00, args00, setup);

    // Test getters
    std::string str00 = event00_ptr->GetName();
    CHECK(str00.compare("event00") == 0);

    CHECK(event00_ptr->GetNumChildren() == args00.size() + 1);

    // Test boolean functions
    CHECK(event00_ptr->IsInternal());
    CHECK(event00_ptr->IsLeaf() == false);

    // Test Process()
    event00_ptr->Process();
    CHECK(action_result.compare("action00") == 0);
    CHECK(children_processed == args00.size());

    // Test Write()
    std::stringstream ss;
    event00_ptr->Write(ss, "");
    CHECK(ss.str().compare("@event00(name00, name01) action00") == 0);

    // Test Destructor
    event00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(action00.id));
  }
}
TEST_CASE("ASTNode_Block Full Process", "[config]"){
  {
    mabe::ConfigEntry_Scope fake_scope("fake_scope", "Fake scope for testing", nullptr);
    // Create ASTNode object
    emp::Ptr<mabe::ASTNode_Block> block00_ptr = emp::NewPtr<mabe::ASTNode_Block>(fake_scope);

    // Create setup function
    size_t children_processed = 0;
    bool function_called;

    std::function<double(const entry_vector_t&)> setup = [&children_processed, &function_called](const entry_vector_t& entries) {
      for (entry_ptr_t entry : entries) {
        children_processed++;
      }
      function_called = true;
      return 0;
    };

    // Create ConfigEntry_Function object
    mabe::ConfigEntry_Function entry_func("func00", "desc00", nullptr);
    entry_func.SetFunction(setup);
    node_ptr_t funcs00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry_func);

    // Create vector of arguments
    node_vector_t args00;
    // Create ConfigEntry objects
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
    emp::Ptr<mabe::ASTNode_Call> call00_ptr = emp::NewPtr<mabe::ASTNode_Call>(funcs00, args00);

    // Add ASTNode_Call object as leaf
    block00_ptr->AddChild(call00_ptr);

    // Test Process() on ASTNode_Block, should process ASTNode_Call aswell
    entry_ptr_t result00 = block00_ptr->Process();
    CHECK(result00 == nullptr);

    CHECK(children_processed == args00.size());
    CHECK(function_called == true);

    // Delete objects
    block00_ptr.Delete();
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(funcs00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    CHECK(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));
  }
}
