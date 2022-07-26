/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPU_Inst_Manipulation.hpp
 *  @brief Provides data and head manipulation instructions to a population of VirtualCPUOrgs.
 *
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_MANIPULATION_H
#define MABE_VIRTUAL_CPU_INST_MANIPULATION_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  /// A collection of head and data manipulation instructions to be used by VirtualCPUOrgs 
  class VirtualCPU_Inst_Manipulation : public Module {
  public: 
    using org_t = VirtualCPUOrg;
    using data_t = org_t::data_t;
    using inst_func_t = org_t::inst_func_t;
    using this_t = VirtualCPU_Inst_Manipulation;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions

  public:
    VirtualCPU_Inst_Manipulation(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Manipulation",
                    const std::string & desc="Manipulation instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;}
    ~VirtualCPU_Inst_Manipulation() { }
    
    void Inst_Pop(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      if(!inst.nop_vec.empty()) hw.AdvanceIP();
      hw.StackPop(idx);
    }
    void Inst_Push(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      if(!inst.nop_vec.empty()) hw.AdvanceIP();
      hw.StackPush(idx);
    }
    void Inst_SwapStack(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.StackSwap();
    }
    void Inst_Swap(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = inst.nop_vec.size() < 2 ? hw.GetComplementNop(idx_1) : inst.nop_vec[1];
        data_t tmp = hw.regs[idx_1];
        hw.regs[idx_1] = hw.regs[idx_2];
        hw.regs[idx_2] = tmp;
      }
      else {
        size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementNop(idx_1);
        data_t tmp = hw.regs[idx_1];
        hw.regs[idx_1] = hw.regs[idx_2];
        hw.regs[idx_2] = tmp;
      }
    }
    void Inst_MoveHead(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t dest_idx = hw.flow_head;
        if(inst.nop_vec.size() >= 2) dest_idx = hw.GetModdedHead(inst.nop_vec[1]);
        if(!inst.nop_vec.empty()) hw.SetModdedHead(inst.nop_vec[0], dest_idx);
        else hw.SetIP(dest_idx);
      }
      else{
        if(!inst.nop_vec.empty()){
          // IP is a special case because it auto advances!
          if(inst.nop_vec[0] % 4 == 0) hw.SetIP(hw.flow_head - 1);
          else hw.SetModdedHead(inst.nop_vec[0], hw.flow_head);
        }
        else hw.SetIP(hw.flow_head - 1);
      }
    }
    void Inst_JumpHead(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t jump_dist = hw.regs[1];
        if(inst.nop_vec.size() >= 2) jump_dist = hw.regs[inst.nop_vec[1]];
        if(!inst.nop_vec.empty()) hw.AdvanceModdedHead(inst.nop_vec[0], jump_dist);
        else hw.AdvanceIP(jump_dist);
      }
      else{
        if(!inst.nop_vec.empty()) hw.AdvanceModdedHead(inst.nop_vec[0], hw.regs[2]);
        else hw.AdvanceIP(hw.regs[2]);
      }
    }
    void Inst_GetHead(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t head_val = 
            inst.nop_vec.empty() ? hw.inst_ptr : hw.GetModdedHead(inst.nop_vec[0]);
        if(inst.nop_vec.size() < 2) hw.regs[2] = head_val;
        else hw.regs[inst.nop_vec[1]] = head_val;
      }
      else{
        if(inst.nop_vec.empty()) hw.regs[2] = hw.inst_ptr;
        else hw.regs[2] = hw.GetModdedHead(inst.nop_vec[0]);
      }
    }
    void Inst_SetFlow(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 2 : inst.nop_vec[0];
      hw.SetFH(hw.regs[idx]);
    }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    /// When config is loaded, set up functions
    void SetupModule() override {
      SetupFuncs();
    }
    
    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      { // Pop 
        const inst_func_t func_pop = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Pop(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Pop", func_pop);
      }
      { // Push 
        const inst_func_t func_push = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Push(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Push", func_push);
      }
      { // Swap stack 
        const inst_func_t func_swap_stack = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SwapStack(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SwapStk", func_swap_stack);
      }
      { // Swap 
        const inst_func_t func_swap = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Swap(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Swap", func_swap);
      }
      { // Move head 
        const inst_func_t func_mov_head = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_MoveHead(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "MovHead", func_mov_head);
      }
      { // Jump head 
        const inst_func_t func_jmp_head = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_JumpHead(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "JumpHead", func_jmp_head);
      }
      { // Get head  
        const inst_func_t func_get_head = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_GetHead(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "GetHead", func_get_head);
      }
      { // Set flow  
        const inst_func_t func_set_flow = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SetFlow(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SetFlow", func_set_flow);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Manipulation, "Manipulation instructions for VirtualCPUOrg");
}

#endif
