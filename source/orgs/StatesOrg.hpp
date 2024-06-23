/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file StatesOrg.hpp
 *  @brief An organism consisting of a fixed-size series of states.
 *  @note Status: ALPHA
 */

#ifndef MABE_STATES_ORGANISM_H
#define MABE_STATES_ORGANISM_H

#include <span>

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/datastructs/span_utils.hpp"
#include "emp/math/Distribution.hpp"

namespace mabe {

  class StatesOrg : public OrganismTemplate<StatesOrg> {
  protected:
    // How can a state change?
    enum ChangeType {
      CHANGE_NONE=0,  // No changes are a allow.
      CHANGE_UNIFORM, // States can change to any other state with uniform probability.
      CHANGE_RING,    // States can change + or - one, looping at ends.
    };

  public:
    struct ManagerData : public Organism::ManagerData {
      emp::String genome_name = "states";  ///< Name of trait that should be used to access values.
      size_t num_states;                   ///< Number of unique states in an organism.
      size_t genome_size = 100;            ///< Number of positions in this genome.
      double mut_prob = 0.01;              ///< Probability of position mutating on reproduction.
      ChangeType change_type = CHANGE_UNIFORM;
      bool init_random = true;             ///< Should we randomize ancestor?  (false = all 0.0)

      // Helper member variables.
      emp::Binomial mut_dist;              ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;            ///< A pre-allocated vector for mutation sites. 
    };

    StatesOrg(OrganismManager<StatesOrg> & _manager)
      : OrganismTemplate<StatesOrg>(_manager) { }
    StatesOrg(const StatesOrg &) = default;
    StatesOrg(StatesOrg &&) = default;
    ~StatesOrg() { ; }

    emp::String ToString() const override {
      std::span<const size_t> vals = GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);
      return emp::MakeString(vals);
    }

    size_t Mutate(emp::Random & random) override {
      if (SharedData().change_type == CHANGE_NONE) {
        emp::notify::Warning("Trying to mutate StatesOrg, but no changes allowed.");
        return 0;
      }

      // Identify number of and positions for mutations.
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);
      if (num_muts == 0) return 0;

      emp::BitVector & mut_sites = SharedData().mut_sites;
      mut_sites.ChooseRandom(random, num_muts);
      std::span<size_t> genome = GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);

      // Trigger the correct type of mutations at the identified positions.
      for (size_t mut_pos = mut_sites.FindOne();
           mut_pos < mut_sites.GetSize();
           mut_pos = mut_sites.FindOne(mut_pos+1))
      {
        size_t & locus = genome[mut_pos];      // Identify the next site to mutate.
        switch (SharedData().change_type) {
        case CHANGE_RING:
          if (random.P(0.5)) {
            ++locus;
            if (locus == SharedData().num_states) locus = 0;
          } else {
            if (locus == 0) locus = SharedData().num_states;
            --locus;
          }
          break;
        case CHANGE_UNIFORM:
          locus = random.GetUInt(SharedData().num_states);
          break;
        default: break;
        }
      }

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      std::span<size_t> genome = GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);
      for (size_t & x : genome) x = random.GetUInt(SharedData().num_states);
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
      else { 
        std::span<size_t> genome = GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);
        for (size_t & x : genome) x = 0;
      }
    }


    /// Put the values in the correct output positions.
    void GenerateOutput() override {
      /// Output is already stored in the DataMap.
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().genome_size, "N", "Number of values in organism");
      GetManager().LinkVar(SharedData().num_states, "D", "How many states are possible per site?");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
        "Probability of each value mutating on reproduction.");
      GetManager().LinkMenu(
        SharedData().change_type, "change_type", "What should a point mutation do?",
        CHANGE_NONE, "null", "Do not allow mutations; issue warning if attempted.",
        CHANGE_RING, "ring", "State changes add or subtract one, looping",
        CHANGE_UNIFORM, "uniform", "Change to another state with equal probability.");
      GetManager().LinkVar(SharedData().genome_name, "genome_name",
        "Name of variable to contain set of values.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
        "Should we randomize ancestor?  (0 = all 0.0)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, SharedData().genome_size);

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(SharedData().genome_size);

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().genome_name,
                                  "Value array output from organism.",
                                  static_cast<size_t>(0),
                                  SharedData().genome_size);
    }
  };



  MABE_REGISTER_ORG_TYPE(StatesOrg, "Organism consisting of a series of N state values.");
}

#endif
