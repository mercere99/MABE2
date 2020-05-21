/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalNK.h
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_NK_H
#define MABE_EVAL_NK_H

#include "../core/MABE.h"
#include "../core/Module.h"
#include "../tools/NK.h"

#include "tools/reference_vector.h"

namespace mabe {

  class EvalNK : public Module {
  private:
    size_t N;
    size_t K;    
    NKLandscape landscape;
    int target_pop;

    std::string bits_trait;
    std::string fitness_trait;

  public:
    EvalNK(mabe::MABE & control,
           const std::string & name="EvalNK",
           const std::string & desc="Module to evaluate bitstrings on an NK Fitness Lanscape",
           size_t _N=100, size_t _K=3, const std::string & _btrait="bits", const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , N(_N), K(_K), target_pop(0), bits_trait(_btrait), fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "NK fitness value", 0.0);
      SetMinPops(1);
    }
    ~EvalNK() { }

    void SetupConfig() override {
      LinkPop(target_pop, "target_pop", "Which population should we evaluate?");
      LinkVar(N, "N", "Number of bits required in output");
      LinkVar(K, "K", "Number of bits used in each gene");
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
    }

    void SetupModule() override {
      landscape.Config(N, K, control.GetRandom());  // Setup the fitness landscape.
    }

    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      for (Organism & org : control.GetPopulation(target_pop).Alive()) {
        org.GenerateOutput();
        const auto & bits = org.GetVar<emp::BitVector>(bits_trait);
        if (bits.size() != N) {
          AddError("Org returns ", bits.size(), " bits, but ",
                   N, " bits needed for NK landscape.",
                   "\nOrg: ", org.ToString());
        }
        double fitness = landscape.GetFitness(bits);
        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalNK, "Evaluate bitstrings on an NK fitness lanscape.");
}

#endif
