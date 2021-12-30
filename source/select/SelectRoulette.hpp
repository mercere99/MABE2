/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SelectRoulette.hpp
 *  @brief MABE module to enable roulette selection.
 */

#ifndef MABE_SELECT_ROULETTE_H
#define MABE_SELECT_ROULETTE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/datastructs/IndexMap.hpp"

namespace mabe {

  /// Add roulette selection with the current population.
  class SelectRoulette : public Module {
  private:
    std::string fit_equation;    ///< Which equation should we select on?

    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      if (select_pop.GetID() == birth_pop.GetID()) {
        emp::notify::Error("SelectRoulette currently requires birth_pop and select_pop to be different.");
        return Collection{};
      }

      auto fit_fun = control.BuildTraitEquation(select_pop, fit_equation);

      emp::IndexMap fit_map(select_pop.GetSize(), 0.0);
      for (size_t org_pos = 0; org_pos < select_pop.GetSize(); org_pos++) {
        if (select_pop.IsEmpty(org_pos)) continue;
        fit_map[org_pos] = fit_fun(select_pop[org_pos]);
      }

      // Loop through picking IDs proportional to fitness_trait, replicating each
      emp::Random & random = control.GetRandom();
      Collection placement_list;
      for (size_t birth_id = 0; birth_id < num_births; birth_id++) {
        size_t org_id = fit_map.Index( random.GetDouble(fit_map.GetWeight()) );
        placement_list += control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
      }

      return placement_list;
    }

  public:
    SelectRoulette(
      mabe::MABE & control,
      const std::string & name="SelectRoulette",
      const std::string & desc="Module to choose random organisms for replication, based on fitness."
    ) : Module(control, name, desc)
    {
      SetSelectMod(true);               ///< Mark this module as a selection module.
    } 
    ~SelectRoulette() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectRoulette & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform roulette selection on the provided organisms.");
    }

    void SetupConfig() override {
      LinkVar(fit_equation, "fitness_fun", "Function used as fitness for selection?");
    }

    void SetupModule() override {
      AddRequiredEquation(fit_equation);   // The fitness traits must be set by another module.
    }

  };

  MABE_REGISTER_MODULE(SelectRoulette, "Randomly choose organisms to replicate weighted by fitness.");
}

#endif
