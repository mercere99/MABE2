/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  EvalMatchBits.h
 *  @brief MABE Evaluation module for counting the number of bits that MATCH with another organism.
 * 
 * 
 *  DEVELOPER NOTES:
 *  - We should allow offsets, skips, etc, to do more sophisticated pairings for matches.
 */

#ifndef MABE_EVAL_COUNT_BITS_H
#define MABE_EVAL_COUNT_BITS_H

#include "../core/MABE.h"
#include "../core/Module.h"

#include "tools/reference_vector.h"

namespace mabe {

  class EvalCountBits : public Module {
  private:
    int eval_pop = 0;
    int compare_pop = 1;

    std::string bits_trait = "bits";
    std::string fitness_trait = "bit_matches";
    bool count_matches;   // =0 counts MISmatches, or =1 for count matches.

  public:
    EvalCountBits(mabe::MABE & control,
                  const std::string & name="EvalCountBits",
                  const std::string & desc="Evaluate bitstrings by counting ones (or zeros).")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalCountBits() { }

    void SetupConfig() override {
      LinkPop(eval_pop, "eval_pop", "Which population should we evaluate?");
      LinkPop(compare_pop, "compare_pop", "Which population should we compare to?");

      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
      LinkVar(count_matches, "count_matches", "=0 counts MISmatches, or =1 for count matches.");
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "All-ones fitness value", 0.0);
    }

    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the populations and evaluate each organism pair.
      double best_match = 0.0;
      Population & pop1 = control.GetPopulation(eval_pop);
      Population & pop2 = control.GetPopulation(compare_pop);

      for (size_t pos = 0; pos < pop1.GetSize(); pos++) {
        if (pop1.IsEmpty(pos)) continue;  // Skip over empty cells.

        Organism & org = pop1[pos];

        // If there is NO corresponding organisms in pop2, return a zero match.
        double fitness = 0.0;
        if (pop2.IsValid(pos) && pop2.IsOccupied(pos)) {
          // Find the corresponding organism in the compare population.
          Organism & org2 = pop2[pos];

          // Make sure both organisms have bit sequences ready for us to access.
          org.GenerateOutput();
          org2.GenerateOutput();

          // Count the number of matches in the bit sequences.
          const emp::BitVector & bits1 = org.GetVar<emp::BitVector>(bits_trait);
          const emp::BitVector & bits2 = org2.GetVar<emp::BitVector>(bits_trait);

          if (count_matches) {
            fitness = (double) (bits1 ^ bits2).CountZeros();
          }
          else {
            fitness = (double) (bits1 ^ bits2).CountOnes();
          }

          if (fitness > best_match) best_match = fitness;
        }

        // Store the count on the organism in the fitness trait.
        org.SetVar<double>(fitness_trait, fitness);

      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalMatchBits, "Evaluate bitstrings based on how well they match other organisms.");
}

#endif
