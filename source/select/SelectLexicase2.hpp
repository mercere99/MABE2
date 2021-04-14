/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
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

namespace mabe {

  /// Add Lexicase selection with the current population.
  class SelectLexicase : public Module {
  private:
    std::string trait_inputs;   ///< Which set of trait values should we select on?
    TraitSet<double> trait_set; ///< Processed version of trait_inputs.
    double epsilon = 0.0;       ///< Range from max value to be preserved? (fraction of max)
    int select_pop_id = 0;      ///< Which population are we selecting from?
    int birth_pop_id = 1;       ///< Which population should births go into?
    size_t num_births = 1;      ///< How many offspring organisms should we produce?

  public:
    SelectLexicase(mabe::MABE & control,
               const std::string & name="SelectLexicase",
               const std::string & desc="Module to choose the top fitness organisms for replication.")
      : Module(control, name, desc)
    {
      SetSelectMod(true);    ///< Mark this module as a selection module.
    }
    ~SelectLexicase() { }

    void SetupConfig() override {
      LinkPop(select_pop_id, "select_pop", "Which population should we select parents from?");
      LinkPop(birth_pop_id, "birth_pop", "Which population should births go into?");
      LinkVar(trait_inputs, "fitness_traits", "Which traits provide the fitness values to use?");
      LinkVar(epsilon, "epsilon", "Range from max value to be preserved? (fraction of max)");
      LinkVar(num_births, "num_births", "Number of offspring organisms to produce");
    }

    void SetupModule() override {
      // @CAO: We should set these traits up as required, but cannot be sure of their type yet...
      //       (They may be double or emp::vector<double>)
      // emp::vector<std::string> trait_names = emp::slice(trait_inputs);
      // for (const std::string & name : trait_names) {
      //   AddRequiredTrait<double>(name);
      // }
    }

    void SetupDataMap(emp::DataMap & dmap) override {
      trait_set.SetLayout(dmap.GetLayout()); ///< Give this trait set a layout to optimize.
      trait_set.SetTraits(trait_inputs);     ///< Parse set of trait inputs passed in.
    }

    void OnUpdate(size_t update) override {

      // Collect information about the population we're using.
      mabe::Population & select_pop = control.GetPopulation(select_pop_id);
      mabe::Population & birth_pop = control.GetPopulation(birth_pop_id);
      const size_t num_orgs = select_pop.GetSize();

      // Build a fitness map for each trait.  A fitness map is an ordered map (low fitness to
      // high) when each entry is associated with a BitVector indicating which organism have
      // the fitness.  We'll then be able to quickly jump through fitness tiers during organism
      // selection.
      using trait_map_t = std::map<double, emp::BitVector>;  // Map of fitness for a single trait.
      emp::vector< trait_map_t > trait_scores;               // Set of maps for ALL traits.

      // Loop through each organism to collect trait information.
      emp::vector<double> cur_values;  // Vector to collect each org's trait values.
      for (size_t org_id = 0; org_id < num_orgs; ++org_id) {
        // Skip empty positions in the population.
        if (select_pop.IsEmpty(org_id)) continue;

        // Collect all of the trait values for the current organism.
        trait_set.GetValues(select_pop[org_id].GetDataMap(), cur_values);

        // Update the number of traits if we haven't set it yet.
        if (trait_scores.size() == 0) {
          trait_scores.resize(cur_values.size());
        }

        // Place organism values into associated fitness maps.
        for (size_t trait_id = 0; trait_id < cur_values.size(); ++trait_id) {
          double cur_val = cur_values[trait_id];                  // Get this organism's trait value.
          trait_map_t & trait_map = trait_scores[trait_id];       // Grab proper fitness map.
          emp::BitVector & trait_bits = trait_map[cur_val];       // Find entry in the fitness map.
          if (!trait_bits.GetSize()) trait_bits.Resize(num_orgs); // Initialize entry if needed.
          trait_bits.Set(org_id);                                 // Include this org in fitness entry.
        }
      }

      const size_t num_traits = trait_scores.size();

      // Move trait bit collections over to vectors ordered by fitness.
      using fit_rank_t = emp::vector<emp::BitVector>;
      emp::vector<fit_rank_t> trait_fit_ranks(num_traits);
      for (size_t trait_id = 0; trait_id < trait_scores.size(); ++trait_id) {
        trait_map_t & trait_map = trait_scores[trait_id];
        fit_rank_t & fit_rank = trait_fit_ranks[trait_id];
        fit_rank.reserve(trait_map.size());
        for (auto it = trait_map.rbegin(); it != trait_map.rend(); ++it) {
          fit_rank.push_back(it->second);
        }
      }

      // Setup a vector with each trait index to be shuffled as needed for selection.
      emp::vector<size_t> trait_ids = emp::NRange<size_t>(0, num_traits);

      emp::BitVector cur_orgs(num_orgs);

      // Create the correct number of offspring.
      for (size_t birth_id = 0; birth_id < num_births; ++birth_id) {
        // For each offspring, start with full population
        cur_orgs.SetAll();

        // Shuffle traits into a random order.
        emp::Shuffle(control.GetRandom(), trait_ids);

        // then step through traits and filter based on each.
        for (size_t trait_id : trait_ids) {          // @CAO Track used traits to limit shuffling?
          // For the current trait, step through to figure out how it limits organisms.
          for (const emp::BitVector & fit_ids : trait_fit_ranks[trait_id]) {
            if (cur_orgs.HasOverlap(fit_ids)) {
              cur_orgs &= fit_ids;
              break;
            }
          }

          // If we are down to one organism, stop filtering.
          if (cur_orgs.CountOnes() == 1) break;    // @CAO Add BitVector::HasMultiple() or similar?
        }

        // If there's only one organism left, replicate it!
        const size_t orgs_remaining = cur_orgs.CountOnes();
        if (orgs_remaining == 1) {
          int org_id = cur_orgs.FindOne();
          control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
        }

        // Otherwise pick a random organism from the ones remaining.
        else {
          const size_t rep_pos = control.GetRandom().GetUInt(orgs_remaining);
          int org_id = cur_orgs.FindOne();
          for (size_t i = 0; i < rep_pos; ++i) org_id = cur_orgs.FindOne(org_id+1);
          control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
        }

      }

    }
  };

  MABE_REGISTER_MODULE(SelectLexicase, "Shuffle traits each time an organism is chose for replication.");
}

#endif
