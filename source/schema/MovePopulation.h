/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  MovePopulation.h
 *  @brief Module to move organisms from one population to another (optionally clearing the destination)
 */

#ifndef MABE_SCHEMA_MOVE_POPULATION_H
#define MABE_SCHEMA_MOVE_POPULATION_H

#include "../core/Module.h"

namespace mabe {

  /// Add elite selection with the current population.
  class MovePopulation : public Module {
  private:
    int from_id = 1;    ///< Which population are we moving from?
    int to_id = 0;      ///< Which population are we moving to?
    bool clear = true;  ///< Should we clear the 'to' population before moving in?

  public:
    MovePopulation(mabe::MABE & control,
           const std::string & name="MovePopulation",
           const std::string & desc="Module to move organisms to a new population",
           int _from_id=0, int _to_id=1, bool _clear=true)
      : Module(control, name, desc), from_id(_from_id), to_id(_to_id), clear(_clear)
    {
      SetManageMod(true);         ///< Mark this module as a population  module.
    }

    void SetupConfig() override {
      LinkPop(from_id, "from_pop", "Population to move organisms from.");
      LinkPop(to_id, "to_pop", "Population to move organisms into.");
      LinkVar(clear, "clear", "Should we erase organisms at the destination?");
    }

    void OnUpdate(size_t update) override {
      Population & from_pop = control.GetPopulation(from_id);
      Population & to_pop = control.GetPopulation(to_id);

      // Clear out the current main population and resize.
      control.EmptyPop(to_pop, from_pop.GetSize());  

      // Move the next generation to the main population.
      OrgPosition it_to = to_pop.begin();
      for (OrgPosition it_from = from_pop.begin(); it_from != from_pop.end(); ++it_from, ++it_to) {
        if (it_from.IsOccupied()) control.MoveOrg(it_from, it_to);
      }

      // Clear out the next generation
      control.EmptyPop(from_pop, 0);
    }
  };

  MABE_REGISTER_MODULE(MovePopulation, "Move organisms from one populaiton to another.");
}

#endif
