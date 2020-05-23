/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  GrowthPlacement.h
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

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class GrowthPlacement : public Module {
  private:
    int pop_id = 1;

  public:
    GrowthPlacement(mabe::MABE & control,
                    const std::string & name="GrowthPlacement",
                    const std::string & desc="Module to always appened organisms onto a population.")
      : Module(control, name, desc)
    {
      SetPlacementMod(true);
    }
    ~GrowthPlacement() { }

    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population to manage.");
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    OrgPosition DoPlaceBirth(Organism & /* org */, OrgPosition /* ppos */,
                             Population & target_pop) override
    {
      // If birth is not going to monitored population, don't place!
      if (target_pop.GetID() != pop_id) return OrgPosition();
      return control.PushEmpty(target_pop);
    }

    // Injections always go into the active population.
    OrgPosition DoPlaceInject(Organism & org, Population & target_pop) override {
      // If inject is not going to monitored population, don't place!
      if (target_pop.GetID() != pop_id) return OrgPosition();
      return control.PushEmpty(target_pop);
    }

    OrgPosition DoFindNeighbor(OrgPosition pos) override {
      emp::Ptr<Population> pop_ptr = pos.PopPtr();

      // If the current position is either not a population or not one monitored, don't find!
      if (pop_ptr.IsNull() || pos.PopID() != pop_id) return OrgPosition();
      return OrgPosition(pop_ptr, control.GetRandom().GetUInt(pop_ptr->GetSize()));
    }

  };

  MABE_REGISTER_MODULE(GrowthPlacement, "Always appened births to the end of a population.");
}

#endif
