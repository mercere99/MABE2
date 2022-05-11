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
    using inst_func_t = VirtualCPUOrg::inst_func_t;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
  private:
    bool include_label = true;          ///< Config option indicating if instruction is used
    bool include_search_label_direct_s = false; ///< Config option indicating if inst. is used
    bool include_search_label_direct_f = false; ///< Config option indicating if inst. is used
    bool include_search_label_direct_b = false; ///< Config option indicating if inst. is used
    bool include_search_seq_direct_s = false;   ///< Config option indicating if inst. is used
    bool include_search_seq_direct_f = false;   ///< Config option indicating if inst. is used
    bool include_search_seq_direct_b = false;   ///< Config option indicating if inst. is used
    int label_id = -1;                 ///< ID of the label instruction 
    int search_label_direct_s_id = -1; ///< ID of the search_label_direct_s instruction  
    int search_label_direct_f_id = -1; ///< ID of the search_label_direct_f instruction 
    int search_label_direct_b_id = -1; ///< ID of the search_label_direct_b instruction 
    int search_seq_direct_s_id = -1;   ///< ID of the search_seq_direct_s instruction 
    int search_seq_direct_f_id = -1;   ///< ID of the search_seq_direct_f instruction
    int search_seq_direct_b_id = -1;   ///< ID of the search_seq_direct_b instruction

  public:
    VirtualCPU_Inst_Label(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_Label",
                    const std::string & desc="Label control instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;}
    ~VirtualCPU_Inst_Label() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
      LinkVar(include_label, "include_label", 
          "Do we include the 'label' instruction?");
      LinkVar(include_search_label_direct_s, "include_search_label_direct_s", 
          "Do we include the 'search_label_direct_s' instruction?");
      LinkVar(include_search_label_direct_f, "include_search_label_direct_f", 
          "Do we include the 'search_label_direct_f' instruction?");
      LinkVar(include_search_label_direct_b, "include_search_label_direct_b", 
          "Do we include the 'search_label_direct_b' instruction?");
      LinkVar(include_search_seq_direct_s, "include_search_seq_direct_s", 
          "Do we include the 'search_seq_direct_s' instruction?");
      LinkVar(include_search_seq_direct_f, "include_search_seq_direct_f", 
          "Do we include the 'search_seq_direct_f' instruction?");
      LinkVar(include_search_seq_direct_b, "include_search_seq_direct_b", 
          "Do we include the 'search_seq_direct_b' instruction?");
      LinkVar(label_id, "label_id", 
          "ID of the 'label' instruction");
      LinkVar(search_label_direct_s_id, "search_label_direct_s_id", 
          "ID of the 'search_label_direct_s' instruction");
      LinkVar(search_label_direct_f_id, "search_label_direct_f_id", 
          "ID of the 'search_label_direct_s' instruction");
      LinkVar(search_label_direct_b_id, "search_label_direct_b_id", 
          "ID of the 'search_label_direct_s' instruction");
      LinkVar(search_seq_direct_s_id, "search_seq_direct_s_id", 
          "ID of the 'search_seq_direct_s' instruction");
      LinkVar(search_seq_direct_f_id, "search_seq_direct_f_id", 
          "ID of the 'search_seq_direct_s' instruction");
      LinkVar(search_seq_direct_b_id, "search_seq_direct_b_id", 
          "ID of the 'search_seq_direct_s' instruction");
    }
    
    /// When config is loaded, set up functions
    void SetupModule() override {
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      if(include_label){ // Label 
        const inst_func_t func_label = [](VirtualCPUOrg& /*hw*/, const VirtualCPUOrg::inst_t& /*inst*/){ ; };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "Label", func_label);
        action.data.AddVar<int>("inst_id", label_id);
      }
      if(include_search_label_direct_s){ // SearchLabelDirectS 
        const inst_func_t func_search_label_direct_s = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindLabel(false, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchLabelDirectS", func_search_label_direct_s);
        action.data.AddVar<int>("inst_id", search_label_direct_s_id);
      }
      if(include_search_label_direct_f){ // SearchLabelDirectF 
        const inst_func_t func_search_label_direct_f = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindLabel(true, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchLabelDirectF", func_search_label_direct_f);
        action.data.AddVar<int>("inst_id", search_label_direct_f_id);
      }
      if(include_search_label_direct_b){ // SearchLabelDirectB 
        const inst_func_t func_search_label_direct_b = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindLabel(true, true); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchLabelDirectB", func_search_label_direct_b);
        action.data.AddVar<int>("inst_id", search_label_direct_b_id);
      }
      if(include_search_seq_direct_s){ // SearchSeqDirectS 
        const inst_func_t func_search_seq_direct_s = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindNopSequence(false, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchSeqDirectS", func_search_seq_direct_s);
        action.data.AddVar<int>("inst_id", search_seq_direct_s_id);
      }
      if(include_search_seq_direct_f){ // SearchSeqDirectF 
        const inst_func_t func_search_seq_direct_f = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindNopSequence(true, false); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchSeqDirectF", func_search_seq_direct_f);
        action.data.AddVar<int>("inst_id", search_seq_direct_f_id);
      }
      if(include_search_seq_direct_b){ // SearchSeqDirectB 
        const inst_func_t func_search_seq_direct_b = [](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){ 
          hw.flow_head = hw.FindNopSequence(true, true); 
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "SearchSeqDirectB", func_search_seq_direct_b);
        action.data.AddVar<int>("inst_id", search_seq_direct_b_id);
      }
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_Label, "Label control instructions for VirtualCPUOrg");
}

#endif
