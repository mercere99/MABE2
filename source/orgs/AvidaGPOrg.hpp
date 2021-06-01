/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  AvidaGPOrg.hpp
 *  @brief An organism consisting of lineaer code.
 *  @note Status: ALPHA
 */

#ifndef MABE_AVIDA_GP_ORGANISM_H
#define MABE_AVIDA_GP_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/hardware/AvidaGP.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class AvidaGPOrg : public OrganismTemplate<AvidaGPOrg> {
  protected:
    emp::AvidaGP hardware;

  public:
    AvidaGPOrg(OrganismManager<AvidaGPOrg> & _manager)
      : OrganismTemplate<AvidaGPOrg>(_manager) { }
    AvidaGPOrg(const AvidaGPOrg &) = default;
    AvidaGPOrg(AvidaGPOrg &&) = default;
    ~AvidaGPOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      // Configuration variables
      double mut_prob = 0.01;              ///< Probability of each bit mutating on reproduction.
      size_t init_length = 100;            ///< Length of new organisms.
      bool init_random = true;             ///< Should we randomize ancestor?  (false = all zeros)
      size_t eval_time = 500;              ///< How long should the CPU be given on each evaluate?
      std::string output_name = "output";  ///< Name of trait that should be used to access bits.

      // Internal use
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;            ///< A pre-allocated vector for mutation sites. 
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { return hardware.ToString(); }

    size_t Mutate(emp::Random & random) override {
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);

      if (num_muts == 0) return 0;
      if (num_muts == 1) {
        const size_t pos = random.GetUInt(hardware.GetSize());
        hardware.RandomizeInst(pos, random);
        return 1;
      }

      // Only remaining option is num_muts > 1.
      auto & mut_sites = SharedData().mut_sites;
      mut_sites.Clear();
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(hardware.GetSize());
        if (mut_sites[pos]) { --i; continue; }  // Duplicate position; try again.
        hardware.RandomizeInst(pos, random);
      }

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      for (size_t pos = 0; pos < hardware.GetSize(); pos++) {
        hardware.RandomizeInst(pos, random);
      }
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
    }

    /// Put the bits in the correct output position.
    void GenerateOutput() override {
      SetVar<emp::BitVector>(SharedData().output_name, bits);
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkFuns<size_t>([this](){ return bits.size(); },
                       [this](const size_t & N){ return bits.Resize(N); },
                       "N", "Number of bits in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each bit mutating on reproduction.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain bit sequence.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = all zeros)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(bits.size());

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Bitset output from organism.",
                                  emp::BitVector(0));
    }
  };

  MABE_REGISTER_ORG_TYPE(AvidaGPOrg, "Organism consisting of a series of N bits.");
}

#endif
