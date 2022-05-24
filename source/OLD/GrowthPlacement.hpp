/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  GrowthPlacement.hpp
 *  @brief Default placement rules for how an organism should be added to a population.
 * 
 *  This standard population organization has all new organisms (whether born or injected)
 *  appended on to the end of the population vector.  It does not monitor deaths at all, so
 *  if deaths occur they are left as empty positions.
 * 
 *  When a neighbor position is requested, a random position from the entire population is
 *  returned.
 * 
 *  Pros: This is a fast, easy population method, and probably the correct default for most
 *        simple experiments.
 * 
 *  Cons: If an experiment has frequent deaths, populations can become sparse and hard to 
 *        work with.  (Also, if you need any form of spatial structure in the population
 *        object, this won't provide it.)
 */

#ifndef MABE_GROWTH_PLACEMENT_H
#define MABE_GROWTH_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class GrowthPlacement : public Module {
  private:
    Collection target_collect;

  public:
    GrowthPlacement(mabe::MABE & control,
                    const std::string & name="GrowthPlacement",
                    const std::string & desc="Module to always appened organisms onto a population.")
      : Module(control, name, desc), target_collect(control.GetPopulation(1),control.GetPopulation(0))
    {
      SetPlacementMod(true);
    }
    ~GrowthPlacement() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    OrgPosition DoPlaceBirth(Population & target_pop,
                             Organism & /* org */, OrgPosition /* ppos */) override
    {
      // If birth is going to a monitored population, place it in a new, empty cell!
      if (target_collect.HasPopulation(target_pop)) return control.PushEmpty(target_pop);

      // Otherwise, don't place!
      return OrgPosition();      
    }

    // Injections always go into the active population.
    OrgPosition DoPlaceInject(Population & target_pop, Organism & /* org */) override {
      // If inject is going to a monitored population, place it in a new, empty cell!
      if (target_collect.HasPopulation(target_pop)) return control.PushEmpty(target_pop);

      // Otherwise, don't place!
      return OrgPosition();      
    }

    OrgPosition DoFindNeighbor(OrgPosition pos) override {
      emp::Ptr<Population> pop_ptr = pos.PopPtr();

      // If the current position is monitored, return a random place in the population.
      if (target_collect.HasPosition(pos)) {
        return OrgPosition(pop_ptr, control.GetRandom().GetUInt(pop_ptr->GetSize()));
      }

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

  };

  MABE_REGISTER_MODULE(GrowthPlacement, "Always appened births to the end of a population.");
}

#endif
