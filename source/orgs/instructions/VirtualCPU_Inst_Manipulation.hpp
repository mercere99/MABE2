/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Manipulation.hpp
 *  @brief Provides manipulation instructions to a population of VirtualCPUOrgs.
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
      func_pop = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        hw.StackPop(idx);
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("Pop", func_pop);
      // Push 
      func_push = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        hw.StackPush(idx);
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("Push", func_push);
      // Swap stack 
      func_swap_stack = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        hw.StackSwap();
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("SwapStk", 
          func_swap_stack);
      // Swap 
      func_swap = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_2 = hw.GetComplementIdx(idx_1);
        data_t tmp = hw.regs[idx_1];
        hw.regs[idx_1] = hw.regs[idx_2];
        hw.regs[idx_2] = tmp;
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("Swap", func_swap);
      // Move head 
      func_mov_head = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        if(inst.nop_vec.empty())
          hw.inst_ptr = hw.flow_head - 1;
        else{
          if(inst.nop_vec[0] == 0)
            hw.inst_ptr = hw.flow_head - 1;
          else if(inst.nop_vec[0] == 1)
            hw.read_head = hw.flow_head;
          else if(inst.nop_vec[0] == 2)
            hw.write_head = hw.flow_head;
        }
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("MovHead", 
          func_mov_head);
      // Jump head 
      func_jmp_head = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        size_t& head = hw.inst_ptr;
        if(!inst.nop_vec.empty()){
          if(inst.nop_vec[0] == 0)
            head = hw.inst_ptr;
          else if(inst.nop_vec[0] == 1)
            head = hw.read_head;
          else if(inst.nop_vec[0] == 2)
            head = hw.write_head;
        }
        head += hw.regs[2];
        while(head >= hw.genome_working.size())
          head -= hw.genome_working.size();
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("JumpHead", 
          func_jmp_head);
      // Get head  
      func_get_head = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        if(inst.nop_vec.empty())
          hw.regs[2] = hw.inst_ptr;
        else{
          if(inst.nop_vec[0] == 0)
            hw.regs[2] = hw.inst_ptr;
          else if(inst.nop_vec[0] == 1)
            hw.regs[2] = hw.read_head;
          else if(inst.nop_vec[0] == 2)
            hw.regs[2] = hw.write_head;
        }
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("GetHead", 
          func_get_head);
      // Set flow  
      func_set_flow = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        size_t idx = inst.nop_vec.empty() ? 2 : inst.nop_vec[0];
        hw.flow_head = hw.regs[idx];
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("SetFlow", 
          func_set_flow);
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Manipulation, "Manipulation instructions for VirtualCPUOrg");
}

#endif
