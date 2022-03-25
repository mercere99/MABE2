/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalPacking.hpp
 *  @brief MABE Evaluation module for counting the number of successful packages that have been packed.
 * 
 *  Note : A package is comprised of three sections: 
 *    1. front padding of 0's 
 *    2. a package of 1's
 *    3. back padding of 0's.
 *    A package of size p 1's (ex. p = 3, package is 111) is successfully packed if 
 *        it is padded by z 0's on both sides. 
 *      For example, if p = 3, z = 2, a successfull package would be 0011100. 
 *      Packages can have overlapping buffers. 
 *        Thus with p = 3, z = 2, 001110011100 counts as two packages. 
 *    Edge cases:
 *      - No padding is needed at the beginning of the bitstring. 
 *        - e.g., for p = 3, z = 2, 11100 counts as a complete package
 *      - Similarly, no padding is needed at the end of the genome
 *        - e.g., for p = 3, z = 2, 00111 counts as a complete package
 *        - 0111, 1110, and 111 also count as complete packages thanks to these cases
 *      - Extra padding is fine
 *        - e.g., for p = 3, z = 2, 11100000111 counts as a two complete packages
 */

#ifndef MABE_EVAL_PACKING_H
#define MABE_EVAL_PACKING_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  /// \brief Evaluation module that counts the number of packages successfully packed.
  class EvalPacking : public Module {
  protected:
    std::string bits_trait;    ///< Name of the trait containing the bitstring to evaluate
    std::string fitness_trait; ///< Name of the trait that stores the resulting fitness
    size_t package_size = 6;   ///< Number of ones expected in a package
    size_t padding_size = 3;   ///< Number of zeros expected on each side of a package

  public:
    EvalPacking(mabe::MABE & control,
        const std::string & name="EvalPacking",
        const std::string & desc="Evaluate bitstrings by counting correctly packed bricks.")
      : Module(control, name, desc) , bits_trait("bits") , fitness_trait("fitness")
    {
      SetEvaluateMod(true);
    }
    ~EvalPacking() { }

    /// Set up variables for configuration files
    void SetupConfig() override {
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", 
          "Which trait should we store package fitness in?");
      LinkVar(package_size, "package_size", "Number of ones to form a single package.");
      LinkVar(padding_size, "padding_size", 
          "Minimum nubmer of zeros to surround packages of ones.");
    }

    /// Set up the traits that will be used
    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "Packing fitness value", 0.0);
    }

    /// \brief Evaluate the fitness of an organism
    ///
    ///  \param bits a BitVector comprised of the bits_traits of an organism
    ///  \param num_zeros the number of zeros expected as padding
    ///  \param num_ones the number of ones expected as the package size
    double EvaluateOrg(const emp::BitVector& bits, size_t num_zeros, size_t num_ones) {
      // Keep track of fitness of organism
      double fitness = 0.0; 

      // Keep track of where we are in the construction of a package
      //    if 0: adding zeros to beginning padding 
      //    if 1: adding ones to fill package (it not preceded by full package)
      //    if 2: adding zeros to end padding
      //    if 3: complete package!
      // If the first bit is a 1, start in state 1 for special case where there is no padding 
      //    at the start of the bitstring 
      int package_status = ((bits[0] == 1) ? 1 : 0); 
        
      // Keep track of the number of ones and zeros in each package section
      size_t zeros_counter = 0; 
      size_t ones_counter = 0; 

      for (size_t i = 0; i < bits.size(); i++) {
        if (package_status % 2 == 0) { // Looking at a section of padding
          if (num_zeros == 0) package_status++; // Special case: no padding
          if (bits[i] == 0) {
            zeros_counter++; 
            if (zeros_counter == num_zeros) {
              package_status++;
              zeros_counter = 0; 
            }
          } 
          else{
            zeros_counter = 0; 
            package_status = 0; 
          }
        } 
        else if (package_status == 1) { // Adding ones to the package
          if (bits[i] == 1) {
            ones_counter++;
            if (ones_counter == num_ones) {
              ones_counter = 0;  
              // Check for special cases where this package is acceptable at this point
              //  1. No padding is needed
              //  2. Package ended right at the end of the bitstring
              if(num_zeros == 0 || i == bits.size() - 1) 
                package_status = 3; 
              else 
                package_status++; 
            } 
          } 
          else { 
            if(ones_counter != 0){ // Allow extra 0s before ones start, otherwise fail
              package_status--; 
              ones_counter = 0; 
            }
          }

        } 
        if (package_status == 3) { // Package completed!
          package_status = 1; // ie (011(0)110) if buffer = 1, brick = 2; 
          fitness += 1.0; 
        } 
      } // End for
 
      // Return calculated fitness for the organism (the number of packages)
      return fitness; 
    }
  
    /// Evaluate all organisms in a collection, return the max fitness
    double Evaluate(Collection orgs) {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      mabe::Collection alive_collect( orgs.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();
        // Get the bits_traits of the orgnism.
        const emp::BitVector & bits = org.GetTrait<emp::BitVector>(bits_trait);
        // Evaluate the fitness of the orgnism
        double fitness = EvaluateOrg(bits, padding_size, package_size); 
        // Set the fitness_trait for the organism
        org.SetTrait<double>(fitness_trait, fitness);
        // Update the max_fitness if applicable
        if (fitness > max_fitness) {
          max_fitness = fitness;
        }
      }
      return max_fitness;
    }

    /// Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
         [](EvalPacking & mod, Collection list) { 
           return mod.Evaluate(list); 
         },
        "Evaluate all orgs in an OrgList on the packing problem.");
    }

  };

  MABE_REGISTER_MODULE(EvalPacking, "Evaluate bitstrings by counting correctly packed packages.");
}

#endif
