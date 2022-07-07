/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
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

  /// A collection of label declaration and search instructions to be used by VirtualCPUOrgs 
  class VirtualCPU_Inst_Label : public Module {
  public:
    using org_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;
    using this_t = VirtualCPU_Inst_Label;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions

  public:
    VirtualCPU_Inst_Label(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Label",
                    const std::string & desc="Label control instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;}
    ~VirtualCPU_Inst_Label() { }


    void Inst_Label(org_t& /*hw*/, const org_t::inst_t& /*inst*/){ ; }
    void Inst_SearchLabelDirectS(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.flow_head = hw.FindLabel(false, false); 
    }
    void Inst_SearchLabelDirectF(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.flow_head = hw.FindLabel(true, false); 
    }
    void Inst_SearchLabelDirectB(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.flow_head = hw.FindLabel(true, true); 
    }
    void Inst_SearchSeqDirectS(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.flow_head = hw.FindNopSequence(false, false); 
    }
    void Inst_SearchSeqDirectF(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.flow_head = hw.FindNopSequence(true, false); 
    }
    void Inst_SearchSeqDirectB(org_t& hw, const org_t::inst_t& /*inst*/){
      hw.flow_head = hw.FindNopSequence(true, true); 
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
      { // Label 
        const inst_func_t func_label = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_Label(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "Label", func_label);
      }
      { // SearchLabelDirectS 
        const inst_func_t func_search_label_direct_s = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SearchLabelDirectS(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SearchLabelDirectS", func_search_label_direct_s);
      }
      { // SearchLabelDirectF 
        const inst_func_t func_search_label_direct_f = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SearchLabelDirectF(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SearchLabelDirectF", func_search_label_direct_f);
      }
      { // SearchLabelDirectB 
        const inst_func_t func_search_label_direct_b = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SearchLabelDirectB(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SearchLabelDirectB", func_search_label_direct_b);
      }
      { // SearchSeqDirectS 
        const inst_func_t func_search_seq_direct_s = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SearchSeqDirectS(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SearchSeqDirectS", func_search_seq_direct_s);
      }
      { // SearchSeqDirectF 
        const inst_func_t func_search_seq_direct_f = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SearchSeqDirectF(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SearchSeqDirectF", func_search_seq_direct_f);
      }
      { // SearchSeqDirectB 
        const inst_func_t func_search_seq_direct_b = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_SearchSeqDirectB(hw, inst); };
        action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "SearchSeqDirectB", func_search_seq_direct_b);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Label, "Label control instructions for VirtualCPUOrg");
}

#endif
