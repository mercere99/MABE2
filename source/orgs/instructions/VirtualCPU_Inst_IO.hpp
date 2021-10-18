/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  VirtualCPU_Inst_IO.hpp
 *  @brief Provides VirtualCPUOrgs an IO function that loads a new input and caches the output.
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_IO_H
#define MABE_VIRTUAL_CPU_INST_IO_H

#include <limits>

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  class VirtualCPU_Inst_IO : public Module {
  public: 
    using data_t = VirtualCPUOrg::data_t;
  private:
    Collection target_collect;
    int pop_id = 0;
    std::string input_name = "input";
    std::string output_name = "input";
    std::string input_idx_name = "input_idx";
    size_t num_inputs = 3;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_input;

  public:
    VirtualCPU_Inst_IO(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_IO",
                    const std::string & desc="IO instructions for VirtualCPUOrg population")
      : Module(control, name, desc), 
        target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}

    ~VirtualCPU_Inst_IO() {;}

    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
       LinkVar(input_name, "input_name", "Name of variable to store inputs");
       LinkVar(output_name, "output_name", "Name of variable to store outputs");
       LinkVar(input_idx_name, "input_idx_name", "Index of next input to be loaded");
    }

    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);

      func_input = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
        emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(input_name);
        emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(output_name);
        size_t& input_idx = hw.GetTrait<size_t>(input_idx_name);
        // Ensure inputs have been generated
        if(input_vec.size() < num_inputs){
          for(size_t idx = input_vec.size(); idx < num_inputs; idx++){
            input_vec.push_back(
                (data_t)(std::numeric_limits<data_t>::max() * control.GetRandom().GetDouble()) );
          }
        }
        // Load input into register
        size_t reg_idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        output_vec.push_back(hw.regs[reg_idx]);
        hw.regs[reg_idx] = input_vec[input_idx];
        // Advance stored index
        input_idx++;
        if(input_idx >= num_inputs) input_idx = 0;
      };

      Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
          "IO", func_input);
      action.data.AddVar<int>("inst_id", 24);
    }

    void SetupModule() override {
      AddOwnedTrait<emp::vector<data_t>>(input_name, "VirtualCPUOrg inputs", {} );
      AddOwnedTrait<size_t>(input_idx_name, "Index of next input", 0 );
      AddOwnedTrait<emp::vector<data_t>>(output_name, "VirtualCPUOrg outputs", {} );
      SetupFuncs();
    }
  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_IO, "IO instructions for VirtualCPUOrg");
}

#endif
