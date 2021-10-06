/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  EvalMatchBits.hpp
 *  @brief MABE Evaluation module for counting the number of bits that MATCH with another organism.
 * 
 * 
 *  DEVELOPER NOTES:
 *  - We should allow offsets, skips, etc, to do more sophisticated pairings for matches.
 */

#ifndef MABE_EVAL_MATCH_BITS_H
#define MABE_EVAL_MATCH_BITS_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalMatchBits : public Module {
  private:
    int eval_pop1 = 0;
    int eval_pop2 = 0;

    std::string bits_trait = "bits";
    std::string fitness_trait = "bit_matches";
    bool count_matches;   // =0 counts MISmatches, or =1 for count matches.

  public:
    EvalMatchBits(mabe::MABE & control,
                  const std::string & name="EvalMatchBits",
                  const std::string & desc="Evaluate bitstrings by counting ones (or zeros).")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalMatchBits() { }

    void SetupConfig() override {
      LinkPop(eval_pop1, "eval_pop1", "Population to evaluate.");
      LinkPop(eval_pop2, "eval_pop2", "Population to compare to.");

      LinkVar(bits_trait, "bits_trait", "Trait storing bit sequence to evaluate.");
      LinkVar(fitness_trait, "fitness_trait", "Trait to store fitness result.");
      LinkVar(count_matches, "count_matches", "=0 counts MISmatches, or =1 for count matches.");
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "All-ones fitness value", 0.0);
    }

    void OnUpdate(size_t /* update */) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the populations and evaluate each organism pair.
      double best_match = 0.0;
      Population & pop1 = control.GetPopulation(eval_pop1);
      Population & pop2 = control.GetPopulation(eval_pop2);

      // Loop through all organisms in the first population, matching them with the second.
      for (size_t pos = 0; pos < pop1.GetSize(); pos++) {
        // If the first population is empty, still check for organism in the second to score.
        if (pop1.IsEmpty(pos)) {
          if (pop2.IsOccupied(pos)) pop2[pos].SetTrait<double>(fitness_trait, 0.0);
          continue;  // Skip over empty cell in first population.
        }

        Organism & org = pop1[pos];

        // If there is NO corresponding organisms in pop2, return a zero match.
        double fitness = 0.0;
        if (pop2.IsOccupied(pos)) {
          // Find the corresponding organism in the compare population.
          Organism & org2 = pop2[pos];

          // Make sure both organisms have bit sequences ready for us to access.
          org.GenerateOutput();
          org2.GenerateOutput();

          // Count the number of matches in the bit sequences.
          const emp::BitVector & bits1 = org.GetTrait<emp::BitVector>(bits_trait);
          const emp::BitVector & bits2 = org2.GetTrait<emp::BitVector>(bits_trait);

          if (count_matches) {
            fitness = (double) (bits1 ^ bits2).CountZeros();
          }
          else {
            fitness = (double) (bits1 ^ bits2).CountOnes();
          }

          if (fitness > best_match) best_match = fitness;

          // Store the count on the second organism in the fitness trait.
          org2.SetTrait<double>(fitness_trait, fitness);

        }

        // Store the count on the organism in the fitness trait.
        org.SetTrait<double>(fitness_trait, fitness);

      }

      // If pop2 is bigger, make sure to mark any extra organisms as having a zero match fitness.
      for (size_t pos = pop1.GetSize(); pos < pop2.GetSize(); pos++) {
        if (pop2.IsOccupied(pos)) pop2[pos].SetTrait<double>(fitness_trait, 0.0);
      }

    }
  };

  MABE_REGISTER_MODULE(EvalMatchBits, "Evaluate bitstrings based on how well they match other organisms.");
}

#endif
