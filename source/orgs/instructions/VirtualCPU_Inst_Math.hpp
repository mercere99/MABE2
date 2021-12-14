/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Math.hpp
 *  @brief Provides math instructions to a population of VirtualCPUOrgs.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_MATH_H
#define MABE_VIRTUAL_CPU_INST_MATH_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_Math : public Module {
  private:
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_inc;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_dec;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_add;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_sub;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_nand;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_shift_l;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_shift_r;

  public:
    VirtualCPU_Inst_Math(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Math",
                    const std::string & desc="Math instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Math() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Increment
      {
        func_inc = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          ++hw.regs[idx];
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Inc", func_inc);
          action.data.AddVar<int>("inst_id", 12);
      }
      // Decrement 
      {
        func_dec = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          --hw.regs[idx];
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Dec", func_dec);
        action.data.AddVar<int>("inst_id", 13);
      }
      // Shift Right 
      {
        func_shift_r = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[idx] >>= 1;
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "ShiftR", func_shift_r);
        action.data.AddVar<int>("inst_id", 10);
      }
      // Shift Left 
      {
        func_shift_l = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[idx] <<= 1;
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "ShiftL", func_shift_l);
        action.data.AddVar<int>("inst_id", 11);
      }
      // Add 
      {
        func_add = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          if(hw.expanded_nop_args){
            size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
            size_t idx_op_2 = inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
            hw.regs[idx_res] = hw.regs[idx_op_1] + hw.regs[idx_op_2];
          }
          else{
            size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_2 = hw.GetComplementNop(idx);
            hw.regs[idx] = hw.regs[idx] + hw.regs[idx_2];
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Add", func_add);
        action.data.AddVar<int>("inst_id", 18);
      }
      // Sub 
      {
        func_sub = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          if(hw.expanded_nop_args){
            size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
            size_t idx_op_2 = inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
            hw.regs[idx_res] = hw.regs[idx_op_1] - hw.regs[idx_op_2];
          }
          else{
            size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_2 = hw.GetComplementNop(idx);
            hw.regs[idx] = hw.regs[idx] - hw.regs[idx_2];
          }
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Sub", func_sub);
        action.data.AddVar<int>("inst_id", 19);
      }
      // NAND 
      {
        func_nand = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          if(hw.expanded_nop_args){
            size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
            size_t idx_op_2 = inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
            hw.regs[idx_res] = ~(hw.regs[idx_op_1] & hw.regs[idx_op_2]);
          }
          else{
            size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            size_t idx_2 = hw.GetComplementNop(idx);
            hw.regs[idx] = hw.regs[idx] + hw.regs[idx_2];
            hw.regs[idx] = ~(hw.regs[idx] & hw.regs[idx_2]);
          }
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Nand", func_nand);
        action.data.AddVar<int>("inst_id", 20);
      }
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Math, "Math instructions for VirtualCPUOrg");
}

#endif
