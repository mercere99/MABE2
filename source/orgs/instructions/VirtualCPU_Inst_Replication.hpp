/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
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

  /// A collection of replication instructions to be used by VirtualCPUOrgs 
  class VirtualCPU_Inst_Replication : public Module {
  public:
    using inst_func_t = VirtualCPUOrg::inst_func_t;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
    std::string org_pos_trait = "org_pos"; ///< Name of the trait storing organism's position
    std::string offspring_genome_trait = "offspring_genome"; ///< Name of the trait storing the genome of the offspring organism 
    bool include_h_alloc  = true;  ///< Config option indicating if inst. is used
    bool include_h_divide = true;  ///< Config option indicating if inst. is used
    bool include_h_copy   = true;  ///< Config option indicating if inst. is used
    bool include_h_search = true;  ///< Config option indicating if inst. is used
    bool include_repro    = false; ///< Config option indicating if inst. is used
    int h_alloc_id  = -1;  ///< ID of the h_alloc instruction
    int h_divide_id  = -1;  ///< ID of the h_divide instruction
    int h_copy_id  = -1;  ///< ID of the h_copy instruction
    int h_search_id  = -1;  ///< ID of the h_search instruction
    int repro_id  = -1;  ///< ID of the repro instruction

  public:
    VirtualCPU_Inst_Replication(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Replication",
                    const std::string & desc="Replication instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;}
    ~VirtualCPU_Inst_Replication() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
      LinkVar(org_pos_trait, "pos_trait", "Name of trait that holds organism's position");
      LinkVar(offspring_genome_trait, "offspring_genome_trait", 
          "Name of trait that holds the offspring organism's genome");
      LinkVar(include_h_alloc, "include_h_alloc", "Do we include the 'h_alloc' instruction?");
      LinkVar(include_h_divide, "include_h_divide", "Do we include the 'h_divide' instruction?");
      LinkVar(include_h_copy, "include_h_copy", "Do we include the 'h_copy' instruction?");
      LinkVar(include_h_search, "include_h_search", "Do we include the 'h_search' instruction?");
      LinkVar(include_repro, "include_repro", "Do we include the 'repro' instruction?");
      LinkVar(h_alloc_id, "h_alloc_id", "ID of the h_alloc instruction");
      LinkVar(h_divide_id, "h_divide_id", "ID of the h_divide instruction");
      LinkVar(h_copy_id, "h_copy_id", "ID of the h_copy instruction");
      LinkVar(h_search_id, "h_search_id", "ID of the h_search instruction");
      LinkVar(repro_id, "repro_id", "ID of the repro instruction");
    }

    /// When config is loaded, create traits and set up functions
    void SetupModule() override {
      AddRequiredTrait<OrgPosition>(org_pos_trait);
      AddRequiredTrait<VirtualCPUOrg::genome_t>(offspring_genome_trait);
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      if(include_h_alloc){ // Head allocate 
        inst_func_t func_h_alloc = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          hw.genome_working.resize(hw.genome.size() * 2, hw.GetDefaultInst());
          hw.regs[0] = hw.genome.size();
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
              "HAlloc", func_h_alloc);
        action.data.AddVar<int>("inst_id", h_alloc_id);
      }
      if(include_h_divide){ // Head divide 
        inst_func_t func_h_divide = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          if(hw.read_head >= hw.genome.size() && hw.copied_inst_id_vec.size() >= hw.genome_working.size() / 2){
            OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
            VirtualCPUOrg::genome_t& offspring_genome = hw.GetTrait<VirtualCPUOrg::genome_t>(
                "offspring_genome");
            offspring_genome = hw.genome_working;
            offspring_genome.resize(0,0);
            offspring_genome.resize(hw.genome_working.size() - hw.read_head, 
                hw.GetDefaultInst()); 
            std::copy(
                hw.genome_working.begin() + hw.read_head, 
                hw.genome_working.end(),
                offspring_genome.begin());
            hw.genome_working.resize(hw.read_head, hw.GetDefaultInst());
            hw.ResetHardware();
            hw.inst_ptr = hw.genome.size() - 1;
            control.Replicate(org_pos, *org_pos.PopPtr());
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "HDivide", func_h_divide);
        action.data.AddVar<int>("inst_id", h_divide_id);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
      if(include_h_copy){ // Head copy 
        inst_func_t func_h_copy = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
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
        action.data.AddVar<int>("inst_id", h_copy_id);
      }
      if(include_h_search){ // Head search 
        inst_func_t func_h_search = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t res = hw.FindNopSequence(hw.GetComplementNopSequence(inst.nop_vec), hw.inst_ptr);
          if(inst.nop_vec.size() == 0 || res == hw.inst_ptr){
            hw.regs[1] = 0;
            hw.regs[2] = 0;
            hw.SetFH(hw.inst_ptr + 1);
          }
          else{
            hw.regs[1] = (res - hw.inst_ptr) > 0 ? res - hw.inst_ptr : res + hw.genome_working.size() - res + hw.inst_ptr;
            hw.regs[2] = inst.nop_vec.size();
            hw.SetFH(res + inst.nop_vec.size() + 1);
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "HSearch", func_h_search);
        action.data.AddVar<int>("inst_id", h_search_id);
      }
      if(include_repro){ // Repro 
        inst_func_t func_repro = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          if(hw.inst_ptr > 0.75 * hw.genome_working.size() && 
              hw.num_insts_executed > 0.75 * hw.genome_working.size()){
            OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
            VirtualCPUOrg::genome_t& offspring_genome = hw.GetTrait<VirtualCPUOrg::genome_t>(
                "offspring_genome");
            offspring_genome = hw.genome_working;
            std::copy(
                hw.genome_working.begin(), 
                hw.genome_working.end(),
                offspring_genome.begin());
            hw.ResetHardware();
            hw.inst_ptr = hw.genome_working.size() - 1;
            control.Replicate(org_pos, *org_pos.PopPtr());
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Repro", func_repro);
        action.data.AddVar<int>("inst_id", repro_id);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Replication, "Replication instructions for VirtualCPUOrg");
}

#endif
