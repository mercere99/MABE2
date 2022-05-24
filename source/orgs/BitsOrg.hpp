/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  BitsOrg.hpp
 *  @brief An organism consisting of a series of bits.
 *  @note Status: ALPHA
 */

#ifndef MABE_BITS_ORGANISM_H
#define MABE_BITS_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class BitsOrg : public OrganismTemplate<BitsOrg> {
  protected:
    emp::BitVector bits;

  public:
    BitsOrg(OrganismManager<BitsOrg> & _manager)
      : OrganismTemplate<BitsOrg>(_manager), bits(100) { }
    BitsOrg(const BitsOrg &) = default;
    BitsOrg(BitsOrg &&) = default;
    BitsOrg(const emp::BitVector & in, OrganismManager<BitsOrg> & _manager)
      : OrganismTemplate<BitsOrg>(_manager), bits(in) { }
    BitsOrg(size_t N, OrganismManager<BitsOrg> & _manager)
      : OrganismTemplate<BitsOrg>(_manager), bits(N) { }
    ~BitsOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of each bit mutating on reproduction.
      std::string output_name = "bits";  ///< Name of trait that should be used to access bits.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites. 
      bool init_random = true;           ///< Should we randomize ancestor?  (false = all zeros)
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { return emp::to_string(bits); }

    size_t Mutate(emp::Random & random) override {
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);

      if (num_muts == 0) return 0;
      if (num_muts == 1) {
        const size_t pos = random.GetUInt(bits.size());
        bits.Toggle(pos);
        return 1;
      }

      // Only remaining option is num_muts > 1.
      auto & mut_sites = SharedData().mut_sites;
      mut_sites.Clear();
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(bits.size());
        if (mut_sites[pos]) { --i; continue; }  // Duplicate position; try again.
        mut_sites.Set(pos);
      }
      bits ^= mut_sites;

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      emp::RandomizeBitVector(bits, random, 0.5);
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) emp::RandomizeBitVector(bits, random, 0.5);
    }

    /// Put the bits in the correct output position.
    void GenerateOutput() override {
      SetTrait<emp::BitVector>(SharedData().output_name, bits);
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
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, bits.size());

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(bits.size());

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Bitset output from organism.",
                                  emp::BitVector(0));
    }
  };

  MABE_REGISTER_ORG_TYPE(BitsOrg, "Organism consisting of a series of N bits.");
}

#endif
