/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalRoyalRoad.hpp
 *  @brief MABE Evaluation module for evaluating the royal road problem.
 * 
 *  In royal road, the number of 1s from the beginning of a bitstring are counted, but only
 *  in groups of B (brick size).
 */

#ifndef MABE_EVAL_ROYAL_ROAD_H
#define MABE_EVAL_ROYAL_ROAD_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalRoyalRoad : public Module {
  private:
    Collection target_collect;

    std::string bits_trait;
    std::string fitness_trait;

    size_t brick_size = 8;
    double extra_bit_cost = 0.5;

  public:
    EvalRoyalRoad(mabe::MABE & control,
                  const std::string & name="EvalRoyalRoad",
                  const std::string & desc="Evaluate bitstrings by counting ones (or zeros).")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
      , bits_trait("bits")
      , fitness_trait("fitness")
    {
      SetEvaluateMod(true);
    }
    ~EvalRoyalRoad() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store Royal Road fitness in?");
      LinkVar(brick_size, "brick_size", "Number of ones to have a whole brick in the road.");
      LinkVar(extra_bit_cost, "extra_bit_cost", "Penalty per-bit for extra-long roads.");
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "Royal Road fitness value", 0.0);
    }

    void OnUpdate(size_t /* update */) override {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      mabe::Collection alive_collect = target_collect.GetAlive();
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

        // Count the number of ones in the bit sequence.
        const emp::BitVector & bits = org.GetTrait<emp::BitVector>(bits_trait);
        int road_length = 0.0;
        for (size_t i = 0; i < bits.size(); i++) {
          if (bits[i] == 0) break;
          road_length++;
        }

        const int overage = road_length % brick_size;

        // Store the count on the organism in the fitness trait.
        double fitness = road_length - overage * (extra_bit_cost + 1.0);
        org.SetTrait<double>(fitness_trait, fitness);

        if (fitness > max_fitness) {
          max_fitness = fitness;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalRoyalRoad, "Evaluate bitstrings by counting ones (or zeros).");
}

#endif
