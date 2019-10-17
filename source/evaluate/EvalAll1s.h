/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  EvalAll1s.h
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_ALL_1_H
#define MABE_EVAL_ALL_1_H

#include "../core/MABE.h"
#include "../core/Module.h"

#include "tools/reference_vector.h"

namespace mabe {

  class EvalAll1s : public Module {
  private:
    int target_pop;

    std::string bits_trait;
    std::string fitness_trait;

  public:
    EvalAll1s(mabe::MABE & control, const std::string & _btrait="BITS", const std::string & _ftrait="fitness")
      : Module(control, "EvalAll1s", "Module to evaluate bitstrings on an all-ones Fitness Lanscape")
      , target_pop(0), bits_trait(_btrait), fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
      AddOwnedTrait<emp::BitVector>(bits_trait, "Bit Sequence", emp::BitVector());
      AddOwnedTrait<double>(fitness_trait, "All-ones fitness value", 0.0);
      SetMinPops(1);
    }
    ~EvalAll1s() { }

    void SetupConfig() override {
      LinkPop(target_pop, "target_pop", "Which population should we evaluate?", 0);
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?", "bits");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?", "fitness");
    }

    void SetupModule() override {
    }

    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      for (Organism & org : control.GetPopulation(0).Alive()) {
        org.GenerateOutput(bits_trait);
        double fitness = (double)  org.GetVar<emp::BitVector>(bits_trait).CountOnes();
        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalAll1s, "Evaluate bitstrings on an all-ones fitness lanscape.");
}

#endif
