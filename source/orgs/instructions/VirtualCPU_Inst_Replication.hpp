/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPU_Inst_Replication.hpp
 *  @brief Provides replication instructions to a population of VirtualCPUOrgs.
 *
 *  TODO: 
 *    - Currently a trait that stores the offspring genome is created, but is unused in the 
 *        replication instructions. 
 *      - HCopy (and other instructions) should be able to add mutations to this genome
 *      - When it's time to replicate, HDivide _should_ use this genome instead of relying on
 *          reproduction mechanisms from VirtualCPUOrg / MABE
 *      - How would this interact with other MABE systems?
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
    bool include_h_alloc  = true;  ///< Config option indicating if inst. is used
    bool include_h_divide = true;  ///< Config option indicating if inst. is used
    bool include_h_copy   = true;  ///< Config option indicating if inst. is used
    bool include_h_search = true;  ///< Config option indicating if inst. is used
    bool include_repro    = false; ///< Config option indicating if inst. is used
    double req_frac_inst_executed = 0.5;  /**< Config option indicating the fraction of 
                                            an organism's genome that must have been executed 
                                            for org to reproduce **/
    int req_count_inst_executed = -1;     /**< Config option indicating the number of 
                                            instructions an organism must have executed in 
                                            order to reproduce **/
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

    void Inst_HAlloc(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.genome_working.resize(hw.genome.size() * 2, hw.GetDefaultInst());
      hw.regs[0] = hw.genome.size();
    }
    void Inst_HDivide(org_t& hw, const org_t::inst_t& /*inst*/){
      // First make sure that: 
        // If required inst count isn't negative, that many insts have been executed 
        // Else, the correct fraction of the genome has been executed
      if( (req_count_inst_executed >= 0 
            && hw.num_insts_executed >= req_count_inst_executed)
          || (req_count_inst_executed < 0 
            && hw.num_insts_executed >= req_frac_inst_executed * hw.genome_working.size())){
        OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
        //org_t::genome_t& offspring_genome = hw.GetTrait<org_t::genome_t>(
        //    "offspring_genome");
        //offspring_genome.resize(0,0);
        //offspring_genome.resize(hw.genome_working.size() - hw.read_head, 
        //    hw.GetDefaultInst()); 
        //std::copy(
        //    hw.genome_working.begin() + hw.read_head, 
        //    hw.genome_working.end(),
        //    offspring_genome.begin());
        //hw.genome_working.resize(hw.read_head, hw.GetDefaultInst());
        hw.ResetWorkingGenome();
        hw.ResetHardware();
        hw.inst_ptr = hw.genome.size() - 1;
        control.Replicate(org_pos, *org_pos.PopPtr());
        hw.SetTrait<bool>(reset_self_trait, true);
      }
    }
    void Inst_HCopy(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.genome_working[hw.write_head] = hw.genome_working[hw.read_head];
      hw.copied_inst_id_vec.push_back(hw.genome_working[hw.write_head].id);
      hw.read_head++;
      while(hw.read_head >= hw.genome_working.size()){
        hw.read_head -= hw.genome_working.size();
      }
      hw.write_head++;
      while(hw.write_head >= hw.genome_working.size()){
        hw.write_head -= hw.genome_working.size();
      }
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
            && hw.num_insts_executed >= req_count_inst_executed)
          || (req_count_inst_executed < 0 
            && hw.num_insts_executed >= req_frac_inst_executed * hw.genome_working.size())){
        OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
        org_t::genome_t& offspring_genome = hw.GetTrait<org_t::genome_t>(
            "offspring_genome");
        offspring_genome = hw.genome_working;
        std::copy(
            hw.genome_working.begin(), 
            hw.genome_working.end(),
            offspring_genome.begin());
        hw.ResetHardware();
        // Set to end so completion of this inst moves it 0 
        hw.inst_ptr = hw.genome_working.size() - 1; 
        control.Replicate(org_pos, *org_pos.PopPtr());
        //control.DoBirth(hw, org_pos, org_pos, false); // Reset parent
        hw.SetTrait<bool>(reset_self_trait, true);
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
      LinkVar(org_pos_trait, "pos_trait", "Name of trait that holds organism's position");
      LinkVar(offspring_genome_trait, "offspring_genome_trait", 
          "Name of trait that holds the offspring organism's genome");
      LinkVar(reset_self_trait, "reset_self_trait", 
          "Name of trait that determines if the organism needs reset");
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
      AddRequiredTrait<org_t::genome_t>(offspring_genome_trait);
      AddRequiredTrait<bool>(reset_self_trait);
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      if(include_h_alloc){ // Head allocate 
        const inst_func_t func_h_alloc = std::bind(&this_t::Inst_HAlloc, this, 
            std::placeholders::_1, std::placeholders::_2);
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
              "HAlloc", func_h_alloc);
        action.data.AddVar<int>("inst_id", h_alloc_id);
      }
      if(include_h_divide){ // Head divide 
        const inst_func_t func_h_divide = std::bind(&this_t::Inst_HDivide, this, 
            std::placeholders::_1, std::placeholders::_2);
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "HDivide", func_h_divide);
        action.data.AddVar<int>("inst_id", h_divide_id);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
      if(include_h_copy){ // Head copy 
        const inst_func_t func_h_copy = std::bind(&this_t::Inst_HCopy, this, 
            std::placeholders::_1, std::placeholders::_2);
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "HCopy", func_h_copy);
        action.data.AddVar<int>("inst_id", h_copy_id);
      }
      if(include_h_search){ // Head search 
        const inst_func_t func_h_search = std::bind(&this_t::Inst_HSearch, this, 
            std::placeholders::_1, std::placeholders::_2);
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "HSearch", func_h_search);
        action.data.AddVar<int>("inst_id", h_search_id);
      }
      if(include_repro){ // Repro 
        const inst_func_t func_repro = std::bind(&this_t::Inst_Repro, this, 
            std::placeholders::_1, std::placeholders::_2);
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Repro", func_repro);
        action.data.AddVar<int>("inst_id", repro_id);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Replication, "Replication instructions for VirtualCPUOrg");
}

#endif
