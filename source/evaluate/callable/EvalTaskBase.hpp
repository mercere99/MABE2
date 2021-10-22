/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskBase.h
 *  @brief Base class to test if an organism output performed a given binary logic task
 */

#ifndef MABE_EVAL_TASK_BASE_H
#define MABE_EVAL_TASK_BASE_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"

namespace mabe {

  class EvalTaskBase : public Module {
  public:
    using data_t = VirtualCPUOrg::data_t;
  protected:
    std::string inputs_trait = "input";   ///< Name of trait for organism's inputs  (required)
    std::string outputs_trait = "output"; ///< Name of trait for organism's outputs (required)
    std::string fitness_trait = "merit";  ///< Name of trait for organism's fitness (required)
    Collection target_collect;
    int pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_task;
    std::string task_name;
    // Name of trait that tracks if task was performed (owned)
    std::string performed_trait = "unnamed_performed";  
    size_t num_args = 2;
    double reward_value = 1; 
    bool is_multiplicative = false;

  public:
    EvalTaskBase(mabe::MABE & _control,
                  const std::string & _mod_name="EvalTaskBase",
                  const std::string & _task_name = "unnamed",
                  size_t _num_args = 2,
                  const std::string & _desc="Evaluate organism on BASE logic task")
      : Module(_control, _mod_name, _desc)
      , target_collect(_control.GetPopulation(0))
      , task_name(_task_name)
      , num_args(_num_args)
      , performed_trait(_task_name + "_performed"){;}

    ~EvalTaskBase() { }

    void SetupConfig() override {
      LinkVar(inputs_trait,  "inputs_trait", "Which trait contains the organism's inputs?");
      LinkVar(outputs_trait, "outputs_trait", "Which trait contains the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait","Which trait should we increase if BASE was executed?");
      LinkVar(performed_trait, "performed_trait", "Which trait should track if BASE was executed?");
      LinkVar(reward_value, "reward_value", "How large is the reward for performing this task?");
      LinkVar(is_multiplicative, "is_multiplicative", "Should reward be multiplied (true) or added(false) to current score?");
    }
   
    virtual bool CheckOneArg(data_t& output, data_t& input){
      emp_error("Derived EvalTask class did not define CheckOneArg");
    }; 

    virtual bool CheckTwoArg(data_t& output, data_t& input_a, data_t& input_b){ 
      emp_error("Derived EvalTask class did not define CheckOneArg");
    }

    void SetupFunc(){
      ActionMap& action_map = control.GetActionMap(pop_id);

      if(num_args == 1){
        func_task = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          bool& task_performed = hw.GetTrait<bool>(performed_trait);
          if(!task_performed){
            emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(inputs_trait);
            emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(outputs_trait);
            double original_fitness = hw.GetTrait<double>(fitness_trait);
            double fitness_increase = 0;
            if(input_vec.size() > 0 && output_vec.size() > 0){
              data_t& output = *output_vec.rbegin();
              for(data_t input : input_vec){
                if( CheckOneArg(output, input) ){
                  if(is_multiplicative)
                    hw.SetTrait<double>(fitness_trait, original_fitness * reward_value);
                  else
                    hw.SetTrait<double>(fitness_trait, original_fitness + reward_value);
                  task_performed = true;
                  break;
                }
              }
            }
          }
        };
      }
      else if(num_args == 2){
        func_task = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          bool& task_performed = hw.GetTrait<bool>(performed_trait);
          if(!task_performed){
            emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(inputs_trait);
            emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(outputs_trait);
            double original_fitness = hw.GetTrait<double>(fitness_trait);
            double fitness_increase = 0;
            if(input_vec.size() > 0 && output_vec.size() > 0){
              data_t& output = *output_vec.rbegin();
              for(size_t idx_a = 0; idx_a < input_vec.size() - 1; idx_a++){
                for(size_t idx_b = idx_a + 1; idx_b < input_vec.size(); idx_b++){
                  if( CheckTwoArg(output, input_vec[idx_a], input_vec[idx_b]) ){
                    if(is_multiplicative)
                      hw.SetTrait<double>(fitness_trait, original_fitness * reward_value);
                    else
                      hw.SetTrait<double>(fitness_trait, original_fitness + reward_value);
                    task_performed = true;
                    break;
                  }
                }
              }
            }
          }
        };
      }
      else{ 
        emp_error("EvalTaskBase can currently only handle tasks involving one or two arguments");
      }

      action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("IO", func_task);
    }

    void SetupModule() override {
      AddRequiredTrait<emp::vector<data_t>>(inputs_trait);
      AddRequiredTrait<emp::vector<data_t>>(outputs_trait);
      AddRequiredTrait<double>(fitness_trait);
      AddOwnedTrait<bool>(performed_trait, "Was the task performed?", false);
      SetupFunc();
    }
  };

}

#endif
