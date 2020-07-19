/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  SelectWith.h
 *  @brief MABE module to link selection in one population to that of another.
 */

#ifndef MABE_SELECT_WITH_H
#define MABE_SELECT_WITH_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  /// Link the selection in one population to that of another.
  class SelectWith : public Module {
  private:
    /// A record of a replication event by the monitored module.
    struct ReproRecord {
      size_t parent_pos;     // Population position of parent organism.
      size_t offspring_pos;  // Population position where offspring placed.
    };

    emp::vector<ReproRecord> record;      ///< Set of reproduce events to replicate in this module.

    emp::Ptr<ModuleBase> tracked_module;  ///< Module that we are linked to.
    int parent_pop_id = 0;                ///< Which population are we taking parents from?
    int offspring_pop_id = 1;             ///< Which population should births go into?

  public:
    SelectWith(mabe::MABE & control,
               const std::string & name="SelectWith",
               const std::string & desc="Mimic reproduction events in another selection module.")
      : Module(control, name, desc)
    {
      SetSelectMod(true);                ///< Mark this module as a selection module.
    } 
    ~SelectWith() { }

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

    void OnUpdate(size_t update) override {
      // Construct a map of all IDs to their associated fitness values.
      emp::valsort_map<OrgPosition, double> id_fit_map;
      Collection select_col = control.GetAlivePopulation(select_pop_id);
      for (auto it = select_col.begin(); it != select_col.end(); it++) {
        id_fit_map.Set(it.AsPosition(), it->GetVar<double>(trait));
        //std::cout << "Measuring fit " << it->GetVar<double>(trait) << std::endl;
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

  MABE_REGISTER_MODULE(SelectWith, "Choose the top fitness organisms for replication.");
}

#endif
