/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EvalNK.hpp
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_NK_H
#define MABE_EVAL_NK_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../tools/NK.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalNK : public Module {
  private:
    size_t N;
    size_t K;    
    NKLandscape landscape;
    mabe::Collection target_collect;

    std::string bits_trait;
    std::string fitness_trait;

  public:
    EvalNK(mabe::MABE & control,
           const std::string & name="EvalNK",
           const std::string & desc="Module to evaluate bitstrings on an NK Fitness Lanscape",
           size_t _N=100, size_t _K=3, const std::string & _btrait="bits", const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , N(_N), K(_K)
      , target_collect(control.GetPopulation(0))
      , bits_trait(_btrait)
      , fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalNK() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(N, "N", "Number of bits required in output");
      LinkVar(K, "K", "Number of bits used in each gene");
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "NK fitness value", 0.0);

      // Setup the fitness landscape.
      landscape.Config(N, K, control.GetRandom());  // Setup the fitness landscape.
    }

    void OnUpdate(size_t /* update */) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {
        org.GenerateOutput();
        const auto & bits = org.GetTrait<emp::BitVector>(bits_trait);
        if (bits.size() != N) {
          AddError("Org returns ", bits.size(), " bits, but ",
                   N, " bits needed for NK landscape.",
                   "\nOrg: ", org.ToString());
        }
        double fitness = landscape.GetFitness(bits);
        org.SetTrait<double>(fitness_trait, fitness);

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
