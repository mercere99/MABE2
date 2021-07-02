/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalPacking.hpp
 *  @brief MABE Evaluation module for counting the number of successful packages that have been packed.
 * 
 *  Note : a package of size p 1's (ex. p = 3, package is 111) is successfully packed if it is padded by 
 *           z 0's on both sides. 
 *           For example, if p = 3, z = 2, a successfull package would be 0011100. 
 *         Packages can have overlapping buffers. Thus with p = 3, z = 2, 001110011100 counts as two packages. 
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

    size_t num_ones = 3;
    size_t num_zeros = 2; 
    
  public:
    EvalPacking(mabe::MABE & control,
                  const std::string & name="EvalPacking",
                  const std::string & desc="Evaluate bitstrings using the Royal Road"
                  )
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
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store the fitness in?");
      LinkVar(num_ones, "num_ones", "Number of ones to be packaged together."); 
      LinkVar(num_zeros, "num_zeros", "Number of zeros to buffer each side of ones package."); 
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "Royal Road Fitness Value", 0.0);
    }

    void OnUpdate(size_t /* update */) override {

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;

      // keeps track of where we are in the construction of a package
      //    if 0: adding zeros to beginning padding 
      //    if 1: adding ones to fill package (it not preceded by full package)
      //    if 2: adding zeros to end padding
      //    if 3: complete package!
      int package_status = 0; 
        
      // keep track of our counters
      size_t zeros_counter = 0; 
      size_t ones_counter = 0; 

      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

        // Count the number of ones in the bit sequence.
        const emp::BitVector & bits = org.GetVar<emp::BitVector>(bits_trait);

        // keep track of fitness: 
        double fitness = 0.0; 
        
        for (size_t i = 0; i < bits.size(); i++) {
          
          if (package_status % 2 == 0) { //looking at a section of padding
            if (bits[i] == 0) {
              zeros_counter++; 
              if (zeros_counter == num_zeros) {
                package_status++;
                zeros_counter = 0; 
              }
            } else { zeros_counter = 0; package_status = 0; }

          } else if (package_status == 1) { //adding one's to the package
            if (bits[i] == 1) {
              ones_counter++;
              if (ones_counter == num_ones) {
                package_status++; 
                ones_counter = 0;  
              } 
            } else { package_status--; ones_counter = 0; }

          } 

          if (package_status == 3) { // completed a package!
            package_status = 1; // ie (011(0)110) if buffer = 1, brick = 2; 
            fitness += 1.0; 
          } 
        }
 
        //reset counters for next organism
        zeros_counter = 0; 
        ones_counter = 0; 
        package_status = 0; 

        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness) {
          max_fitness = fitness;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalPacking, "Evaluate bitstrings by counting ones (or zeros).");
}

#endif
