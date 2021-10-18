/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskNand.h
 *  @brief Tests organism output for NAND operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_NAND_H
#define MABE_EVAL_TASK_NAND_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"

namespace mabe {

  class EvalTaskNand : public Module {
  public:
    using data_t = VirtualCPUOrg::data_t;
  private:
    std::string inputs_trait = "input";   ///< Name of trait for organism's inputs  (required)
    std::string outputs_trait = "output"; ///< Name of trait for organism's outputs (required)
    std::string fitness_trait = "merit";  ///< Name of trait for organism's fitness (owned)
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_task_nand;

  public:
    EvalTaskNand(mabe::MABE & control,
                  const std::string & name="EvalTaskNand",
                  const std::string & desc="Evaluate organism on NAND logic task")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}

    ~EvalTaskNand() { }

    void SetupConfig() override {
      LinkVar(inputs_trait,  "inputs_trait", "Which trait contains the organism's inputs?");
      LinkVar(outputs_trait, "outputs_trait", "Which trait contains the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we increase if NAND was executed?");
    }
    
    void SetupFunc(){
      ActionMap& action_map = control.GetActionMap(pop_id);

      func_task_nand = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
        bool& nand_performed = hw.GetTrait<bool>("nand_performed");
        if(!nand_performed){
          emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(inputs_trait);
          emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(outputs_trait);
          double original_fitness = hw.GetTrait<double>(fitness_trait);
          double fitness_increase = 0;
          if(input_vec.size() > 0 && output_vec.size() > 0){
            data_t& output = *output_vec.rbegin();
            for(size_t idx_a = 0; idx_a < input_vec.size() - 1; idx_a++){
              for(size_t idx_b = idx_a + 1; idx_b < input_vec.size(); idx_b++){
                if(output == ~(input_vec[idx_a] & input_vec[idx_b])){
                  fitness_increase += 1;
                  nand_performed = true;
                  break;
                }
              }
            }
          }
          hw.SetTrait<double>(fitness_trait, original_fitness + fitness_increase);
        }
      };

      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("IO", func_task_nand);
    }

    void SetupModule() override {
      AddRequiredTrait<emp::vector<data_t>>(inputs_trait);
      AddRequiredTrait<emp::vector<data_t>>(outputs_trait);
      AddRequiredTrait<double>(fitness_trait);
      AddOwnedTrait<bool>("nand_performed", "Was NAND performed?", false);
      SetupFunc();
    }
  };

  MABE_REGISTER_MODULE(EvalTaskNand, "Manual evaluation of organism for NAND operation");

}

#endif
