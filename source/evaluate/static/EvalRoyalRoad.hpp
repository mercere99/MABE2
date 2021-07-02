/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalRoyalRoad.hpp
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
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
    
  public:
    EvalRoyalRoad(mabe::MABE & control,
                  const std::string & name="EvalRoyalRoad",
                  const std::string & desc="Evaluate bitstrings using the Royal Road"
                  )
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
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store the fitness in?");
      LinkVar(brick_size, "brick_size", "Size of brick that we are using to build the road"); 
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "Royal Road Fitness Value", 0.0);
    }

    void OnUpdate(size_t /* update */) override {

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

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
        org.SetVar<double>(fitness_trait, fitness);

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