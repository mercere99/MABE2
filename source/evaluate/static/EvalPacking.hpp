/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalPacking.hpp
 *  @brief MABE Evaluation module for evaluating the royal road problem.
 * 
 *  In royal road, the number of 1s from the beginning of a bitstring are counted, but only
 *  in groups of B (brick size).
 */

#ifndef MABE_EVAL_PACKING_H
#define MABE_EVAL_PACKING_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalPacking : public Module {
  private:
    Collection target_collect;

    std::string bits_trait;
    std::string fitness_trait;

    size_t brick_size = 6;
    size_t packing_size = 3;

  public:
    EvalPacking(mabe::MABE & control,
                  const std::string & name="EvalPacking",
                  const std::string & desc="Evaluate bitstrings by counting correctly packed bricks.")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
      , bits_trait("bits")
      , fitness_trait("fitness")
    {
      SetEvaluateMod(true);
    }
    ~EvalPacking() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store Royal Road fitness in?");
      LinkVar(brick_size, "brick_size", "Number of ones to have a whole brick in the road.");
      LinkVar(packing_size, "packing_size", "Minimum nubmer of zeros to surround bricks of ones.");
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "Packing fitness value", 0.0);
    }

    size_t evaluate(size_t brick_s, size_t packing_s, const emp::BitVector bits) {
      size_t brick_size = brick_s;
      size_t packing_size = packing_s;

      if (bits.GetSize() < brick_size) {
          return 0;
      }

      size_t packed = 0; // number of correctly packed bricks

      size_t ones_count = 0;
      size_t zeros_count = 0;
      int check_step = 1; // 0 = count front packing, 1 = count brick, 2 = count back packing, 3 = all elements found
      
      for (size_t i = 0; i < bits.size(); i++) {
        if (check_step == 0 || check_step == 2) {
          if (bits[i] == 0) {
            zeros_count++;
          }
          if (zeros_count == packing_size) {
            zeros_count = 0;
            check_step++;
          }
          // one found, restart search for front packing
          else if (bits[i] == 1) {
            zeros_count = 0;
            check_step = 0;
          }
        }
        // looking for brick
        else if (check_step == 1) {
          if (bits[i] == 1) {
            ones_count++;
            // full brick found, begin looking for zeros
            if (ones_count == brick_size) {
              ones_count = 0;
              check_step++;
            }
          }
          // zero found, begin looking for front packing
          else if (bits[i] == 0) {
            ones_count = 0;
            zeros_count = 1;
            check_step = 0;
          }
        }
        if (check_step == 3) {
          packed++;
          check_step = 1;
        }
      }

      return packed;
    }

    void OnUpdate(size_t /* update */) override {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      mabe::Collection alive_collect = target_collect.GetAlive();
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

        // Count the number of ones in the bit sequence.
        const emp::BitVector & bits = org.GetVar<emp::BitVector>(bits_trait);

        size_t fitness = evaluate(brick_size, packing_size, bits);
          // Store the count on the organism in the fitness trait.

          org.SetVar<double>(fitness_trait, fitness);

          if (fitness > max_fitness) {
            max_fitness = fitness;
          }

        std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
      }
    }
  };

  MABE_REGISTER_MODULE(EvalPacking, "Evaluate bitstrings by counting correctly packed bricks.");
}

#endif

/* 
        while (i < bits.size()) {
          // reset brick and back_packing to not found
          brick_found = false;
          back_packing_found = false;
          // check for full brick
          size_t j = 0;
          for (j = 0; j < brick_size; j++) {
            if (bits[i+j] == 0) {
              brick_found = false;
              break;
            } else {
              brick_found = true;
            }
          }
          // brick not found, check if zero found makes full front_packing
          if (!brick_found && bits[i + j] == 0) {
            i += j + 1;
            // check for front_packing
            size_t k = 0;
            for (k = 0; k < packing_size; k ++) {
              if (bits[i+k] == 1) {
                front_packing_found = false;
                break;
              } else {
                front_packing_found = true;
              }
            }
            // move beyond font_packing or to first one found
            if (front_packing_found) {
              i += k + 1; 
            } else {
              i += k;
            }
            continue;
          }
          // check that no more ones follow full brick, if so front_packing is incorrect, continue
          if (brick_found && bits[i + j + 1] == 1) {
            i += j + 1;
            front_packing_found = false;
            continue;
          }
          // check for full back_packing 
          i += j + 1;
          size_t k = 0;
          for (k = 0; k < packing_size; k ++) {
            if (bits[i+k] == 1) {
              back_packing_found = false;
              break;
            } else {
              back_packing_found = true;
            }
          }
          if (front_packing_found && brick_found && back_packing_found) {
            packed++;
          }
          // back_packing of current brick is fornt_packing of next brick
          front_packing_found = back_packing_found;
          i += k + 1;
        }
*/
