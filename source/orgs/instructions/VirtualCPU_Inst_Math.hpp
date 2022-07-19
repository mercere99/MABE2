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
    using org_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;
    using this_t = VirtualCPU_Inst_Math;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions

  public:
    VirtualCPU_Inst_Math(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Math",
                    const std::string & desc="Math instructions for VirtualCPUOrg population")
      : Module(control, name, desc){;}
    ~VirtualCPU_Inst_Math() { }

    void Inst_Inc(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      ++hw.regs[idx];
    }
    void Inst_Dec(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      --hw.regs[idx];
    }
    void Inst_Add(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
        size_t idx_op_2 = 
            inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
        hw.regs[idx_res] = hw.regs[idx_op_1] + hw.regs[idx_op_2];
      }
      else{ // Nop determines destination, computation is always B + C
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        hw.regs[idx] = hw.regs[1] + hw.regs[2];
      }
    }
    void Inst_Sub(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
        size_t idx_op_2 = 
            inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
        hw.regs[idx_res] = hw.regs[idx_op_1] - hw.regs[idx_op_2];
      }
      else{ // Nop determines destination. Computation is always B - C
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        hw.regs[idx] = hw.regs[1] - hw.regs[2];
      }
    }
    void Inst_Nand(org_t& hw, const org_t::inst_t& inst){
      if(hw.expanded_nop_args){
        size_t idx_res = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        size_t idx_op_1 = inst.nop_vec.size() < 2 ? idx_res : inst.nop_vec[1];
        size_t idx_op_2 = 
            inst.nop_vec.size() < 3 ? hw.GetComplementNop(idx_op_1) : inst.nop_vec[2];
        hw.regs[idx_res] = ~(hw.regs[idx_op_1] & hw.regs[idx_op_2]);
      }
      else{ // Nop determines destination. Computation is always B NAND C
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        hw.regs[idx] = ~(hw.regs[1] & hw.regs[2]);
      }
    }
    void Inst_ShiftL(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      hw.regs[idx] <<= 1;
    }
    void Inst_ShiftR(org_t& hw, const org_t::inst_t& inst){
      size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
      hw.regs[idx] >>= 1;
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
      { // Increment
        const inst_func_t func_inc = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Inc(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Inc", func_inc);
      }
      { // Decrement 
        const inst_func_t func_dec = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Dec(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Dec", func_dec);
      }
      { // Add 
        const inst_func_t func_add = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Add(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Add", func_add);
      }
      { // Sub 
        const inst_func_t func_sub = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Sub(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Sub", func_sub);
      }
      { // NAND 
        const inst_func_t func_nand = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Nand(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Nand", func_nand);
      }
      { // Shift Left 
        const inst_func_t func_shift_l = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_ShiftL(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "ShiftL", func_shift_l);
      }
      { // Shift Right 
        const inst_func_t func_shift_r = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_ShiftR(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "ShiftR", func_shift_r);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Math, "Math instructions for VirtualCPUOrg");
}

#endif
