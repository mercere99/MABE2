/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskNot.h
 *  @brief Tests organism output for NOT operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_NOT_H
#define MABE_EVAL_TASK_NOT_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"

namespace mabe {

  class EvalTaskNot : public Module {
  public:
    using data_t = VirtualCPUOrg::data_t;
  private:
    std::string inputs_trait = "input";   ///< Name of trait for organism's inputs  (required)
    std::string outputs_trait = "output"; ///< Name of trait for organism's outputs (required)
    std::string fitness_trait = "merit";  ///< Name of trait for organism's fitness (owned)
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_task_not;

  public:
    EvalTaskNot(mabe::MABE & control,
                  const std::string & name="EvalTaskNot",
                  const std::string & desc="Evaluate organism on NOT logic task")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(1),control.GetPopulation(0)){;}

    ~EvalTaskNot() { }

    void SetupConfig() override {
      LinkVar(inputs_trait,  "inputs_trait", "Which trait contains the organism's inputs?");
      LinkVar(outputs_trait, "outputs_trait", "Which trait contains the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we increase if NOT was executed?");
    }
    
    void SetupFunc(){
      ActionMap& action_map = control.GetActionMap(pop_id);

      func_task_not = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
        bool& not_performed = hw.GetTrait<bool>("not_performed");
        if(!not_performed){
          emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(inputs_trait);
          emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(outputs_trait);
          double original_fitness = hw.GetTrait<double>(fitness_trait);
          double fitness_increase = 0;
          if(input_vec.size() > 0 && output_vec.size() > 0){
            data_t& output = *output_vec.rbegin();
            for(data_t input : input_vec){
              //std::cout << output << " =?= " << ~input << std::endl;
              if(output == ~input){
                //std::cout << "NOT performed!" << std::endl;
                fitness_increase += 1;
                not_performed = true;
                break;
              }
            }
          }
          hw.SetTrait<double>(fitness_trait, original_fitness + fitness_increase);
        }
      };

      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("IO", func_task_not);
    }

    void SetupModule() override {
      AddRequiredTrait<emp::vector<data_t>>(inputs_trait);
      AddRequiredTrait<emp::vector<data_t>>(outputs_trait);
      AddRequiredTrait<double>(fitness_trait);
      AddOwnedTrait<bool>("not_performed", "Was not performed?", false);
      SetupFunc();
    }
  };

  MABE_REGISTER_MODULE(EvalTaskNot, "Manual evaluation of organism for NOT operation");

}

#endif
