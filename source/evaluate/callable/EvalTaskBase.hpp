/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskBase.h
 *  @brief Generic base class for evaluating an organism on a binary logic task. 
 */

#ifndef MABE_EVAL_TASK_BASE_H
#define MABE_EVAL_TASK_BASE_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"

namespace mabe {

  /// \brief Generic base class for evaluating an organism on a binary logic task. 
  template <typename DERIVED, size_t NUM_ARGS>
  class EvalTaskBase : public Module {
  public:
    using derived_t = DERIVED;
    using org_t = VirtualCPUOrg;
    using data_t = org_t::data_t;
    using inst_func_t = org_t::inst_func_t;

    enum RewardType{
      ADD,   // Additive. New merit = old merit + reward
      MULT,  // Multiplicative. New merit = old merit * reward
      POW    // Power. New merit = old merit * (2 ^ reward)
    };

  protected:
    std::string inputs_trait = "input";   ///< Name of trait for organism's inputs  (required)
    std::string outputs_trait = "output"; ///< Name of trait for organism's outputs (required)
    std::string fitness_trait = "merit";  ///< Name of trait for organism's fitness (required)
    int pop_id = 0;                       ///< ID of the population to be evaluated
    std::string task_name;                ///< Name of the task (used to derive trait names)
    std::string performed_trait = "unnamed_performed"; /**< Name of the trait tracking if the
                                                            trait was performed */
    double reward_value = 1;          ///< Magnitude of the reward bestowed for completion of the task 
    RewardType reward_type = ADD; /// How do we apply the reward to the organism's merit?

  public:
    EvalTaskBase(mabe::MABE & _control,
                  const std::string & _mod_name="EvalTaskBase",
                  const std::string & _task_name = "unnamed",
                  const std::string & _desc="Evaluate organism on BASE logic task")
      : Module(_control, _mod_name, _desc)
      , task_name(_task_name)
      , performed_trait(_task_name + "_performed"){;}

    ~EvalTaskBase() { }

    /// Set up configuration variables
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", 
          "Population to evaluate.");
      LinkVar(inputs_trait,  "inputs_trait", 
          "Which trait contains the organism's inputs?");
      LinkVar(outputs_trait, "outputs_trait", 
          "Which trait contains the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait",
          "Which trait should we increase if BASE was executed?");
      LinkVar(performed_trait, "performed_trait", 
          "Which trait should track if BASE was executed?");
      LinkVar(reward_value, "reward_value", 
          "How large is the reward for performing this task?");
      LinkMenu(reward_type, "reward_type", "How to apply the reward to the organism's merit?",
               ADD,         "add",         "Additive. New merit = old merit + reward",
               MULT,        "mult",        "Multiplicative. New merit = old merit * reward",
               POW,         "pow",         "Power. New merit = old merit * (2 ^ reward)"
      );
    }
  
    /** Determine if output is the result of applying the given function to input
        To be overridden by derived class. */
    virtual bool CheckOneArg(const data_t& /*output*/, const data_t& /*input*/){
      emp_error("Derived EvalTask class did not define CheckOneArg");
      return false;
    }; 

    /** Determine if input_a OP input_b = output (with OP depending on the derived class)
        To be overridden by derived class. */
    virtual bool CheckTwoArg(const data_t& /*output*/, const data_t& /*input_a*/, 
        const data_t& /*input_b*/){ 
      emp_error("Derived EvalTask class did not define CheckOneArg");
      return false;
    }

    /// Evaluate an organism on the given logic task (assuming only one argument is needed)
    bool EvaluateOneArg(Organism& hw){
      bool& task_performed = hw.GetTrait<bool>(performed_trait);
      if(!task_performed){ // Only do check if org hasn't already performed the task
        emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(inputs_trait);
        emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(outputs_trait);
        double original_fitness = hw.GetTrait<double>(fitness_trait);
        if(input_vec.size() > 0 && output_vec.size() > 0){
          data_t& output = *output_vec.rbegin(); // Check latest output
          for(data_t input : input_vec){ // Must check against all inputs
            if( CheckOneArg(output, input) ){ // Unary check
              switch(reward_type){
                case ADD:
                  hw.SetTrait<double>(fitness_trait, original_fitness + reward_value);
                  break;
                case MULT:
                  hw.SetTrait<double>(fitness_trait, original_fitness * reward_value);
                  break;
                case POW: // new = old * (2 ^ power)
                  hw.SetTrait<double>(fitness_trait, 
                      original_fitness * std::pow(2.0, reward_value));
                  break;
              }
              task_performed = true;
              return true;
            }
          }
        }
      }
      return task_performed;
    } 

    /// Evaluate an organism on the given logic task (assuming two arguments are needed)
    bool EvaluateTwoArg(Organism& hw){
      bool& task_performed = hw.GetTrait<bool>(performed_trait);
      if(!task_performed){ // Only do check if org hasn't already performed the task
        emp::vector<data_t>& input_vec = hw.GetTrait<emp::vector<data_t>>(inputs_trait);
        emp::vector<data_t>& output_vec = hw.GetTrait<emp::vector<data_t>>(outputs_trait);
        double original_fitness = hw.GetTrait<double>(fitness_trait);
        if(input_vec.size() > 1 && output_vec.size() > 0){
          data_t& output = *output_vec.rbegin(); // Fetch latest output
          // Must check all possible pairs of input values
          for(size_t idx_a = 0; idx_a < input_vec.size() - 1; idx_a++){
            for(size_t idx_b = idx_a + 1; idx_b < input_vec.size(); idx_b++){
              if( CheckTwoArg(output, input_vec[idx_a], input_vec[idx_b]) ){ // Binary check
                switch(reward_type){
                  case ADD:
                    hw.SetTrait<double>(fitness_trait, original_fitness + reward_value);
                    break;
                  case MULT:
                    hw.SetTrait<double>(fitness_trait, original_fitness * reward_value);
                    break;
                  case POW: // new = old * (2 ^ power)
                    hw.SetTrait<double>(fitness_trait, 
                        original_fitness * std::pow(2.0, reward_value));
                    break;
                }
                task_performed = true;
                return true;
              }
            }
          }
        }
      }
      return task_performed;
    }

    /// Evaluate all organisms in the collection
    double EvaluateCollection(Collection & orgs) {
      if constexpr (NUM_ARGS == 1){
        for (Organism & org : orgs) EvaluateOneArg(org);
      }
      else if constexpr(NUM_ARGS == 2){
        for (Organism & org : orgs) EvaluateTwoArg(org);
      }
      return 0;
    }

    /// Registers the evaluation function in the ActionMap so it can be used by organisms
    void SetupFunc(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Create a function that checks either a unary or binary function 
      inst_func_t func_task;
      if constexpr(NUM_ARGS == 1){ // Unary function
        func_task = [this](org_t& hw, const org_t::inst_t& /*inst*/){ EvaluateOneArg(hw); };
      }
      else if constexpr (NUM_ARGS == 2){ // Binary function
        func_task = [this](org_t& hw, const org_t::inst_t& /*inst*/){ EvaluateTwoArg(hw); };
      }
      else{ 
        emp_error("EvalTaskBase can currently only handle tasks with one or two arguments");
      }
      // Add function to action map (can either be unary or binary at this point)
      action_map.AddFunc<void, org_t&, const org_t::inst_t&>("IO", func_task);
    }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
          [](derived_t& mod, Collection list) { 
            return mod.EvaluateCollection(list); 
          },
          "Evaluate all orgs in OrgList on a logic task");
    }

    /// Set up traits
    void SetupModule() override {
      AddRequiredTrait<emp::vector<data_t>>(inputs_trait);
      AddRequiredTrait<emp::vector<data_t>>(outputs_trait);
      AddRequiredTrait<double>(fitness_trait);
      AddOwnedTrait<bool>(performed_trait, "Was the task performed?", false);
      SetupFunc();
    }

    /// When a new organism is placed, set "task performed" trait to false
    void OnPlacement(OrgPosition placement_pos) override{
      Population & pop = placement_pos.Pop();
      size_t org_idx = placement_pos.Pos();
      pop[org_idx].SetTrait<bool>(performed_trait, false);
    }
  };

}

#endif
