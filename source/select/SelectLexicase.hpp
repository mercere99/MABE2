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
      trait_set.SetTraits(trait_inputs);     ///< Parse set of trait inputs passed in.
      AddRequiredTraits<double>(trait_set);  ///< The fitness trait must be set by another module.
    }

    void OnUpdate(size_t update) override {

      // Collect information about the population we're using.
      const mabe::Population & in_pop = control.GetPopulation(select_pop_id);
      const size_t num_orgs = in_pop.GetSize();
      const size_t num_traits = trait_set.GetNumValues();

      // Build a fitness map for each trait.  A fitness map is an ordered map (low fitness to
      // high) when each entry is associated with a BitVector indicating which organism have
      // the fitness.  We'll then be able to quickly jump through fitness tiers during organism
      // selection.
      using fit_map_t = std::map<double, emp::BitVector>;  // Map of fitness for a single trait.
      emp::vector< fit_map_t > trait_scores; 

      // Loop through each organism to collect trait information.
      emp::vector<double> cur_values;  // Vector to collect each org's trait values.
      for (size_t org_id = 0; org_id < num_orgs; ++org_id) {
        // Skip empty positions in the population.
        if (in_pop.IsEmpty(org_id)) continue;

        // Collect all of the values for this organism.
        trait_set.GetValues(in_pop[org_id].GetDataMap(), cur_values);

        // Place organism values into associated fitness maps.
        for (size_t trait_id = 0; trait_id < num_traits; ++trait_id) {
          double cur_fit = cur_values[trait_id];             // Get this organism's trait value.
          fit_map_t & fit_map = trait_scores[trait_id];      // Grab proper fitness map.
          emp::BitVector & fit_ids = fit_map[cur_fit];       // Find entry in the fitness map.
          if (!fit_ids.GetSize()) fit_ids.Resize(num_orgs);  // Initialize entry if needed.
          fit_ids.Set(org_id);                               // Include this org in fitness entry.
        }
      }

      // Setup a vector with each trait index to be shuffled as needed for selection.
      emp::vector<size_t> trait_ids = emp::NRange(0, num_traits);

      // Create the correct number of offspring.
      for (size_t birth_id = 0; birth_id < num_births; ++birth_id) {

      }


      // Loop through the IDs in fitness order (from highest), replicating each
      size_t num_reps = 0;
      Population & birth_pop = control.GetPopulation(birth_pop_id);
      for (auto it = id_fit_map.crvbegin(); it != id_fit_map.crvend() && num_reps++ < top_count; it++) {
        //std::cout << "Replicating fit " << it->first->GetVar<double>(trait) << std::endl;
        control.Replicate(it->first, birth_pop, copy_count);
      }
    }
  };

  MABE_REGISTER_MODULE(SelectLexicase, "Choose the top fitness organisms for replication.");
}

#endif
