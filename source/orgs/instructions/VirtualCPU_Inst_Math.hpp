/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
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

  /// A collection of math instructions to be used by VirtualCPUOrgs 
  class VirtualCPU_Inst_Math : public Module {
  public:
    using inst_func_t = VirtualCPUOrg::inst_func_t;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
    bool include_inc = true;     ///< Config option indicating if instruction is used
    bool include_dec = true;     ///< Config option indicating if instruction is used
    bool include_add = true;     ///< Config option indicating if instruction is used
    bool include_sub = true;     ///< Config option indicating if instruction is used
    bool include_nand = true;    ///< Config option indicating if instruction is used
    bool include_shift_l = true; ///< Config option indicating if instruction is used
    bool include_shift_r = true; ///< Config option indicating if instruction is used

  public:
    VirtualCPU_Inst_Math(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Math",
                    const std::string & desc="Math instructions for VirtualCPUOrg population")
      : Module(control, name, desc){;}
    ~VirtualCPU_Inst_Math() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
      LinkVar(include_inc, "include_inc", "Do we include the 'inc' instruction?");
      LinkVar(include_dec, "include_dec", "Do we include the 'dec' instruction?");
      LinkVar(include_add, "include_add", "Do we include the 'add' instruction?");
      LinkVar(include_sub, "include_sub", "Do we include the 'sub' instruction?");
      LinkVar(include_nand, "include_nand", "Do we include the 'nand' instruction?");
      LinkVar(include_shift_l, "include_shift_l", "Do we include the 'shift_l' instruction?");
      LinkVar(include_shift_r, "include_shift_r", "Do we include the 'shift_r' instruction?");
    }
    
    /// When config is loaded, set up functions
    void SetupModule() override {
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      if(include_inc){ // Increment
        const inst_func_t func_inc = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          ++hw.regs[idx];
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Inc", func_inc);
          action.data.AddVar<int>("inst_id", 12);
      }
      if(include_dec){ // Decrement 
        const inst_func_t func_dec = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          --hw.regs[idx];
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Dec", func_dec);
        action.data.AddVar<int>("inst_id", 13);
      }
      if(include_add){ // Add 
        const inst_func_t func_add = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
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
      if(include_sub){ // Sub 
        const inst_func_t func_sub = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
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
      if(include_nand){ // NAND 
        const inst_func_t func_nand = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
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
      if(include_shift_l){ // Shift Left 
        const inst_func_t func_shift_l = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[idx] <<= 1;
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "ShiftL", func_shift_l);
        action.data.AddVar<int>("inst_id", 11);
      }
      if(include_shift_r){ // Shift Right 
        const inst_func_t func_shift_r = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[idx] >>= 1;
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "ShiftR", func_shift_r);
        action.data.AddVar<int>("inst_id", 10);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Math, "Math instructions for VirtualCPUOrg");
}

#endif
