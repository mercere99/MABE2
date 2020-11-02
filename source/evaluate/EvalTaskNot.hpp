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

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class EvalTaskNot : public Module {
  private:
    std::string inputs_trait;  ///< Name of trait for organism's inputs  (required)
    std::string outputs_trait; ///< Name of trait for organism's outputs (required)
    std::string fitness_trait; ///< Name of trait for organism's fitness (owned)

  public:
    EvalTaskNot(mabe::MABE & control,
                  const std::string & name="EvalTaskNot",
                  const std::string & desc="Evaluate organism on NOT via triggers",
                  const std::string & _itrait="inputs",
                  const std::string & _otrait="outputs",
                  const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , inputs_trait(_itrait)
      , outputs_trait(_otrait)
      , fitness_trait(_ftrait)
    { }
    ~EvalTaskNot() { }

    void SetupConfig() override {
      LinkVar(outputs_trait, "inputs_trait", "Which trait contains the organism's inputs?");
      LinkVar(outputs_trait, "outputs_trait", "Which trait contains the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
    }

    void SetupModule() override {
      AddRequiredTrait<std::unordered_map<int,double>>(inputs_trait);
      AddRequiredTrait<std::unordered_map<int,double>>(outputs_trait);
      AddOwnedTrait<double>(fitness_trait, "Organism's fitness value", 1.0);
    }

    /// Test to see if organism performed the NOT operation
    void OnManualEval(Organism& org) override {
        const std::unordered_map<int,double> & inputs = 
            org.GetVar<std::unordered_map<int,double>>(inputs_trait);
        const std::unordered_map<int,double> & outputs = 
            org.GetVar<std::unordered_map<int,double>>(outputs_trait);
        if(inputs.size() > 0 && outputs.size() > 0){
          // TODO: Modify AvidaGPOrg so that it's not using doubles everywhere
          const uint64_t input = static_cast<uint64_t>(inputs.begin()->second);
          const uint64_t output = static_cast<uint64_t>(outputs.begin()->second);
          if(output == ~input){
            org.SetVar<double>(fitness_trait, 2);
            std::cout << "NOT performed!" << std::endl;
            exit(0);
          }
          else
            org.SetVar<double>(fitness_trait, 1);
        }
        else
          org.SetVar<double>(fitness_trait, 1);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskNot, "Manual evaluation of organism for NOT operation");

}

#endif
