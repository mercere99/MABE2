/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
 *
 *  @file  VirtualCPUOrg_Inst_Nop.cpp
 *  @brief Test functionality of Nop instructions for VirtualCPUOrgs. 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "orgs/VirtualCPUOrg.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Nop.hpp"
#include "orgs/instructions/VirtualCPU_Inst_IO.hpp"

//  [ ] SetupConfig 
//  [ ] SetupModule
//  [ ] SetupFuncs
//  [ ] Generated actions

template<typename T>
T& GetConfiguredRef(
    mabe::MABE& control, 
    const std::string& type_name, 
    const std::string& var_name, 
    emplode::Symbol_Scope& scope){
  emplode::Symbol_Object& symbol_obj = 
      control.GetConfigScript().GetSymbolTable().MakeObjSymbol(type_name, var_name, scope);
  return *dynamic_cast<T*>(symbol_obj.GetObjectPtr().Raw());
}

TEST_CASE("VirtualCPUOrg_Inst_Nop_Main", "[orgs/instructions]"){
  // Initialize the instruction library, which only needs done once
  mabe::MABE control(0, nullptr);
  control.GetRandom().ResetSeed(100);
  control.AddPopulation("test_pop", 0);
  mabe::OrganismManager<mabe::VirtualCPUOrg> manager(control, "name", "desc");
  emplode::Symbol_Scope root_scope("root_scope", "desc", nullptr);
  // Add NopA, NopB, and NopC
  mabe::VirtualCPU_Inst_Nop& nop_inst_module = 
      GetConfiguredRef<mabe::VirtualCPU_Inst_Nop>(
        control, "VirtualCPU_Inst_Nop", "insts_nop", root_scope); 
  // Add IO (for required traits) 
  mabe::VirtualCPU_Inst_IO& io_inst_module = 
      GetConfiguredRef<mabe::VirtualCPU_Inst_IO>(
          control, "VirtualCPU_Inst_IO", "insts_io", root_scope); 
  mabe::VirtualCPUOrg tmp_org(manager);
  tmp_org.SharedData().inst_set_input_filename = "../inst_set_test.txt";
  control.GetTraitManager().Unlock();
  nop_inst_module.SetupModule();
  io_inst_module.SetupModule();
  tmp_org.SetupModule();
  control.GetTraitManager().Lock();
  control.GetTraitManager().Verify(true);
  std::cout << "InstLib size: " << tmp_org.GetInstLib().GetSize() << std::endl;
  emp::DataMap data_map = control.GetOrganismDataMap();
  control.GetTraitManager().RegisterAll(data_map);
  data_map.LockLayout();          
  { // Ensure nops by themselves don't actually change the organism
    control.GetRandom().ResetSeed(100);
    mabe::OrganismManager<mabe::VirtualCPUOrg>& manager = GetConfiguredRef<mabe::OrganismManager<mabe::VirtualCPUOrg>>(control, "VirtualCPUOrg", "org_manager_2", root_scope);
    mabe::VirtualCPUOrg org(manager);
    manager.AsScope().GetSymbol("point_mut_prob")->SetValue(0.01);
    manager.AsScope().GetSymbol("insertion_mut_prob")->SetValue(0);
    manager.AsScope().GetSymbol("deletion_mut_prob")->SetValue(0);
    org.SetupMutationDistribution();
    emp::DataMap data_map = control.GetOrganismDataMap();
    control.GetTraitManager().RegisterAll(data_map);
    data_map.LockLayout();          
    org.SetDataMap(data_map);
    org.Initialize(control.GetRandom());
    org.PushInst("NopA");
    org.PushInst("NopB");
    org.PushInst("NopC");
    org.PushInst("NopA");
    org.Reset();
    CHECK(10 == (org.regs[0] = 10));
    CHECK(11 == (org.regs[1] = 11));
    CHECK(12 == (org.regs[2] = 12));
    CHECK(0 == (org.inst_ptr = 0));
    org.ProcessStep();
    CHECK(org.regs[0] == 10);
    CHECK(org.regs[1] == 11);
    CHECK(org.regs[2] == 12);
    CHECK(org.inst_ptr == 1);
    org.ProcessStep();
    CHECK(org.regs[0] == 10);
    CHECK(org.regs[1] == 11);
    CHECK(org.regs[2] == 12);
    CHECK(org.inst_ptr == 2);
    org.ProcessStep();
    CHECK(org.regs[0] == 10);
    CHECK(org.regs[1] == 11);
    CHECK(org.regs[2] == 12);
    CHECK(org.inst_ptr == 3);

  }
}
