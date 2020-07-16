/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  BitsOrg.h
 *  @brief An organism consisting of a series of bits.
 *  @note Status: ALPHA
 */

#ifndef MABE_BITS_ORGANISM_H
#define MABE_BITS_ORGANISM_H

#include "../core/MABE.h"
#include "../core/Organism.h"
#include "../core/OrganismManager.h"

#include "tools/BitVector.h"
#include "tools/random_utils.h"

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
      double mut_prob = 0.01;            // Probability of each bit mutating on reproduction.
      std::string output_name = "bits";  // Name of trait that should be used to access bits.
    };

    /// Use "to_string" to convert.
    std::string ToString() override { return emp::to_string(bits); }

    size_t Mutate(emp::Random & random) override {
      if (bits.size() == 0) return 0;
      for (size_t i = 0; i < 3; i++) {
        size_t pos = random.GetUInt(bits.size());
        bits[pos] = random.P(0.5);
      }
      return 3;
    }

    void Randomize(emp::Random & random) override {
      emp::RandomizeBitVector(bits, random, 0.5);
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
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Bitset output from organism.",
                                  emp::BitVector(0));
    }
  };

  MABE_REGISTER_ORG_TYPE(BitsOrg, "Organism consisting of a series of N bits.");
}

#endif
