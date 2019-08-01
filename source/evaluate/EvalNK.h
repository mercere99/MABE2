/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
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
    size_t target_pop;

    std::string bits_trait;
    std::string fitness_trait;

  public:
    EvalNK(size_t _N, size_t _K, const std::string & _btrait="NK", const std::string & _ftrait="fitness")
      : Module("EvalNK", "Module to evaluate bitstrings on an NK Fitness Lanscape")
      , N(_N), K(_K), target_pop(0), bits_trait(_btrait), fitness_trait(_ftrait)
    {
      IsEvaluate(true);
      AddOwnedTrait<emp::BitVector>(bits_trait, "NK Bit Sequence", emp::BitVector());
      AddOwnedTrait<double>(fitness_trait, "NK fitness value", 0.0);
      SetMinPops(1);
    }
    ~EvalNK() { }

    void SetupConfig(ConfigScope & config_scope) override {
      config_scope.LinkVar(N, "N", "Number of bits required in output", 20);
      config_scope.LinkVar(K, "K", "Number of bits used in each gene", 3);
    }

    void Setup(mabe::MABE & control) {
      landscape.Config(N, K, control.GetRandom());  // Setup the fitness landscape.
    }

    void Update(mabe::MABE & control) {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      for (Organism & org : control.GetPopulation(0).Alive()) {
        org.GenerateOutput(bits_trait);
        double fitness = landscape.GetFitness( org.GetVar<emp::BitVector>(bits_trait) );
        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

}

#endif
