/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Flow.hpp
 *  @brief Provides flow control instructions to a population of VirtualCPUOrgs.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_FLOW_H
#define MABE_VIRTUAL_CPU_INST_FLOW_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_Flow : public Module {
  private:
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_if_n_equ;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_if_less;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_if_label;

  public:
    VirtualCPU_Inst_Flow(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Flow",
                    const std::string & desc="Flow control instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Flow() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // If not equal
      {
        func_if_n_equ = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          size_t idx_2 = hw.GetComplementIdx(idx_1);
          if(hw.regs[idx_1] == hw.regs[idx_2])
            hw.AdvanceIP(1);
          if(inst.nop_vec.size()) hw.AdvanceIP(1); 
        };
        Action& action = 
            action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
                "IfNEqu",func_if_n_equ);
        action.data.AddVar<int>("inst_id", 3);
      }
      // If less 
      {
        func_if_less = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          size_t idx_1 = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          size_t idx_2 = hw.GetComplementIdx(idx_1);
          if(hw.regs[idx_1] >= hw.regs[idx_2])
            hw.AdvanceIP(1);
          if(inst.nop_vec.size()) hw.AdvanceIP(1); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "IfLess", func_if_less);
        action.data.AddVar<int>("inst_id", 4);
      }
      // If label 
      {
        func_if_label = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          //std::cout << "If label!" << std::endl;
          //std::cout << "Nop vec: [";
          //for(size_t i = 0; i < inst.nop_vec.size(); ++i){
          //  if(i != 0) std::cout << ", ";
          //  std::cout << inst.nop_vec[i];
          //}
          //std::cout << "]" << std::endl;
          //std::cout << "Copied instructions: [";
          //for(size_t i = 0; i < hw.copied_inst_id_vec.size(); ++i){
          //  if(i != 0) std::cout << ", ";
          //  std::cout << hw.copied_inst_id_vec[i];
          //}
          //std::cout << "]" << std::endl;
          hw.AdvanceIP(inst.nop_vec.size());
          if(!hw.CheckIfLastCopiedComplement(inst.nop_vec)) hw.AdvanceIP();
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&,const VirtualCPUOrg::inst_t&>(
            "IfLabel",func_if_label);
        action.data.AddVar<int>("inst_id", 5);
      }
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Flow, "Flow control instructions for VirtualCPUOrg");
}

#endif
