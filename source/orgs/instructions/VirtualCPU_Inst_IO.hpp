/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPU_Inst_IO.hpp
 *  @brief Provides VirtualCPUOrgs an IO instruction that loads a new input and caches the output
 * 
 */

#ifndef MABE_VIRTUAL_CPU_INST_IO_H
#define MABE_VIRTUAL_CPU_INST_IO_H

#include <limits>

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../VirtualCPUOrg.hpp"

namespace mabe {

  /// \brief Provides VirtualCPUOrgs an IO instruction that loads a new input and caches the output
  class VirtualCPU_Inst_IO : public Module {
  public: 
    using org_t = VirtualCPUOrg;
    using data_t = org_t::data_t;
    using inst_func_t = org_t::inst_func_t;
    using this_t = VirtualCPU_Inst_IO;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
    std::string input_name = "input";         ///< Name of trait that stores inputs
    std::string output_name = "output";       ///< Name of trait that stores outputs
    std::string input_idx_name = "input_idx"; ///< Number of trait that stores the index of the current input
    size_t num_inputs = 3; ///< Number of random inputs generated for each organism (they are reused if more inputs are requested)
    emp::vector<data_t> stamp_vec; /**< Vector of "stamps" that ensure that logic tasks on 
                                          inputs give unique outputs */

  public:
    VirtualCPU_Inst_IO(mabe::MABE & control,
                    const std::string & name="VirtualCPU_Inst_IO",
                    const std::string & desc="IO instructions for VirtualCPUOrg population")
      : Module(control, name, desc){ ; }

    ~VirtualCPU_Inst_IO() {;}

    void Inst_IO(org_t& hw, const org_t::inst_t& inst){
        emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(input_name);
        emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(output_name);
        size_t& input_idx = hw.GetTrait<size_t>(input_idx_name);
        // Ensure inputs have been generated
        if(input_vec.size() < num_inputs){
          for(size_t idx = input_vec.size(); idx < num_inputs; idx++){
            data_t rand_num = 
                (data_t)(std::numeric_limits<data_t>::max() * control.GetRandom().GetDouble()) ;
            input_vec.push_back((rand_num << 8) | stamp_vec[idx]);
          }
        }
        size_t reg_idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
        // Output current register value
        output_vec.push_back(hw.regs[reg_idx]);
        // Load input into register
        hw.regs[reg_idx] = input_vec[input_idx];
        // Advance stored index
        input_idx++;
        if(input_idx >= num_inputs) input_idx = 0;
        if(!inst.nop_vec.empty()) hw.AdvanceIP(1);
    }
    /// Set up variables for configuration file 
    void SetupConfig() override {
       LinkPop(pop_id, "target_pop", "Population(s) to manage.");
       LinkVar(input_name, "input_name", "Name of variable to store inputs");
       LinkVar(output_name, "output_name", "Name of variable to store outputs");
       LinkVar(input_idx_name, "input_idx_name", "Index of next input to be loaded");
    }

    /// Create organism traits and create IO instruction 
    void SetupModule() override {
      AddOwnedTrait<emp::vector<data_t>>(input_name, "VirtualCPUOrg inputs", {} );
      AddOwnedTrait<size_t>(input_idx_name, "Index of next input", 0 );
      AddSharedTrait<emp::vector<data_t>>(output_name, "VirtualCPUOrg outputs", {} );
      SetupStamps();
      SetupFuncs();
    }

    /// Create and "stamp" values to ensure inputs yield unique outputs
    void SetupStamps(){
      stamp_vec.push_back(170); // Binary: 10101010
      stamp_vec.push_back(204); // Binary: 11001100
      stamp_vec.push_back(56);  // Binary: 00111000
    }

    /// Define IO instruction and make it available to the specified population
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      const inst_func_t func_input = 
          [this](org_t& hw, const org_t::inst_t& inst){ Inst_IO(hw, inst); };
      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
          "IO", func_input);
    }

  };

  MABE_REGISTER_MODULE(VirtualCPU_Inst_IO, "IO instruction for VirtualCPUOrg");
}

#endif
