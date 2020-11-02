/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  RandomReplacement.h
 *  @brief Each birth replace a random organism in the population, keeping a constant size
 *
 *   
 *  When a birth occurs, the child is placed over an existing organism at random.
 *  This keeps the population size constant assuming there are no other deaths
 * 
 *  When a neighbor position is requested, a random position from the entire population is
 *  returned.
 */

#ifndef MABE_RANDOM_REPLACEMENT_H
#define MABE_RANDOM_REPLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class RandomReplacement : public Module {
  private:
    Collection target_collect;

  public:
    RandomReplacement(mabe::MABE & control,
                    const std::string & name="RandomReplacement",
                    const std::string & desc="Module to place new organisms over random organisms.")
      : Module(control, name, desc), target_collect(control.GetPopulation(1))
    {
      SetPlacementMod(true);
    }
    ~RandomReplacement() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    OrgPosition DoPlaceBirth(Organism & /* org */, OrgPosition /* ppos */,
                             Population & target_pop) override
    {
      // If the current position is monitored, return a random place in the population.
      if (target_collect.HasPopulation(target_pop)) 
        return OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

    // Injections always go into the active population.
    OrgPosition DoPlaceInject(Organism & org, Population & target_pop) override {
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

  MABE_REGISTER_MODULE(RandomReplacement, "Always appened births to the end of a population.");
}

#endif
