/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPU_Inst_Replication.hpp
 *  @brief Provides replication instructions to a population of VirtualCPUOrgs.
 *
 *  TODO: 
 *      - HCopy (and other instructions) should be able to add mutations to this genome
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
    using org_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;
    using this_t = VirtualCPU_Inst_Replication;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
    std::string org_pos_trait = "org_pos"; ///< Name of the trait storing organism's position
    std::string offspring_genome_trait = "offspring_genome"; ///< Name of the trait storing the genome of the offspring organism 
    std::string reset_self_trait = "reset_self"; ///< Name of the trait storing if org needs reset 
    double req_frac_inst_executed = 0.5;  /**< Config option indicating the fraction of 
                                            an organism's genome that must have been executed 
                                            for org to reproduce **/
    int req_count_inst_executed = -1;     /**< Config option indicating the number of 
                                            instructions an organism must have executed in 
                                            order to reproduce **/
    double req_frac_inst_copied = 0.5;  /**< Config option indicating the fraction of 
                                             an organism's genome that must have been copied 
                                             for org to reproduce **/

  public:
    VirtualCPU_Inst_Replication(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Replication",
                    const std::string & desc="Replication instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;}
    ~VirtualCPU_Inst_Replication() { }

    void Inst_HAlloc(org_t& hw, const org_t::inst_t& /*inst*/){
      // Only expand once
      if(hw.genome_working.size() == hw.genome.size()){
        hw.genome_working.resize(hw.genome.size() * 2, hw.GetDefaultInst());
        hw.regs[0] = hw.genome.size();
      }
    }
    void Inst_HDivide(org_t& hw, const org_t::inst_t& /*inst*/){
      // First make sure that: 
        // If required inst count isn't negative, that many insts have been executed 
        // Else, the correct fraction of the genome has been executed
        // Regardless, make sure copy limit has been met
      const bool can_repro_exec_count = 
          (req_count_inst_executed >= 0 
            && hw.num_insts_executed >= (size_t)req_count_inst_executed);
      const bool can_repro_exec_frac = (req_count_inst_executed < 0 
          && hw.num_insts_executed >= req_frac_inst_executed * hw.GetGenomeSize());
      const bool can_repro_copy_frac =  
          hw.GetNumInstsCopied() >= req_frac_inst_copied * hw.GetGenomeSize();
      if(can_repro_copy_frac && (can_repro_exec_count || can_repro_exec_frac)){
        // Make sure we've had an HAlloc
        if(hw.GetGenomeSize() == hw.GetWorkingGenomeSize()){
          return;
        }
        OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
        // Store the soon-to-be offspring's genome
        org_t::genome_t& offspring_genome = hw.GetTrait<org_t::genome_t>(
            offspring_genome_trait);
        offspring_genome.resize(hw.genome_working.size() - hw.read_head,
            hw.GetDefaultInst());
        std::copy(
            hw.genome_working.begin() + hw.read_head,
            hw.genome_working.end(),
            offspring_genome.begin());
        hw.genome_working.resize(hw.read_head, hw.GetDefaultInst());
        // Replicate
        control.Replicate(org_pos, *org_pos.PopPtr());
        // Reset the parent
        hw.Reset();
        // Set to end so completion of this inst moves it 0 
        hw.inst_ptr = hw.genome_working.size() - 1; 
      }
    }
    void Inst_HCopy(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.genome_working[hw.write_head] = hw.genome_working[hw.read_head];
      hw.copied_inst_id_vec.push_back(hw.genome_working[hw.write_head].id);
      hw.genome_working[hw.read_head].has_been_copied = true;
      hw.AdvanceRH();
      hw.AdvanceWH();
      // TODO: Mutation
    }
    void Inst_HSearch(org_t& hw, const org_t::inst_t& inst){
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
    }
    void Inst_Repro(org_t& hw, const org_t::inst_t& /* inst */){
      // First make sure that: 
        // If required inst count isn't negative, that many insts have been executed 
        // Else, the correct fraction of the genome has been executed
      if( (req_count_inst_executed >= 0 
            && hw.num_insts_executed >= (size_t)req_count_inst_executed)
          || (req_count_inst_executed < 0 
            && hw.num_insts_executed >= req_frac_inst_executed * hw.genome.size())){
        OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
        // Store the soon-to-be offspring's genome
        org_t::genome_t& offspring_genome = hw.GetTrait<org_t::genome_t>(
            offspring_genome_trait);
        offspring_genome.resize(hw.genome.size(), hw.GetDefaultInst());
        std::copy(
            hw.genome.begin(),
            hw.genome.end(),
            offspring_genome.begin());
        // Replicate 
        control.Replicate(org_pos, *org_pos.PopPtr());
        // Reset the parent
        hw.Reset();
        // Set to end so completion of this inst moves it 0 
        hw.inst_ptr = hw.genome_working.size() - 1; 
      }
    }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
      LinkVar(req_frac_inst_executed, "req_frac_inst_executed", 
              "The organism must have executed at least this fraction of their genome to"
                " reproduce. Otherwise reproduction instructions do nothing. Overruled by"
                " `req_count_inst_executed`");
      LinkVar(req_count_inst_executed, "req_count_inst_executed", 
              "Minimum number of instructions that the organism must execute before its"
                " allowed to reproduce. Otherwise reproduction instructions do nothing. "
                " Takes priority over `req_frac_inst_executed`; -1 to use fraction instead");
      LinkVar(req_frac_inst_copied, "req_frac_inst_copied", 
              "The organism must have copied at least this fraction of their genome to"
                " reproduce via HDivide. Otherwise HDivide does nothing.");
      LinkVar(org_pos_trait, "pos_trait", "Name of trait that holds organism's position");
      LinkVar(offspring_genome_trait, "offspring_genome_trait", 
          "Name of trait that holds the offspring organism's genome");
      LinkVar(reset_self_trait, "reset_self_trait", 
          "Name of trait that determines if the organism needs reset");
    }

    /// When config is loaded, create traits and set up functions
    void SetupModule() override {
      AddRequiredTrait<OrgPosition>(org_pos_trait);
      AddRequiredTrait<org_t::genome_t>(offspring_genome_trait);
      AddRequiredTrait<bool>(reset_self_trait);
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      { // Head allocate 
        const inst_func_t func_h_alloc = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_HAlloc(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
              "HAlloc", func_h_alloc);
      }
      { // Head divide 
        const inst_func_t func_h_divide = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_HDivide(hw, inst); };
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "HDivide", func_h_divide);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
      { // Head copy 
        const inst_func_t func_h_copy = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_HCopy(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "HCopy", func_h_copy);
      }
      { // Head search 
        const inst_func_t func_h_search = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_HSearch(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "HSearch", func_h_search);
      }
      { // Repro 
        const inst_func_t func_repro = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Repro(hw, inst); };
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Repro", func_repro);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Replication, "Replication instructions for VirtualCPUOrg");
}

#endif
