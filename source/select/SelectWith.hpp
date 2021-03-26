/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  SelectWith.hpp
 *  @brief MABE module to link selection in one population to that of another.
 * 
 *  Reproduction in two populations can be linked; this module will monitor another
 *  module and whenever it triggers a reproduction event, this module will select the
 *  corresponding organism position in a population that it is managing to replicate
 *  as well.
 * 
 *  STATUS: Under development.
 */

#ifndef MABE_SELECT_WITH_H
#define MABE_SELECT_WITH_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Link the selection in one population to that of another.
  class SelectWith : public Module {
  private:
    /// A record of a replication event by the monitored module.
    struct ReproRecord {
      size_t parent_pos;     // Population position of parent organism.
      size_t offspring_pos;  // Population position where offspring placed.
    };

    emp::vector<ReproRecord> record;    ///< Set of reproduce events to replicate in this module.

    int tracked_module_id = -1;         ///< Module that we are linked to.
    int parent_pop_id = 0;              ///< Which population are we taking parents from?
    int offspring_pop_id = 1;           ///< Which population should births go into?

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
      LinkModule(tracked_module_id, "tracked_module", "Which module should we parallel?");
      LinkPop(parent_pop_id, "select_pop", "Which population should we select parents from?");
      LinkPop(offspring_pop_id, "birth_pop", "Which population should births go into?");
    }

    void SetupModule() override {
      // No traits are required for this module.
    }

    void OnUpdate(size_t update) override {
      Population & parent_pop = control.GetPopulation(parent_pop_id);
      Population & offspring_pop = control.GetPopulation(offspring_pop_id);

      // Loop through all replication events and trigger the corresponding event.
      for (auto repro_event : record) {
        control.DoBirth(
          parent_pop[repro_event.parent_pos],
          parent_pop.IteratorAt(repro_event.parent_pos),
          offspring_pop.IteratorAt(repro_event.offspring_pos)
        );
      }
    }

    void BeforePlacement(Organism &, OrgPosition to_pos, OrgPosition from_pos) override {
      // @CAO need to record events to duplicate the replications later in the up date.
    }

  };

  MABE_REGISTER_MODULE(SelectWith, "Choose the top fitness organisms for replication.");
}

#endif
