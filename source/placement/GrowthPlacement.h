/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  GrowthPlacement.h
 *  @brief Default placement using one or two unstructured populations, "main" and "next".
 */

#ifndef MABE_GROWTH_PLACEMENT_H
#define MABE_GROWTH_PLACEMENT_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class GrowthPlacement : public Module {
  private:
    int main_pop=0;
    int next_pop=1;

  public:
    GrowthPlacement(mabe::MABE & control,
                    const std::string & name="GrowthPlacement",
                    const std::string & desc="Module to always appened births onto a population.")
      : Module(control, name, desc)
    {
      SetPlacementMod(true);
    }
    ~GrowthPlacement() { }

    void SetupConfig() override {
      LinkPop(main_pop, "from_pop", "Population to manage births from.",0);
      LinkPop(next_pop, "to_pop", "Population to place offspring; use from_pop for async generations.",1);
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    void OnUpdate(size_t update) override {
      // If we are running a synchronous reproduction, move the next generation to this one; delete current.
      if (main_pop != next_pop) {
        Population & from_pop = control.GetPopulation(next_pop);
        Population & to_pop = control.GetPopulation(main_pop);

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
    }

    OrgPosition DoPlaceBirth(Organism & org, OrgPosition ppos) override {
      (void) org;  // By default, organism doesn't matter.

      // If birth is not coming from monitored population, don't place!
      if (ppos.PopID() != main_pop) return OrgPosition();
      return control.PushEmpty(control.GetPopulation(next_pop));  // @CAO Should use next pop...
    }

    // Injections always go into the active population.
    OrgPosition DoPlaceInject(Organism & org) override {
      (void) org;  // By default, organism doesn't matter.
      return control.PushEmpty(control.GetPopulation(main_pop));  // @CAO Should use main pop...
    }

    OrgPosition DoFindNeighbor(OrgPosition pos) override {
      emp::Ptr<Population> pop_ptr = pos.PopPtr();

      // If the current position is either not a population or not one monitored, don't find!
      if (pop_ptr.IsNull() || pos.PopID() != main_pop) return OrgPosition();
      return OrgPosition(pop_ptr, control.GetRandom().GetUInt(pop_ptr->GetSize()));
    }


  };

  MABE_REGISTER_MODULE(GrowthPlacement, "Always appened births onto a population.");
}

#endif
