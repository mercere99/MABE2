/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPU_Inst_Nop.hpp
 *  @brief Provides NOP instructions to a population of VirtualCPUOrgs.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_NOP_H
#define MABE_VIRTUAL_CPU_INST_NOP_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {
  /// A collection of NOP instructions to be used by VirtualCPUOrgs 
  class VirtualCPU_Inst_Nop : public Module {
  public:
    using org_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;
    using this_t = VirtualCPU_Inst_Nop;

  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
  public:
    size_t num_nops = 3; ///< The number of "normal" nops to include (starting with NopA)

    VirtualCPU_Inst_Nop(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Nop",
                    const std::string & desc="Nop instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;} 
    ~VirtualCPU_Inst_Nop() { }


    void Inst_Nop(org_t& /*hw*/, const org_t::inst_t& /*inst*/){;}
    void Inst_NopX(org_t& /*hw*/, const org_t::inst_t& /*inst*/){;}

    /// Set up variables for configuration file
    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
       LinkVar(num_nops, "num_nops", "Number of nops to include.");
    }
    
    /// When config is loaded, set up functions
    void SetupModule() override {
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      emp_assert(num_nops <= 23,"Code only supports 23 normal NOP instructions currently");
      ActionMap& action_map = control.GetActionMap(pop_id);
      const inst_func_t func_nop = 
        [this](org_t& hw, const org_t::inst_t& inst){ Inst_Nop(hw, inst); };
      // Add the appropriate amount of nops
      for(size_t i = 0; i < num_nops; i++){
        std::string s = "Nop";
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            s + (char)('A' + i), func_nop);
      }
      { // Special case: Nop X
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "NopX", func_nop);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Nop, "Nop instructions for VirtualCPUOrg");
}

#endif
