/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
 *
 *  @file  SelectLexicase.hpp
 *  @brief MABE module to enable Lexicase selection
 */

#ifndef MABE_SELECT_LEXICASE_H
#define MABE_SELECT_LEXICASE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../core/TraitSet.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/datastructs/valsort_map.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/math/random_utils.hpp"

#include "emp/debug/debug.hpp"

namespace mabe {

  /// Add Lexicase selection with the current population.
  class SelectLexicase : public Module {
  private:
    emp::String trait_inputs;   ///< Which set of trait values should we select on?
    TraitSet<double> trait_set; ///< Processed version of trait_inputs.
    double epsilon = 0.0;       ///< Range from max value to be preserved? (fraction of max)
    size_t sample_traits = 0;   ///< Number of test cases to use each generation (0=off)

    emp::String major_trait;    ///< Is there a trait we want to emphasize in importance?
    size_t major_range=10;      ///< Major trait guaranteed to be in first X tests.

    int require_first=0;        ///< Do we require each test to be picked first at least once?

    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      if (num_births > 1 && select_pop.GetID() == birth_pop.GetID()) {
        emp::notify::Error("SelectLexicase currently requires birth_pop and select_pop to be different if selecting multiple organisms.");
        return Collection();
      }
      if (select_pop.IsEmpty()) return Collection();  // No living orgs!!

      const size_t live_id = select_pop.FindOccupiedPos();
      const size_t num_traits = trait_set.CountValues(select_pop[live_id].GetDataMap());
      emp::Random & random = control.GetRandom();
      const size_t major_count = (major_trait.size()) ? 1 : 0;

      // If we're not using all of the traits, determine which ones to select on.
      emp::vector<size_t> traits_used;
      if (sample_traits) {
        emp::Choose(random, num_traits-major_count, sample_traits, traits_used);
      }

      // Build a trait vector to hold the scores for each organism.
      emp::vector< emp::vector<double> > trait_scores(select_pop.GetSize());

      // Loop through each organism to collect its trait information.
      emp::vector<size_t> start_orgs;
      for (size_t org_id = live_id; org_id < select_pop.GetSize(); ++org_id) {
        if (select_pop.IsEmpty(org_id)) continue;  // Skip empty positions in the population.
        start_orgs.push_back(org_id);  // Add cell ID to the set of starting organisms.

        // Collect all of the trait values for the current organism.
        // If we are using a subset of traits, take that into account.
        if (traits_used.size() > 0) {
          trait_set.GetValues(select_pop[org_id].GetDataMap(), trait_scores[org_id], traits_used);
        } else {
          trait_set.GetValues(select_pop[org_id].GetDataMap(), trait_scores[org_id]);

          // @CAO: This should be a user error, not a program error:
          emp_assert(num_traits == trait_scores[org_id].size(),
                    org_id, num_traits, trait_scores[org_id].size(),
                    "All organisms must have the same number of traits!");
        }
      }

      // Setup a vector with each trait index to be shuffled as needed for selection.
      if (traits_used.size() == 0) traits_used = emp::NRange<size_t>(0, num_traits);
      emp::vector<size_t> cur_orgs, next_orgs;

      // Create the correct number of offspring.
      Collection placement_list;
      emp::vector<size_t> traits_order;
      for (size_t birth_id = 0; birth_id < num_births; ++birth_id) {
        traits_order = traits_used;
        emp::Shuffle(random, traits_order);  // Shuffle traits into a random order.
        if (major_trait.size()) {            // Insert the major trait if we are using one.
          size_t major_pos = random.GetUInt(major_range);
          traits_order.insert(traits_order.begin()+major_pos, num_traits-1);
        }
        if (require_first && birth_id < num_traits) {  // Give each or a turn first, if needed.
          traits_order.insert(traits_order.begin(), birth_id);
        }

        // Step through traits and filter based on each trait.
        cur_orgs = start_orgs;                              // Start with full population
        for (size_t i = 0; i < traits_order.size(); ++i) {
          size_t trait_id = traits_order[i];
          double min_value = std::numeric_limits<double>::max();
          double max_value = std::numeric_limits<double>::lowest();
          for (size_t org_id : cur_orgs) {
            const double cur_value = trait_scores[org_id][trait_id];
            if (cur_value < min_value) min_value = cur_value;
            if (cur_value > max_value) max_value = cur_value;
          }

          // If there's not enough variation in this trait, move on to the next trait.
          if (min_value + epsilon >= max_value) continue;

          // Eliminate all organisms with a lower score than the threshold.
          double threshold = max_value - epsilon;
          for (size_t org_id : cur_orgs) {
            if (trait_scores[org_id][trait_id] >= threshold) next_orgs.push_back(org_id);
          }

          // Cleanup for the next trait.
          cur_orgs.resize(0);
          std::swap(cur_orgs, next_orgs);

          // If we are down to just one organism, stop early!
          if (cur_orgs.size() == 1) break;
          emp_assert(cur_orgs.size() > 0);
        }

        emp_assert(cur_orgs.size() > 0);

        // If there's only one organism left, mark it for replication.
        if (cur_orgs.size() == 1) {
          placement_list += control.Replicate(select_pop.IteratorAt(cur_orgs[0]), birth_pop);
        }

        // Otherwise pick a random organism from the ones remaining.
        else {
          int org_id = cur_orgs[ random.GetUInt(cur_orgs.size()) ];
          placement_list += control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
        }

      }

      return placement_list;
    }

  public:
    SelectLexicase(mabe::MABE & control,
               const emp::String & name="SelectLexicase",
               const emp::String & desc="Module to choose the top fitness organisms for replication.")
      : Module(control, name, desc)
    {
      SetSelectMod(true);    ///< Mark this module as a selection module.
    }
    ~SelectLexicase() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectLexicase & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform lexicase selection on the identified population.");
    }

    void SetupConfig() override {
      LinkVar(trait_inputs, "fitness_traits", "Which traits provide the fitness values to use?");
      LinkVar(epsilon, "epsilon", "Range from max value to be preserved? (fraction of max)");
      LinkVar(sample_traits, "sample_traits", "Number of test cases to use each generation (0=all)" );
      LinkVar(major_trait, "major_trait", "Is there a particular trait we want to emphasize?");
      LinkVar(major_range, "major_range", "Major trait guaranteed to be in first X tests");
      LinkVar(require_first, "require_first", "Require each test to be first at least once? (0=off; 1=on)");
    }

    void SetupModule() override {
      // We should always have a minimal epsilon to handle mathematical imprecision of doubles.
      if (epsilon <= 0.0) epsilon = 0.000000001; // One billionth.

      // All of the traits used are required to be generated by another module.
      emp::vector<emp::String> trait_names = trait_inputs.Slice(",");
      for (const emp::String & name : trait_names) {
        AddRequiredTrait<double, emp::vector<double>>(name, TraitInfo::ANY_COUNT);
      }

      // Check for the the major trait, if we have one.
      if (major_trait.size()) {
        AddRequiredTrait<double>(major_trait, 1);
      }
    }

    void SetupDataMap(emp::DataMap & dmap) override {
      trait_set.SetLayout(dmap.GetLayout()); ///< Give this trait set a layout to optimize.
      trait_set.SetTraits(trait_inputs);     ///< Parse set of trait inputs passed in.
      if (major_trait.size()) { trait_set.AddTrait(major_trait); } // Add major trait at end, if any
    }


  };

  MABE_REGISTER_MODULE(SelectLexicase, "Shuffle traits each time an organism is chose for replication.");
}

#endif
