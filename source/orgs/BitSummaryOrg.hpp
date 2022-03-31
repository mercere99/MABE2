/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file  BitSummaryOrg.hpp
 *  @brief An organism consisting of bit counts, but not orderings.
 *  @note Status: ALPHA
 * 
 *  This organism types represents a bitstring where the only important aspect is the number of
 *  zeros and ones, such that the whole bitstring does not need to be recorded.
 */

#ifndef MABE_SUMMARY_ORGANISM_HPP
#define MABE_SUMMARY_ORGANISM_HPP

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/math/DistributionSet.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class BitSummaryOrg : public OrganismTemplate<BitSummaryOrg> {
  protected:
    // Uses datamap to store num ones.

  public:
    BitSummaryOrg(OrganismManager<BitSummaryOrg> & _manager)
      : OrganismTemplate<BitSummaryOrg>(_manager) { }
    BitSummaryOrg(const BitSummaryOrg &) = default;
    BitSummaryOrg(BitSummaryOrg &&) = default;
    BitSummaryOrg(const emp::BitVector & in, OrganismManager<BitSummaryOrg> & _manager)
      : OrganismTemplate<BitSummaryOrg>(_manager)
      { GetTrait<size_t>(SharedData().output_name) = in.CountOnes(); }
    ~BitSummaryOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      size_t num_bits = 100;                 ///< How many bits are in the genome.
      double mut_prob = 0.01;                ///< Probability of each bit mutating on reproduction.
      double one_prob = 0.5;                 ///< Probability of each mutation producing a one.
      emp::BinomialSet binomials;            ///< Store pre-calculated binomials.
      std::string output_name = "num_ones";  ///< Name of trait that should be used to access bits.
      bool init_random = true;               ///< Should we randomize ancestor? (false = all zeros)
    };

    /// Use "to_string" to convert.
    std::string ToString() const override {
      size_t num_ones = GetTrait<size_t>(SharedData().output_name);
      const size_t num_zeros = SharedData().num_bits - num_ones;
      return emp::to_string("[0:", num_zeros, ",1:", num_ones, "]");
    }

    size_t Mutate(emp::Random & random) override {
      size_t & num_ones = GetTrait<size_t>(SharedData().output_name);

      const double mut_prob = SharedData().mut_prob;
      const double one_prob = SharedData().one_prob;

      const size_t num_zeros = SharedData().num_bits - num_ones;
      const size_t one_muts = SharedData().binomials.PickRandom(random, mut_prob, num_ones);
      const size_t zero_muts = SharedData().binomials.PickRandom(random, mut_prob, num_zeros);
      const size_t num_muts = one_muts + zero_muts;
      const size_t new_ones = SharedData().binomials.PickRandom(random, one_prob, num_muts);

      num_ones = num_ones + new_ones - one_muts;

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      size_t & num_ones = GetTrait<size_t>(SharedData().output_name);
      num_ones = SharedData().binomials.PickRandom(random, SharedData().one_prob, SharedData().num_bits);
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
    }

    /// Put the bits in the correct output position.
    void GenerateOutput() override {
      // Nothing to do here - output already stored in DataMap.
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().num_bits, "num_bits",
                      "Number of bits in the simulated sequence.");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each bit being randomized on reproduction.");
      GetManager().LinkVar(SharedData().one_prob, "one_prob",
                      "Probability of a randomized bit becoming a one.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to output number of ones.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = all zeros)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Num ones output from organism.",
                                  (size_t) 0);
    }
  };

  MABE_REGISTER_ORG_TYPE(BitSummaryOrg, "Organism consisting of a summary series of N bits.");
}

#endif
