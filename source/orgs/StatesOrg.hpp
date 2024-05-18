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
      std::string genome_name = "states";  ///< Name of trait that should be used to access values.
      size_t num_states;                   ///< Number of unique states in an organism.
      size_t genome_size = 100;            ///< Number of positions in this genome.
      double mut_prob = 0.01;              ///< Probability of position mutating on reproduction.
      ChangeType change_type = CHANGE_UNIFORM;

      // Helper member variables.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites. 
      bool init_random = true;           ///< Should we randomize ancestor?  (false = all 0.0)
    };

    StatesOrg(OrganismManager<StatesOrg> & _manager)
      : OrganismTemplate<StatesOrg>(_manager) { }
    StatesOrg(const StatesOrg &) = default;
    StatesOrg(StatesOrg &&) = default;
    ~StatesOrg() { ; }

    /// Use "to_string" to convert.
    std::string ToString() const override {
      std::span<const size_t> vals = GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);
      return emp::to_string(vals);
    }

    size_t Mutate(emp::Random & random) override {
      if (SharedData().change_type == CHANGE_NONE) {
        notify::Warning("Trying to mutate StatesOrg, but no changes allowed.");
        return 0;
      }

      // Identify number of and positions for mutations.
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);
      emp::BitVector & mut_sites = SharedData().mut_sites;
      mut_sites.ChooseRandom(random, num_muts);
      std::span<size_t> genome = GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);

      // Trigger the correct type of mutations at the identified positions.
      size_t mut_pos = mut_sites.FindOne();
      while (mut_pos < mut_sizes.GetSize()) {
        double & locus = genome[mut_pos];      // Identify the next site to mutate.
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
        }
        mut_pos = mut_sites.FindOne(mut_pos+1);  // Move on to the next site to mutate.
      }

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      std::span<double> genome = GetTrait<double>(SharedData().genome_name, SharedData().genome_size);
      double total = 0.0;
      for (double & x : genome) {
        x = random.GetDouble(SharedData().min_value, SharedData().max_value);
        total += x;
      }
      SetTrait<double>(SharedData().total_name, total);  // Store total in data map.
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
      else { 
        std::span<double> genome = GetTrait<double>(SharedData().genome_name, SharedData().genome_size);
        double total = 0.0;
        for (double & x : genome) x = 0.0;
        SetTrait<double>(SharedData().total_name, total);  // Store total in data map.
      }
    }


    /// Put the values in the correct output positions.
    void GenerateOutput() override {
      /// Output is already stored in the DataMap.
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().genome_size, "N", "Number of values in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each value mutating on reproduction.");
      GetManager().LinkVar(SharedData().mut_size, "mut_size",
                      "Standard deviation on size of mutations.");
      GetManager().LinkVar(SharedData().min_value, "min_value",
                      "Lower limit for value fields.");
      GetManager().LinkVar(SharedData().max_value, "max_value",
                      "Upper limit for value fields.");
      GetManager().LinkMenu(
        SharedData().lower_bound, "lower_bound", "How should the lower limit be enforced?",
        LIMIT_NONE, "no_limit", "Allow values to be arbitrarily low.",
        LIMIT_CLAMP, "clamp", "Reduce too-low values to min_value.",
        LIMIT_WRAP, "wrap", "Make low values loop around to maximum.",
        LIMIT_REBOUND, "rebound", "Make low values 'bounce' back up." );
      GetManager().LinkMenu(
        SharedData().upper_bound, "upper_bound", "How should the upper limit be enforced?",
        LIMIT_NONE, "no_limit", "Allow values to be arbitrarily high.",
        LIMIT_CLAMP, "clamp", "Reduce too-high values to max_value.",
        LIMIT_WRAP, "wrap", "Make high values loop around to minimum.",
        LIMIT_REBOUND, "rebound", "Make high values 'bounce' back down." );
      GetManager().LinkVar(SharedData().genome_name, "genome_name",
                      "Name of variable to contain set of values.");
      GetManager().LinkVar(SharedData().total_name, "total_name",
                      "Name of variable to contain total of all values.");
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
                                  0.0,
                                  SharedData().genome_size);
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().total_name,
                                  "Total of all organism outputs.",
                                  0.0);
    }
  };

  ///////////////////////////////////////////////////////////////////////////////////////////
  //  Helper functions....

  void StatesOrg::ManagerData::ApplyBounds(double & value) {
    if (value > max_value) {
      switch (upper_bound) {
        case LIMIT_NONE:    break;
        case LIMIT_CLAMP:   value = max_value; break;
        case LIMIT_WRAP:    value -= (max_value - min_value); break;
        case LIMIT_REBOUND: value = 2 * max_value - value; break;
        case LIMIT_ERROR:   break;  // For now; perhaps do something with error?
      }
    }
    else if (value < min_value) {
      switch (lower_bound) {
        case LIMIT_NONE:    break;
        case LIMIT_CLAMP:   value = min_value; break;
        case LIMIT_WRAP:    value += (max_value - min_value); break;
        case LIMIT_REBOUND: value = 2 * min_value - value; break;
        case LIMIT_ERROR:   break;  // For now; perhaps do something with error?
      }
    }
  }

  void StatesOrg::ManagerData::ApplyBounds(std::span<double> & vals) {
    const size_t range_size = max_value - min_value;

    switch (upper_bound) {
    case LIMIT_NONE: break;
    case LIMIT_CLAMP:
      for (double & value : vals) {
        if (value > max_value) value = max_value;
        else if (value < min_value) value = min_value;        
      }
      break;
    case LIMIT_WRAP:
      for (double & value : vals) {
        if (value > max_value) value -= range_size;
        else if (value < min_value) value += range_size;        
      }
      break;
    case LIMIT_REBOUND:
      for (double & value : vals) {
        if (value > max_value) value = 2 * max_value - value;
        else if (value < min_value) value = 2 * min_value - value;
      }
      break;
    default:
      break; // Should probably throw error.
    }
  }


  MABE_REGISTER_ORG_TYPE(StatesOrg, "Organism consisting of a series of N floating-point values.");
}

#endif
