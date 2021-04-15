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
      // @CAO: We should set required traits, but cannot be sure of their type yet...
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

      // Build a trait vector to hold the scores for each organism.
      using org_traits_t = emp::vector<double>;
      emp::vector< org_traits_t > trait_scores(num_orgs);

      // Loop through each organism to collect trait information.
      size_t num_traits = 0;
      emp::vector<size_t> start_orgs;
      for (size_t org_id = 0; org_id < num_orgs; ++org_id) {
        if (select_pop.IsEmpty(org_id)) continue;  // Skip empty positions in the population.

        // This cell is not empty so add it to the full set of organisms.
        start_orgs.push_back(org_id);

        // Collect all of the trait values for the current organism.
        trait_set.GetValues(select_pop[org_id].GetDataMap(), trait_scores[org_id]);
        if (num_traits == 0) num_traits = trait_scores[org_id].size();
        emp_assert(num_traits == trait_scores[org_id].size(),
                   org_id, num_traits, trait_scores[org_id].size(),
                   "All organisms need to have the same number of traits!");
      }

      // Setup a vector with each trait index to be shuffled as needed for selection.
      emp::vector<size_t> trait_ids = emp::NRange<size_t>(0, num_traits);
      emp::vector<size_t> cur_orgs, next_orgs;

      // Create the correct number of offspring.
      for (size_t birth_id = 0; birth_id < num_births; ++birth_id) {
        // For each offspring, start with full population
        cur_orgs = start_orgs;

        // Shuffle traits into a random order.
        emp::Shuffle(control.GetRandom(), trait_ids);

        // then step through traits and filter based on each.
        for (size_t trait_id : trait_ids) {
          // Find the maximum value of the current trait.
          double min_value = std::numeric_limits<double>::max();
          double max_value = std::numeric_limits<double>::min();
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
        }

        emp_assert(cur_orgs.size() > 0);

        // If there's only one organism left, replicate it!
        if (cur_orgs.size() == 1) {
          control.Replicate(select_pop.IteratorAt(cur_orgs[0]), birth_pop);
        }

        // Otherwise pick a random organism from the ones remaining.
        else {
          int org_id = cur_orgs[ control.GetRandom().GetUInt(cur_orgs.size()) ];
          control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
        }

      }

    }
  };

  MABE_REGISTER_MODULE(SelectLexicase, "Shuffle traits each time an organism is chose for replication.");
}

#endif
