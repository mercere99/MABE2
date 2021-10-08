/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Replication.hpp
 *  @brief Provides replication instructions to a population of VirtualCPUOrgs.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_REPLICATION_H
#define MABE_VIRTUAL_CPU_INST_REPLICATION_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_Replication : public Module {
  private:
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_h_alloc;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_h_divide;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_h_copy;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_h_search;

  public:
    VirtualCPU_Inst_Replication(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Replication",
                    const std::string & desc="Replication instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Replication() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Head allocate 
      func_h_alloc = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
        hw.genome_working.resize(hw.genome.size() * 2);
        hw.regs[0] = hw.genome.size();
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("HAlloc", func_h_alloc);
      // Head divide 
      func_h_divide = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
        if(hw.read_head >= hw.genome.size()){
          hw.genome_working.resize(hw.read_head);
          hw.ResetHardware();
          hw.inst_ptr = hw.genome.size() - 1;
        }
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("HDivide", 
          func_h_divide);
      // Head copy 
      func_h_copy = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
        hw.genome_working[hw.write_head] = hw.genome_working[hw.read_head];
        hw.copied_inst_id_vec.push_back(hw.genome_working[hw.write_head].id);
        hw.read_head++;
        while(hw.read_head >= hw.genome_working.size()) hw.read_head -= hw.genome_working.size();
        hw.write_head++;
        while(hw.write_head >= hw.genome_working.size()) hw.write_head -= hw.genome_working.size();
        // TODO: Mutation
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("HCopy", func_h_copy);
      // Head search 
      func_h_search = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        int res = hw.FindComplementLabel(inst.nop_vec, hw.inst_ptr);
        if(res == -1){ // Fail
          hw.regs[1] = 0;
          hw.regs[2] = 0;
          hw.flow_head = hw.inst_ptr + 1;
        }
        else{
          hw.regs[1] = res;
          hw.regs[2] = inst.nop_vec.size();
          hw.flow_head = hw.inst_ptr + res + inst.nop_vec.size();
          while(hw.flow_head >= hw.genome_working.size()) hw.flow_head -= hw.genome_working.size();
        }
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("HSearch", 
          func_h_search);
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Replication, "Replication instructions for VirtualCPUOrg");
}

#endif
