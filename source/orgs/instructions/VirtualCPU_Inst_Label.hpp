/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_Label.hpp
 *  @brief Provides label declaration and search instructions to a population of VirtualCPUOrgs.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_LABEL_H
#define MABE_VIRTUAL_CPU_INST_LABEL_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_Label : public Module {
  private:
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_label;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_search_label_direct_s;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_search_label_direct_f;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_search_label_direct_b;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_search_seq_direct_s;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_search_seq_direct_f;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_search_seq_direct_b;

  public:
    VirtualCPU_Inst_Label(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Label",
                    const std::string & desc="Label control instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}
    ~VirtualCPU_Inst_Label() { }

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Label 
      {
        func_label = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ ; };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Label", func_label);
        action.data.AddVar<int>("inst_id", 30);
      }
      // SearchLabelDirectS 
      {
        func_search_label_direct_s = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindMarkedLabel(false, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchLabelDirectS", func_search_label_direct_s);
        action.data.AddVar<int>("inst_id", 31);
      }
      // SearchLabelDirectF 
      {
        func_search_label_direct_f = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindMarkedLabel(true, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchLabelDirectF", func_search_label_direct_f);
        action.data.AddVar<int>("inst_id", 32);
      }
      // SearchLabelDirectB 
      {
        func_search_label_direct_b = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindMarkedLabel(true, true); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchLabelDirectB", func_search_label_direct_b);
        action.data.AddVar<int>("inst_id", 33);
      }
      // SearchSeqDirectS 
      {
        func_search_seq_direct_s = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindSequence(false, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchSeqDirectS", func_search_seq_direct_s);
        action.data.AddVar<int>("inst_id", 34);
      }
      // SearchSeqDirectF 
      {
        func_search_seq_direct_f = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindSequence(true, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchSeqDirectF", func_search_seq_direct_f);
        action.data.AddVar<int>("inst_id", 35);
      }
      // SearchSeqDirectB 
      {
        func_search_seq_direct_b = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindSequence(true, true); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchSeqDirectB", func_search_seq_direct_b);
        action.data.AddVar<int>("inst_id", 36);
      }
    }

    void SetupModule() override {
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Label, "Label control instructions for VirtualCPUOrg");
}

#endif
