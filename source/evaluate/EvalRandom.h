/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalRandom.h
 *  @brief Gives each organism a random score between 0 and a specified max
 */

#ifndef MABE_EVAL_RANDOM_H
#define MABE_EVAL_RANDOM_H

#include "../core/MABE.h"
#include "../core/Module.h"
#include "../tools/NK.h"

#include "tools/reference_vector.h"

namespace mabe {

  class EvalRandom : public Module {
  private:
    double max_score;
    mabe::Collection target_collect;

    std::string output_trait;

  public:
    EvalRandom(mabe::MABE & control,
           const std::string & name="EvalRandom",
           const std::string & desc=
              "Gives each organism a random score between 0 and a specified max",
           double _max_score=1000, const std::string & _trait="fitness")
      : Module(control, name, desc)
      , max_score(_max_score) 
      , target_collect(control.GetPopulation(0))
      , output_trait(_trait)
    {
      SetEvaluateMod(true);
    }
    ~EvalRandom() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(max_score, "max_score", "Maximum value for the random scores");
      LinkVar(output_trait, "output_trait", "Which trait should we store the random score in?");
    }

    void SetupModule() override {
      // Setup the traits.
      AddOwnedTrait<double>(output_trait, "Random fitness value", 0.0);
    }

    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);
      emp::Random & random = control.GetRandom();
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {
        double fitness = random.GetDouble() * max_score;
        org.SetVar<double>(output_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << output_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalRandom, 
      "Gives each organism a random score between 0 and a specified max");
}

#endif
