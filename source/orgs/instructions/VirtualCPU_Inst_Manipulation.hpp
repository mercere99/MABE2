/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Manipulation.hpp
 *  @brief Provides manipulation instructions to a population of VirtualCPUOrgs.
 *
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_MANIPULATION_H
#define MABE_VIRTUAL_CPU_INST_MANIPULATION_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_Manipulation : public Module {
  public: 
    using data_t = VirtualCPUOrg::data_t;
  private:
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_pop;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_push;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_swap_stack;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_swap;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_mov_head;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_jmp_head;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_get_head;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_set_flow;

  public:
    VirtualCPU_Inst_Manipulation(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Manipulation",
                    const std::string & desc="Manipulation instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Manipulation() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Pop 
      {
        func_pop = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.StackPop(idx);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Pop", func_pop);
        action.data.AddVar<int>("inst_id", 15);
      }
      // Push 
      {
        func_push = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.StackPush(idx);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Push", func_push);
        action.data.AddVar<int>("inst_id", 14);
      }
      // Swap stack 
      { 
        func_swap_stack = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          hw.StackSwap();
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SwapStk", func_swap_stack);
        action.data.AddVar<int>("inst_id", 16);
      }
      // Swap 
      {
        func_swap = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          if(hw.expanded_nop_args){
            size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_2 = inst.nop_vec.size() < 2 ? hw.GetComplementIdx(idx_1) : inst.nop_vec[1];
            data_t tmp = hw.regs[idx_1];
            hw.regs[idx_1] = hw.regs[idx_2];
            hw.regs[idx_2] = tmp;
          }
          else {
            size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_2 = hw.GetComplementIdx(idx_1);
            data_t tmp = hw.regs[idx_1];
            hw.regs[idx_1] = hw.regs[idx_2];
            hw.regs[idx_2] = tmp;
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Swap", func_swap);
        action.data.AddVar<int>("inst_id", 17);
      }
      // Move head 
      {
        func_mov_head = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
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
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "MovHead", func_mov_head);
        action.data.AddVar<int>("inst_id", 6);
      }
      // Jump head 
      {
        func_jmp_head = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
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
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "JumpHead", func_jmp_head);
        action.data.AddVar<int>("inst_id", 7);
      }
      // Get head  
      {
        func_get_head = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          if(hw.expanded_nop_args){
            size_t head_val = inst.nop_vec.empty() ? hw.inst_ptr : hw.GetModdedHead(inst.nop_vec[0]);
            if(inst.nop_vec.size() < 2) hw.regs[2] = head_val;
            else hw.regs[inst.nop_vec[1]] = head_val;
          }
          else{
            if(inst.nop_vec.empty()) hw.regs[2] = hw.inst_ptr;
            else hw.regs[2] = hw.GetModdedHead(inst.nop_vec[0]);
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "GetHead", func_get_head);
        action.data.AddVar<int>("inst_id", 8);
      }
      // Set flow  
      {
        func_set_flow = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 2 : inst.nop_vec[0];
          hw.SetFH(hw.regs[idx]);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SetFlow", func_set_flow);
        action.data.AddVar<int>("inst_id", 9);
      }
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Manipulation, "Manipulation instructions for VirtualCPUOrg");
}

#endif
