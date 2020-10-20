/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  AvidaGP.h
 *  @brief Genetic programming-focused Avida organism. 
 *  @note Status: ALPHA
 */

#ifndef MABE_AVIDA_GP_ORGANISM_H
#define MABE_AVIDA_GP_ORGANISM_H

#include "../core/MABE.h"
#include "../core/Organism.h"
#include "../core/OrganismManager.h"

#include "tools/BitVector.h"
#include "tools/Distribution.h"
#include "tools/random_utils.h"
#include "hardware/AvidaGP.h"

namespace mabe {

  class AvidaGPOrg : public OrganismTemplate<AvidaGPOrg> {
  protected:
    emp::AvidaGP cpu;

  public:
    AvidaGPOrg(OrganismManager<AvidaGPOrg> & _manager)
      : OrganismTemplate<AvidaGPOrg>(_manager){ }
    AvidaGPOrg(const AvidaGPOrg &) = default;
    AvidaGPOrg(AvidaGPOrg &&) = default;
    ~AvidaGPOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of each bit mutating on reproduction.
      std::string output_name = "bits";  ///< Name of trait that should be used to access bits.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites. 
      size_t base_size = 10;             ///< Default number of instructions in a random organism
    };

    /// Use "to_string" to convert.
    std::string ToString() override { return "GP"; }

    size_t Mutate(emp::Random & random) override {
      size_t num_muts = random.GetUInt(4);
        for (uint32_t m = 0; m < num_muts; m++) {
          const uint32_t pos = random.GetUInt(cpu.genome.sequence.size());
          cpu.RandomizeInst(pos, random);
        }
        return num_muts;
    }

    void Randomize(emp::Random & random) override {
      cpu.PushRandom(random, SharedData().base_size);
    }

    /// Put the bits in the correct output position.
    void GenerateOutput() override {
    }
    
    /// Execute as single instruction
    bool ProcessStep() override{ 
      cpu.Process(1);
      return false; 
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each bit mutating on reproduction.");
      GetManager().LinkVar(SharedData().base_size, "base_size",
                      "Number of instructions in randomly created organisms");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain bit sequence.");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
    }
  };

  MABE_REGISTER_ORG_TYPE(AvidaGPOrg, "Genetic programming-focused Avida digital organism");
}

#endif
