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
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_mult;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_div;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_mod;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_not;

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
      func_inc = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        size_t idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        ++hw.regs[idx];
      };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("Inc", func_inc);
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Math, "Math instructions for VirtualCPUOrg");
}

#endif
