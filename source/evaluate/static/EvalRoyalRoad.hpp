/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalRoyalRoad.hpp
<<<<<<< HEAD
 *  @brief MABE Evaluation module for evaluating the royal road problem.
 * 
 *  In royal road, the number of 1s from the beginning of a bitstring are counted, but only
 *  in groups of B (brick size).
=======
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
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

<<<<<<< HEAD
    size_t brick_size = 8;
    double extra_bit_cost = 0.5;

  public:
    EvalRoyalRoad(mabe::MABE & control,
                  const std::string & name="EvalRoyalRoad",
                  const std::string & desc="Evaluate bitstrings by counting number of bricks (or zeros).")
=======
    size_t brick_size = 8; 
    
  public:
    EvalRoyalRoad(mabe::MABE & control,
                  const std::string & name="EvalRoyalRoad",
                  const std::string & desc="Evaluate bitstrings using the Royal Road"
                  )
>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
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
<<<<<<< HEAD
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store Royal Road fitness in?");
      LinkVar(brick_size, "brick_size", "Number of ones to have a whole brick in the road.");
      LinkVar(extra_bit_cost, "extra_bit_cost", "Penalty per-bit for extra-long roads.");
=======
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store the fitness in?");
      LinkVar(brick_size, "brick_size", "Size of brick that we are using to build the road"); 
>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
<<<<<<< HEAD
      AddOwnedTrait<double>(fitness_trait, "Royal Road fitness value", 0.0);
    }

    void OnUpdate(size_t /* update */) override {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      mabe::Collection alive_collect = target_collect.GetAlive();
=======
      AddOwnedTrait<double>(fitness_trait, "Royal Road Fitness Value", 0.0);
    }

    void OnUpdate(size_t /* update */) override {

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      mabe::Collection alive_collect( target_collect.GetAlive() );
>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

<<<<<<< HEAD
        // Count the number of ones in the bit sequence.
        const emp::BitVector & bits = org.GetVar<emp::BitVector>(bits_trait);
        int road_length = 0.0;
        for (size_t i = 0; i < bits.size(); i++) {
          if (bits[i] == 0) break;
          road_length++;
        }

        const int overage = road_length % brick_size;

        // Store the count on the organism in the fitness trait.
        double fitness = road_length - overage * (extra_bit_cost + 1.0);
=======
        const emp::BitVector & bits = org.GetVar<emp::BitVector>(bits_trait); 

        // size of successfull road built
        int road_length = 0; 
        // Make the brick road
        for (size_t i = 0; i < bits.size(); i++) {
          if (bits[i] == 0) break; 
          road_length++; 

        }

        // Count number of bits in an incomplete brick
        const int bits_of_incomplete_brick = road_length % brick_size; 
        
        // Fitness is the length of full bricks in the "road"
        double fitness = road_length - bits_of_incomplete_brick; 
        
        // Store the count on the organism in the fitness trait.
>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness) {
          max_fitness = fitness;
        }
<<<<<<< HEAD
=======

>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalRoyalRoad, "Evaluate bitstrings by counting ones (or zeros).");
}

<<<<<<< HEAD
#endif
=======
#endif
>>>>>>> 8abdd5ce85047abf742e14caf5219383fcaa9e1f
