/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file EvalTaskOr.cpp 
 *  @brief Test file for the OR boolean logic task that is called via IO instruction
 */

// [X] Constructor
// [ ] SetupConfig
// [X] CheckTwoArg
// [X] SetupModule
// [X] SetupFunc
// [X] Fabricated instruction 
// [ ] OnPlacement

#define TDEBUG 1
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// Empirical tools
#include "emp/base/assert.hpp"
// MABE
#include "evaluate/callable/EvalTaskOr.hpp"

TEST_CASE("EvalTaskOr", "[evaluate/callable]"){
  using org_t = mabe::VirtualCPUOrg;
  using inst_t = mabe::VirtualCPUOrg::inst_t;
  using data_t = mabe::VirtualCPUOrg::data_t;

  mabe::MABE control = mabe::MABE(0, NULL);
  control.AddPopulation("fake pop"); 
  mabe::EvalTaskOr task(control);

  // Test evaluation on easy numbers
  CHECK(task.CheckTwoArg(1, 0, 1));
  CHECK(task.CheckTwoArg(3, 2, 1));
  CHECK(task.CheckTwoArg(3, 3, 1));
  CHECK(task.CheckTwoArg(5, 5, 1));
  CHECK(task.CheckTwoArg(7, 5, 3));

  // Create a more complicated testing environment
  mabe::OrganismManager<mabe::VirtualCPUOrg> org_manager(control, "test_manager");
  control.GetTraitManager().Unlock();
  org_manager.AddSharedTrait<emp::vector<data_t>>("input", "input vector", emp::vector<data_t>());
  org_manager.AddSharedTrait<emp::vector<data_t>>("output", "output vector", emp::vector<data_t>());
  org_manager.AddSharedTrait<double>("merit", "merit score", 0);
  task.AddOwnedTrait<bool>("or_performed", "Was or performed?", false);
  control.Setup_Traits();
  control.GetTraitManager().Lock();
  mabe::VirtualCPUOrg org(org_manager);
  control.GetTraitManager().RegisterAll(org.GetDataMap());
  mabe::VirtualCPUOrg::inst_t inst(0,0);
  
  // Setup and fetch the new function
  task.SetupFunc();
  mabe::ActionMap& action_map = control.GetActionMap(0);
  std::unordered_map<std::string, mabe::Action>& typed_action_map =
    action_map.GetFuncs<void, mabe::VirtualCPUOrg&, const mabe::VirtualCPUOrg::inst_t&>();
  CHECK(typed_action_map.size() == 1);
  CHECK(typed_action_map.begin()->first == "IO");
  mabe::Action& action = typed_action_map.begin()->second;
  CHECK(action.function_vec.size() == 1);
  
  // Load some example numbers into the organism
  emp::vector<data_t>& input_vec = org.GetTrait<emp::vector<data_t>>("input");
  emp::vector<data_t>& output_vec = org.GetTrait<emp::vector<data_t>>("output");
  input_vec.push_back(127);
  input_vec.push_back(35);
  input_vec.push_back(12);
  // Incorrect answer -> no reward
  output_vec.push_back(1);
  action.function_vec[0].Call<void, mabe::VirtualCPUOrg&, const inst_t&>(org, inst);
  CHECK(org.GetTrait<double>("merit") == 0);
  CHECK(!org.GetTrait<bool>("or_performed"));
  // Correct answer -> reward
  output_vec.push_back(127); // 127 OR 12 = 127
  action.function_vec[0].Call<void, mabe::VirtualCPUOrg&, const inst_t&>(org, inst);
  CHECK(org.GetTrait<double>("merit") == 1);
  CHECK(org.GetTrait<bool>("or_performed"));
  // Repeated answer -> no reward
  output_vec.push_back(35 | 12);
  action.function_vec[0].Call<void, mabe::VirtualCPUOrg&, const inst_t&>(org, inst);
  CHECK(org.GetTrait<double>("merit") == 1);
  CHECK(org.GetTrait<bool>("or_performed"));
}
