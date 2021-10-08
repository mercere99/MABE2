/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Nop.hpp
 *  @brief Provides nop instructions to a population of VirtualCPUOrgs.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_NOP_H
#define MABE_VIRTUAL_CPU_INST_NOP_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_Nop : public Module {
  private:
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_nop_a;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_nop_b;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_nop_c;

  public:
    VirtualCPU_Inst_Nop(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Nop",
                    const std::string & desc="Nop instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Nop() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      func_nop_a = [](VirtualCPUOrg& /*hw*/, const VirtualCPUOrg::inst_t& /*inst*/){ ; };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("NopA", func_nop_a);
      func_nop_b = [](VirtualCPUOrg& /*hw*/, const VirtualCPUOrg::inst_t& /*inst*/){ ; };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("NopB", func_nop_b);
      func_nop_c = [](VirtualCPUOrg& /*hw*/, const VirtualCPUOrg::inst_t& /*inst*/){ ; };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("NopC", func_nop_c);
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Nop, "Nop instructions for VirtualCPUOrg");
}

#endif
