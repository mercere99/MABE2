/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskNot.h
 *  @brief This is JUST A TEST (should be deleted later) to see test task eval via triggers
 */

#ifndef MABE_EVAL_TASK_NOT_H
#define MABE_EVAL_TASK_NOT_H

#include "../core/MABE.h"
#include "../core/Module.h"

#include "tools/reference_vector.h"

namespace mabe {

  class EvalTaskNot : public Module {
  private:
    Collection target_collect;
    std::string inputs_trait;
    std::string outputs_trait;
    std::string fitness_trait;

  public:
    EvalTaskNot(mabe::MABE & control,
                  const std::string & name="EvalTaskNot",
                  const std::string & desc="{TEST} Evaluate organism on NOT via triggers",
                  const std::string & _itrait="inputs",
                  const std::string & _otrait="outputs",
                  const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
      , inputs_trait(_itrait)
      , outputs_trait(_otrait)
      , fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalTaskNot() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(outputs_trait, "inputs_trait", "Which trait containts the organism's inputs?");
      LinkVar(outputs_trait, "outputs_trait", "Which trait containts the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
    }

    void SetupModule() override {
      //AddRequiredTrait<std::unordered_map<int,double>>(inputs_trait);
      AddRequiredTrait<std::unordered_map<int,double>>(outputs_trait);
      AddOwnedTrait<double>(fitness_trait, "All-ones fitness value", 1.0);
    }

    void OnManualEval(Organism& org) override {
        // Count the number of ones in the bit sequence.
//        const std::unordered_map<int,double> & inputs = org.GetVar<std::unordered_map<int,double>>(inputs_trait);
        const std::unordered_map<int,double> & outputs = org.GetVar<std::unordered_map<int,double>>(outputs_trait);
        if(outputs.size() > 0){
          double fitness = outputs.begin()->second;
          //std::cout << "Analyzing: " << fitness << std::endl;
          // Store the in the fitness trait.
          org.SetVar<double>(fitness_trait, fitness > 1 ? fitness : 1);
        }
        else
          org.SetVar<double>(fitness_trait, 1);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskNot, "Test module for evaluating tasks via triggers");
}

#endif
