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
    size_t num_nops = 3;
    bool include_nop_x = false;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_nop;

  public:
    VirtualCPU_Inst_Nop(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Nop",
                    const std::string & desc="Nop instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Nop() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
       LinkVar(num_nops, "num_nops", "Number of nops to include.");
       LinkVar(include_nop_x, "include_nop_x", "Include the special case NopX?");
    }

    void SetupFuncs(){
      emp_assert(num_nops < 12, "Code only supports twelve normal NOP instructions currently");
      ActionMap& action_map = control.GetActionMap(pop_id);
      func_nop = [](VirtualCPUOrg& /*hw*/, const VirtualCPUOrg::inst_t& /*inst*/){ ; };
      // Nop A
      if(num_nops >= 1){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopA", func_nop);
        action.data.AddVar<int>("inst_id", 0);
      }
      // Nop B
      if(num_nops >= 2){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopB", func_nop);
        action.data.AddVar<int>("inst_id", 1);
      }
      // Nop C
      if(num_nops >= 3){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopC", func_nop);
        action.data.AddVar<int>("inst_id", 2);
      }
      // Nop D
      if(num_nops >= 4){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopD", func_nop);
        action.data.AddVar<int>("inst_id", 3);
      }
      // Nop E
      if(num_nops >= 5){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopE", func_nop);
        action.data.AddVar<int>("inst_id", 4);
      }
      // Nop F
      if(num_nops >= 6){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopF", func_nop);
        action.data.AddVar<int>("inst_id", 5);
      }
      // Nop G
      if(num_nops >= 7){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopG", func_nop);
        action.data.AddVar<int>("inst_id", 6);
      }
      // Nop H
      if(num_nops >= 8){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopH", func_nop);
        action.data.AddVar<int>("inst_id", 7);
      }
      // Nop I
      if(num_nops >= 9){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopI", func_nop);
        action.data.AddVar<int>("inst_id", 8);
      }
      // Nop J
      if(num_nops >= 10){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopJ", func_nop);
        action.data.AddVar<int>("inst_id", 9);
      }
      // Nop K
      if(num_nops >= 11){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopK", func_nop);
        action.data.AddVar<int>("inst_id", 10);
      }
      // Nop L
      if(num_nops >= 12){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopL", func_nop);
        action.data.AddVar<int>("inst_id", 11);
      }
      // Special case: Nop X
      if(include_nop_x){
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "NopX", func_nop);
        action.data.AddVar<int>("inst_id", 50);
      }
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Nop, "Nop instructions for VirtualCPUOrg");
}

#endif
