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
    std::string org_pos_trait = "org_pos";
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
      LinkVar(org_pos_trait, "pos_trait", "Name of trait that holds organism's position");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Head allocate 
      {
        func_h_alloc = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          //std::cout << "HAlloc!" << std::endl;
          //std::cout << "Working genome: " << hw.GetString() << std::endl;
          //std::cout << "IP: " << hw.inst_ptr;
          //std::cout << " RH: " << hw.read_head;
          //std::cout << " WH: " << hw.write_head;
          //std::cout << std::endl;
          hw.genome_working.resize(hw.genome.size() * 2, hw.genome_working[0]);
          hw.regs[0] = hw.genome.size();
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
              "HAlloc", func_h_alloc);
        action.data.AddVar<int>("inst_id", 22);
      }
      // Head divide 
      {
        func_h_divide = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          if(hw.read_head >= hw.genome.size() && hw.copied_inst_id_vec.size() >= hw.genome_working.size() / 2){
            OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
            VirtualCPUOrg::genome_t& offspring_genome = hw.GetTrait<VirtualCPUOrg::genome_t>(
                "offspring_genome");
            offspring_genome = hw.genome_working;
            offspring_genome.resize(0);
            offspring_genome.resize(hw.genome_working.size() - hw.read_head, hw.genome_working[0]); 
            //std::cout << hw.read_head << " " << hw.genome_working.size() << std::endl;
            std::copy(
                hw.genome_working.begin() + hw.read_head, 
                hw.genome_working.end(),
                offspring_genome.begin());
            hw.genome_working.resize(hw.read_head);
            hw.ResetHardware();
            hw.inst_ptr = hw.genome_working.size() - 1;
            control.Replicate(org_pos, *org_pos.PopPtr());
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "HDivide", func_h_divide);
        action.data.AddVar<int>("inst_id", 23);
      }
      // Head copy 
      {
        func_h_copy = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          //std::cout << "Copy!" << std::endl;
          //std::cout << "Working genome: " << hw.GetString() << std::endl;
          //std::cout << "IP: " << hw.inst_ptr;
          //std::cout << " RH: " << hw.read_head;
          //std::cout << " WH: " << hw.write_head;
          //std::cout << std::endl;
          hw.genome_working[hw.write_head] = hw.genome_working[hw.read_head];
          hw.copied_inst_id_vec.push_back(hw.genome_working[hw.write_head].id);
          hw.read_head++;
          while(hw.read_head >= hw.genome_working.size()) hw.read_head -= hw.genome_working.size();
          hw.write_head++;
          while(hw.write_head >= hw.genome_working.size()) hw.write_head -= hw.genome_working.size();
          // TODO: Mutation
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "HCopy", func_h_copy);
        action.data.AddVar<int>("inst_id", 21);
      }
      // Head search 
      {
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
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "HSearch", func_h_search);
        action.data.AddVar<int>("inst_id", 25);
      }
    }

    void SetupModule() override {
      AddRequiredTrait<OrgPosition>(org_pos_trait);
      AddRequiredTrait<VirtualCPUOrg::genome_t>("offspring_genome");
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Replication, "Replication instructions for VirtualCPUOrg");
}

#endif
