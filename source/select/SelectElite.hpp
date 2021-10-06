/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  SelectElite.hpp
 *  @brief MABE module to enable elite selection (flexible to handle mu-lambda selection)
 */

#ifndef MABE_SELECT_ELITE_H
#define MABE_SELECT_ELITE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/datastructs/valsort_map.hpp"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectElite : public Module {
  private:
    std::string trait;       ///< Which trait should we select on?
    size_t top_count=1;      ///< Top how-many should we select?
    size_t copy_count=1;     ///< How many copies of each should we make?
    int select_pop_id = 0;   ///< Which population are we selecting from?
    int birth_pop_id = 1;    ///< Which population should births go into?

  public:
    SelectElite(mabe::MABE & control,
               const std::string & name="SelectElite",
               const std::string & desc="Module to choose the top fitness organisms for replication.",
               const std::string & in_trait="fitness", size_t tcount=1, size_t ccount=1)
      : Module(control, name, desc)
      , trait(in_trait), top_count(tcount), copy_count(ccount)
    {
      SetSelectMod(true);               ///< Mark this module as a selection module.
    } 
    ~SelectElite() { }

    void SetupConfig() override {
      LinkPop(select_pop_id, "select_pop", "Which population should we select parents from?");
      LinkPop(birth_pop_id, "birth_pop", "Which population should births go into?");
      LinkVar(top_count, "top_count", "Number of top-fitness orgs to be replicated");
      LinkVar(copy_count, "copy_count", "Number of copies to make of replicated organisms");
      LinkVar(trait, "fitness_trait", "Which trait provides the fitness value to use?");
    }

    void SetupModule() override {
      AddRequiredTrait<double>(trait);  ///< The fitness trait must be set by another module.
    }

    void OnUpdate(size_t /* update */) override {
      // Construct a map of all IDs to their associated fitness values.
      emp::valsort_map<OrgPosition, double> id_fit_map;
      Collection select_col = control.GetAlivePopulation(select_pop_id);
      for (auto it = select_col.begin(); it != select_col.end(); it++) {
        id_fit_map.Set(it.AsPosition(), it->GetTrait<double>(trait));
      }

      // Loop through the IDs in fitness order (from highest), replicating each
      size_t num_reps = 0;
      Population & birth_pop = control.GetPopulation(birth_pop_id);
      for (auto it = id_fit_map.crvbegin(); it != id_fit_map.crvend() && num_reps++ < top_count; it++) {
        control.Replicate(it->first, birth_pop, copy_count);
      }
    }
  };

  MABE_REGISTER_MODULE(SelectElite, "Choose the top fitness organisms for replication.");
}

#endif
